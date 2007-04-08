/*****************************************************************************
	proc_bitxfer.c	- Process bit transfer ops

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

void ProcBitTransfer(EnvContext *ctx, const Mneumonic *op)
{
	int32	result;
	REGISTER reg;
	u_char opcode;


	EmitOpCode(ctx, op, 0);		/* send out page 3 prebyte */

	reg = GetRegister(ctx, false, false);

	switch (reg) {
	case REG_CC:
		opcode = 0x00;
		break;
	case REG_ACCA:
		opcode = 0x40;
		break;
	case REG_ACCB:
		opcode = 0x80;
		break;
	default:
		opcode = 0;	/* Make compiler happy */
		error(ctx, ERR_INVALID_REG_FOR_OPERATION, "in operand for '%s'", op->mnemonic);
		break;
	}

	if (*ctx->m_Ptr++ != ',') {
		error(ctx, ERR_SYNTAX, "invalid format in operand for '%s'", op->mnemonic);
		return;
	}

	Evaluate(ctx, &result, EVAL_NORMAL, NULL);

	if (result > 7) {
		error(ctx, ERR_VALUE_TOO_BIG, "source bit number out of range in operand for '%s'", op->mnemonic);
		return;
	}

	opcode |= (result << 0x03);		/* shift register to correct postiton */

	if (*ctx->m_Ptr++ != ',') {
		error(ctx, ERR_SYNTAX, "invalid format in operand for '%s'", op->mnemonic);
		return;
	}

	Evaluate(ctx, &result, EVAL_NORMAL, NULL);

	if (result > 7) {
		error(ctx, ERR_VALUE_TOO_BIG, "destination bit number out of range in operand for '%s'", op->mnemonic);
		return;
	}

	opcode |= (result );		/* shift register to correct postiton */

	if (*ctx->m_Ptr++ != ',') {
		error(ctx, ERR_SYNTAX, "invalid format in operand for '%s'");
		return;
	}

	if (*ctx->m_Ptr == '#') {
		error(ctx, ERR_INVALID_MODE_FOR_OPERATION, "'%s' does not support immediate mode addressing", op->mnemonic);
		return;
	}

	Evaluate(ctx, &result, EVAL_NORMAL, NULL);

	if (((int)result) >= 0x100) {
		error(ctx, ERR_INVALID_MODE_FOR_OPERATION, "'%s' does not support extended mode addressing", op->mnemonic);
		return;
	}

	EmitOpPostByte(ctx, opcode);
	EmitOpDataByte(ctx, lobyte(result));
}

