/*****************************************************************************
	proc_indexed.c	- Process indexed addressing ops

	Copyright (c) 2004 Chet Simpson, Digital Asphyxia. All rights reserved.

	The distribution, use, and duplication this file in source or binary form
	is restricted by an Artistic License (see license.txt) included with the
	standard distribution. If the license was not included with this package
	please refer to http://www.oarizo.com for more information.


	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that: (1) source code distributions
	retain the above copyright notice and this paragraph in its entirety, (2)
	distributions including binary code include the above copyright notice and
	this paragraph in its entirety in the documentation or other materials
	provided with the distribution, and (3) all advertising materials
	mentioning features or use of this software display the following
	acknowledgement:

		"This product includes software developed by Chet Simpson"
	
	The name of the author may be used to endorse or promote products derived
	from this software without specific priorwritten permission.

	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/
#include "as.h"
#include "error.h"
#include "util.h"
#include "proc_util.h"
#include "proto.h"



/*
 *	GetRegIndexType --- return register type in post-byte format
 */
static u_char GetRegIndexType(EnvContext *ctx, REGISTER r)
{
	u_char modifier;

	switch(r) {
	case REG_INDX:
		modifier = 0x00;
		break;

	case REG_INDY:
		modifier = 0x20;
		break;

	case REG_USTACK:
		modifier = 0x40;
		break;

	case REG_SSTACK:
		modifier = 0x60;
		break;

	default:
		modifier = 0;	/* Make compiler happy */
		internal((ctx, "GetRegIndexType(): '%s' Illegal register for Indexed!", GetRegisterName(r)));
		break;
	}

	return modifier;
}


/*
 *	  IndexByRegister --- a,b or d indexed
 */

static void IndexByRegister(EnvContext *ctx, int postbyte)
{
	REGISTER	 reg;

	if(*(ctx->m_Ptr++) != ',' ) {
		error(ctx, ERR_SYNTAX, "missing comma");
	}

	/* Get the register to index */
	reg = GetRegister(ctx, false, false);
	if(REG_ERROR == reg) {
		error(ctx, ERR_INVALID_REG_FOR_OPERATION, "for index operation");
		return;
	}

	/* We cannot index by ACCW */
	if(reg == REG_ACCW) {
		error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "register offsets not allowed with indexing mode of register W");
	}

	postbyte += GetRegIndexType(ctx, reg);

	EmitOpPostByte(ctx, lobyte(postbyte));

	return;
}






/*
 *	  ProcIndexed --- handle all weird stuff for indexed addressing
 */
