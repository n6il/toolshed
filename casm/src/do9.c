/*****************************************************************************
	do9.c	- Instruction assembly

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
#include "proto.h"
#include "proc_util.h"




const char *GetAddressingModeName(ADDR_MODE mode)
{
	const char *name;

	switch(mode) {
	case INDIRECT:	name = "indirect";			break;	/* Indirect addressing [$xxxx]		*/
	case EXTENDED:	name = "extended";			break;	/* extended							*/
	case DIRECT:	name = "direct";			break;	/* Direct Page access				*/
	case INDEXED:	name = "indexed";			break;	/* Indexed only						*/
	case INHERENT:	name = "inherent";			break;	/* Inherent							*/
	case IMMEDIATE:	name = "immediate";			break;	/* Immediate only					*/
	case GENERAL:	name = "general";			break;	/* General addressing				*/
	case REL_SHORT:	name = "short relative";	break;	/* Short Relative					*/
	case REL_LONG:	name = "long relative";		break;	/* Long Relative					*/
	case NOIMM:		name = "no immediate";		break;	/* General except for Immediate		*/
	case RTOR:		name = "reg to reg";		break;	/* Register To Register				*/
	case REGLIST:	name = "push/pull";			break;	/* Register List					*/
	case GRP2:		name = "Group2";			break;	/* Group 2 (Read/Modify/Write)		*/
	case LOGMEM:	name = "logical memory";	break;	/* 6309 logical memory ops			*/
	case BTM:		name = "bit transfer";		break;	/* Bit transfer and manipulation	*/
	case MRTOR:		name = "reg to reg2";		break;	/* TFM memory transfers				*/
	case MRTOR2:	name = "reg to reg2";		break;	/* TFM memory transfers - CCASM		*/
	default:
		name = "UNKNOWN";
		break;
	}

	return name;
}


static void ProcGroup2(EnvContext *ctx, const Mneumonic *op)
{
	int32		result;
	ADDR_MODE	mode;

	/* pickup indicated addressing mode */
	mode = GetAddrMode(ctx);

	if(INDEXED == mode || INDIRECT == mode) {
		ProcIndexed(ctx, op, 0x60);
		return;
	}

	if(DIRECT != mode && EXTENDED != mode) {
		error(ctx, ERR_INVALID_MODE_FOR_OPERATION, "%s addressing mode not allowed for %s", GetAddressingModeName(mode), op->mnemonic);
		return;
	}

	Evaluate(ctx, &result, EVAL_NORMAL, NULL);

	if(true == ctx->m_ForceByte || (false == ctx->m_ForceWord && true == IsAddressInDirectPage(loword(result)))) {
		EmitOpCode(ctx, op, 0);
		EmitOpDataByte(ctx, lobyte(result));
		ctx->m_CycleCount += 2;
		return;
	}

	/* Default to extended */
	EmitOpCode(ctx, op, 0x70);
	EmitOpAddr(ctx, loword(result));
	ctx->m_CycleCount += 3;

}


/*
 *	  ProcOpcode --- process mnemonic
 *
 *	Called with the base opcode and it's class. operandPtr points to
 *	the beginning of the operand field.
 */
void ProcOpcode(EnvContext *ctx, const Mneumonic *op)
{
	ctx->m_OptimizeLine = false;
	
	switch(op->optype) {
	case GENERAL:
	case NOIMM:
	case IMMEDIATE:
	case INDEXED:
		ProcGeneral(ctx, op);
		break;


	case INHERENT:
		/* inherent addressing */
		ProcInherent(ctx, op);
		break;
		
	case REL_SHORT:
		/* short relative branches */
		ProcShortBranch(ctx, op);
		break;
		
	case REL_LONG:
		/* long relative branches */
		ProcLongBranch(ctx, op);
		break;

	case RTOR:
		/* tfr and exg */
		ProcRegToReg(ctx, op);
		break;
		
	case REGLIST:
		/* pushes and pulls */
		ProcPushPull(ctx, op);
		break;
		
	case GRP2:
		ProcGroup2(ctx, op);
		break;
		
	case LOGMEM:
		ProcLogicalMem(ctx, op);
		break;

	case BTM:
		ProcBitTransfer(ctx, op);
		break;

	case MRTOR:
		ProcMemXfer(ctx, op);
		break;

	case MRTOR2:
		ProcMemXfer2(ctx, op);
		break;

	default:
		internal((ctx, "Error in Mnemonic table (op=%s / %d)", op->mnemonic, op->opcode));
		break;
	}
}

