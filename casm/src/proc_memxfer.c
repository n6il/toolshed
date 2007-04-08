/*****************************************************************************
	proc_memxfer.c	- Process memory transfer ops

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


typedef enum {
	RXFER_NONE,
	RXFER_ADD,
	RXFER_SUB,
} RXFER_OP;

static RXFER_OP GetMemXferOp(EnvContext *ctx)
{
	RXFER_OP op;

	if('+' == *ctx->m_Ptr) {
		op = RXFER_ADD;
		ctx->m_Ptr++;
	}
	
	else if(*ctx->m_Ptr == '-') {
		op = RXFER_SUB;
		ctx->m_Ptr++;
	}
	else {
		op = RXFER_NONE;
	}

	return op;
}


void ProcMemXfer2(EnvContext *ctx, const Mneumonic *op)
{
	REGISTER	srcReg;	/* source register */
	REGISTER	dstReg;	/* destination register */
	u_char		postop;

	srcReg = GetRegister(ctx, false, true);	/* get first register */
	if(REG_ERROR == srcReg) {
		error(ctx, ERR_INVALID_SRCREG, "invalid register for memory transfer operand");
		return;
	}


	if(*ctx->m_Ptr++ != ',') {
		error(ctx, ERR_SYNTAX, "missing ','");
		return;
	}

	dstReg = GetRegister(ctx, false, true);		/* get first register */
	if(REG_ERROR == dstReg) {
		error(ctx, ERR_INVALID_DSTREG, "invalid register for memory transfer operand");
		return;
	}

	postop = ((lobyte(srcReg) & 0x0f) << 0x04) | (lobyte(dstReg) & 0x0f);

	EmitOpCode(ctx, op, 0);
	EmitOpPostByte(ctx, postop);
}


void ProcMemXfer(EnvContext *ctx, const Mneumonic *op)
{
	REGISTER	srcReg;	/* source register */
	REGISTER	dstReg;	/* destination register */
	RXFER_OP	srcOp;	/* Source operation */
	RXFER_OP	dstOp;	/* Destination operation */
	u_char		postop;
	u_char		opcode;

	srcReg = GetRegister(ctx, false, true);	/* get first register */
	if(REG_ERROR == srcReg) {
		error(ctx, ERR_INVALID_SRCREG, "invalid register for '%s'", op->mnemonic);
		return;
	}

	srcOp = GetMemXferOp(ctx);

	if(*ctx->m_Ptr++ != ',') {
		error(ctx, ERR_SYNTAX, "missing ',' in memory transfer operand");
		return;
	}

	dstReg = GetRegister(ctx, false, true);		/* get first register */
	if(REG_ERROR == dstReg) {
		error(ctx, ERR_INVALID_DSTREG, "invalid register for '%s'", op->mnemonic);
		return;
	}

	dstOp = GetMemXferOp(ctx);

	postop = ((lobyte(srcReg) & 0x0f) << 0x04) | (lobyte(dstReg) & 0x0f);
	opcode = op->opcode;

	if(RXFER_ADD == srcOp && RXFER_ADD == dstOp) {
		opcode += 0;
	}
	else if(RXFER_SUB == srcOp && RXFER_SUB == dstOp) {
		opcode += 1;
	}
	else if(RXFER_ADD == srcOp && RXFER_NONE == dstOp) {
		opcode += 2;
	}
	else if(RXFER_NONE == srcOp && RXFER_ADD == dstOp) {
		opcode += 3;
	} else {
		error(ctx, ERR_INVALID_MODE_FOR_OPERATION, "invalid direction for '%s'", op->mnemonic);
	}

	EmitOpCode2(ctx, PAGE3, opcode);
	EmitOpPostByte(ctx, postop);
}