void ProcIndexed(EnvContext *ctx, const Mneumonic *opinfo, u_char modifier)
{
	int32		result;
	u_char		postbyte;
	REGISTER	reg1;
	u_char		predec;
	u_char		pstinc;
	bool		isIndirect;
	u_char		exmodifier;
	bool		hasExpression;

	predec = 0;
	pstinc = 0;
	postbyte = 0x80;


	if(NULL != opinfo && LDQ_OPIMM == opinfo->opcode && PAGE2 == opinfo->page) {
		exmodifier = lobyte((LDQ_OPOTH - LDQ_OPIMM) - 0x10);
	} else {
		exmodifier = 0;
	}

	if(NULL != opinfo) {
		EmitOpCode(ctx, opinfo, modifier + exmodifier);
	}

	/*
		First we check for an indirect addressing mode. Standard syntax does not
		allow for other prefixes to indicate 8bit, 16bit, or other modifiers.
	*/
	if('[' == *ctx->m_Ptr) {
		isIndirect = true;

		/* bump the operand pointer */
		ctx->m_Ptr++;

		/* indirection takes this much longer */
		ctx->m_CycleCount += 3;

		/* set indirect bit */
		postbyte |= 0x10;

		/* ensure that there is a right bracket for indirection */
		if(NULL == strchr(ctx->m_Ptr, ']')) {
			error(ctx, ERR_SYNTAX, "missing ']' for indirect addressing mode");
			return;
		}
	} else {
		/* Not using indirect addressing mode */
		isIndirect = false;
	}

	reg1 = GetRegister(ctx, false, false);
	if(REG_ERROR != reg1) {

		switch(reg1) {

		case REG_ACCA:
			ctx->m_CycleCount++;
			IndexByRegister(ctx, postbyte + 0x06);
			break;

		case REG_ACCB:
			ctx->m_CycleCount++;
			IndexByRegister(ctx, postbyte + 0x05);
			break;

		case REG_ACCE:
			ctx->m_CycleCount++;
			IndexByRegister(ctx, postbyte + 0x07);
			break;

		case REG_ACCF:
			ctx->m_CycleCount++;
			IndexByRegister(ctx, postbyte + 0x0a);
			break;

		case REG_ACCD:
			ctx->m_CycleCount += 4;
			IndexByRegister(ctx, postbyte + 0x0b);
			break;

		case REG_ACCW:
			ctx->m_CycleCount =+ 4;
			IndexByRegister(ctx, postbyte + 0x0e);
			break;

		default:
			EmitOpPostByte(ctx, 0x80);
			error(ctx, ERR_INVALID_REG_FOR_OPERATION, "for indexed operation");
			break;
		}

		return;
	}


	/* Reset values */
	ctx->m_ForceWord = false;
	ctx->m_ForceByte = false;
	hasExpression = false;
	result = 0;

	/* if next char is not a comma, evaluate expression */
	if(',' != *ctx->m_Ptr) {
		reg1 = GetRegister(ctx, false, false);
		if(REG_ERROR == reg1) {	/* if register checked is invalid, check expression */

			hasExpression = Evaluate(ctx, &result, EVAL_NORMAL, NULL);

			/* Check the status */
			if(false == hasExpression) {
				error(ctx, ERR_SYNTAX, "invalid expression");
				return;
			}

			if(*ctx->m_Ptr == ',') {
				ctx->m_Ptr++;
			}
			
			else {
				if(true == isIndirect && ']' == *ctx->m_Ptr) {
					if(true == ctx->m_ForceByte) {
						error(ctx, ERR_GENERAL, "cannot force indirect addressing of a memory location to an 8 bit value");
					}

					/* Do extended indirect */
					EmitOpPostByte(ctx, 0x9f);
					EmitOpAddr(ctx, loword(result));
					return;
				} else {
					error(ctx, ERR_SYNTAX, "missing comma");
				}
			}
		} else {
			ASSERT(0);
			internal((ctx, "unexpected error encountered parsing indexed addressing mode"));
			return;
		}
	} else {
		ctx->m_Ptr++;		/* skip left bracket */
	}


	if(true == ctx->m_ForceByte && true == ctx->m_ForceWord) {	/* if both were set return error */
		error(ctx, ERR_INVALID_MODE_FOR_OPERATION, "cannot force both direct and extended addressing modes");
		ctx->m_ForceByte = false;
	}

	while(*ctx->m_Ptr == '-') {	/* skip through '-' */
		predec++;		/* set predecrement */
		ctx->m_Ptr++;			/* and go to the next pointer */
	}


	/* insert section here to evaluate in-direct register offset indexing */
	reg1 = GetRegister(ctx, false, false);			/* get register number for offset of index */

	/* process post increment */
	while(*ctx->m_Ptr=='+') {
		pstinc++;
		ctx->m_Ptr++;
	}


	if(REG_PC == reg1 || REG_PCR == reg1) {

		/* error checking for both auto inc/dec */
		if(pstinc || predec) {
			if(0 != pstinc) {
				error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "auto increment not allowed on '%s' register", GetRegisterName(reg1));
			} else {
				error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "auto drecrement not allowed on '%s' register", GetRegisterName(reg1));
			}
			return;
		}

		if(REG_PC == reg1) {

			/* Edtasm does not allow PC in indexed operations */
			if(true == ctx->m_Compat.m_DisablePCIndex) {
				error(ctx,
					  ERR_ILLEGAL_REGISTER_NAME,
					  "Invalid register '%s' for indexed addressing",
					  GetRegisterName(REG_PC));
				return;
			}

			warning(ctx,
					WARN_COMPAT,
					"Use of '%s' may be assumed to mean relative addressing of '%s'",
					GetRegisterName(REG_PC),
					GetRegisterName(REG_PCR));
		}

		else if(REG_PCR == reg1 && false == hasExpression) {
			/* Edtasm does not allow ,pcr */
			if(true == ctx->m_Compat.m_DisablePCIndex) {
				error(ctx,
					  ERR_SYNTAX,
					  "missing expression for indexed operation on '%s'",
					  GetRegisterName(REG_PC));
				return;
			}
		}

		if(false == hasExpression) {
			warning(ctx,
					WARN_COMPAT,
					"Some assemblers require an offset or expression for indexed operations with '%s'",
					GetRegisterName(reg1));
		}


		/* Handle PC/PCR compatibility */
		if(true == ctx->m_Compat.m_ForcePCR) {
			reg1 = REG_PCR;
		}


		/* BEGIN - new stuff */
		if(REG_PCR == reg1) {
			int32 pc = GetPCReg();
			result = (int16)(loword(result - pc));

			/* Adjust for the opcode and the post op byte */
			result -= 2;
		}

		if(false == ctx->m_ForceByte && false == ctx->m_ForceWord) {
			if(result >= MIN_BYTE && result <= MAX_BYTE) {	/* check for 8bit vs 16bit offset */
				ctx->m_ForceByte = true;
				ctx->m_ForceWord = false;
			} else {
				ctx->m_ForceByte = false;
				ctx->m_ForceWord = true;
			}
		}

		/* If we force a word value adjust the resulting offset by -1 */
		if(true == ctx->m_ForceWord && REG_PCR == reg1) {
			result--;
		}


		/* if set to force byte (8bit offset) write it out */
		if(true == ctx->m_ForceByte) {
			EmitOpPostByte(ctx, lobyte(postbyte + 0x0c));
			EmitOpDataByte(ctx, lobyte(result));
			ctx->m_CycleCount++;
			return;
		}

		ASSERTX(true == ctx->m_ForceWord);

		/* Default to to force word (16bit offset) write it out */
		EmitOpPostByte(ctx, lobyte(postbyte + 0x0d));
		EmitOpDataWord(ctx, loword(result));
		ctx->m_CycleCount += 5;

		return;
	}

	/* more error checking for auto inc/dec */
	if(0 != predec || 0 != pstinc) {		/* if either are used, check them */

		if(0 != result) {	/* if offset is not zero, error */
			error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "predecrement and postincrement cannot have offset");
			return;
		}

		else if(0 != predec && 0 != pstinc) {		/* if both are used, error */
			error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "predecrement and postincrement modes cannot be used together");
			return;
		}

		else if(predec > 2) {
			error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "only + and ++ are allowed for auto incremenet");
			return;
		}

		else if(pstinc > 2) { /* if inc/dec more than 2, error out */
			error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "only - and -- are allowed for auto decremenet");
			return;
		}

		/* if indirect, auto inc/dec by 2 only */
		if((1 == predec && (postbyte & 0x10) != 0)) {
			error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "auto decrement for indirect addressing must be --");
			return;
		}
		
		else if(1 == pstinc && true == isIndirect) {
			error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "auto increment for indirect addressing must be ++");
			return;
		}

		if(reg1 == REG_ACCW) {		/* support for auto inc/dec W register */
			if(predec == 1) {
				error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "auto decrement for W register must be --");
				return;
			}

			if(pstinc == 1) {
				error(ctx, ERR_INVALID_MODE_FOR_REGISTER, "auto decrement for indirect addressing must be --");
				return;
			}

			if(false == isIndirect) {
				postbyte |= 0x0f; /* if indirect is not set */
			}

			else {
				postbyte &= 0xf0;
			}

			if(predec == 2) {
				postbyte |= 0x60;
			}

			else {
				postbyte |= 0x40;
			}

			EmitOpPostByte(ctx, lobyte(postbyte));

			return;
		}

		/* if predec used, add to post byte */
		if(predec) {
			postbyte += (predec + 1);
		}

		/* or if inc used, add it to post byte */
		else if(pstinc) {
			postbyte += (pstinc - 1);
		}

		postbyte = postbyte + GetRegIndexType(ctx, reg1);		/* test register used to ensure that it's index */

		EmitOpPostByte(ctx, lobyte(postbyte));		/* output the post byte */
		ctx->m_CycleCount += 1 + predec + pstinc;	/* add cycles */
		return;	/* return */
	}

	/* process W as index */
	if(reg1 == REG_ACCW) {		
		/* FIXME - check for generating 0 offset */
		if(false == isIndirect) {
			postbyte |= 0x0f;	/* if indirect is not set */
		}

		else {
			postbyte &= 0xf0;
		}

		if(result == 0) {
			if(false == hasExpression || (true == hasExpression && false == ctx->m_Compat.m_ForceZeroOffset)) {
				EmitOpPostByte(ctx, postbyte);
				return;
			}
		}

		EmitOpPostByte(ctx, postbyte | 0x20);
		EmitOpDataWord(ctx, loword(result));
		return;
	}

	postbyte = postbyte + GetRegIndexType(ctx, reg1);

	/* check to ensure that register can be indexed */
	if(true == ctx->m_ForceWord) {	/* if 16bit value must be used, do it */
		EmitOpPostByte(ctx, postbyte + 0x09);
		EmitOpDataWord(ctx, loword(result));
		ctx->m_CycleCount += 4;
		return;
	}

	if(true == ctx->m_ForceByte) {	/* if 8bit value must be used, do it */
		EmitOpPostByte(ctx, postbyte + 0x08);
		EmitOpDataByte(ctx, lobyte(result));
		ctx->m_CycleCount++;
		return;
	}

	/* if no offset is used, do it */
	if(0 == result) {

		/*
			Handle compatibility options for generating a postop forcing a
			zero offset rather than no offset
		*/
		if(false == hasExpression || (true == hasExpression && false == ctx->m_Compat.m_ForceZeroOffset)) {
			EmitOpPostByte(ctx, postbyte + 0x04);
			return;
		}
	}

		/* check for 5bit value */
	if(false == isIndirect) {
		if((result >= MIN_5BITS) && (result <= MAX_5BITS)) {
			postbyte &= 0x7f;
			postbyte += lobyte(result & 0x1f);
			EmitOpPostByte(ctx, postbyte);
			ctx->m_CycleCount++;
			return;
		}
	}

	/* check for 8bit or 16bit value */
	if((result >= MIN_BYTE) && (result <= MAX_BYTE)) {
		EmitOpPostByte(ctx, postbyte+0x08);	/* use 8bit value */
		EmitOpDataByte(ctx, lobyte(result));
		ctx->m_CycleCount++;
		return;
	}


	EmitOpPostByte(ctx, postbyte + 0x09);	/* use 16bit value */
	EmitOpDataWord(ctx, loword(result));
	ctx->m_CycleCount += 4;

	return;
}
