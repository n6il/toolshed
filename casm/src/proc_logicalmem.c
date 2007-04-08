/*****************************************************************************
	proc_logicalmem.c	- Process logical memory ops

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


void ProcLogicalMem(EnvContext *ctx, const Mneumonic *op)
{
	int32		result;
	int			amode;
	int32		logValue;

	/* get first argument */
	Evaluate(ctx, &logValue, EVAL_NORMAL, NULL);

	if(logValue > 0xff) {
		warning(ctx, WARN_VALUE_TRUNCATED, "immediate value for logical operation too big - value truncated");
	}

	logValue &= 0xff;


	/*
		Here we support 3 dialects of the logical mem operation:
	
			,	-	H.K. dialect
			;	-	CCASM Dialect
			:	-	CLS-97 dialect
	*/
	if(',' != *ctx->m_Ptr && ';' != *ctx->m_Ptr && ':' != *ctx->m_Ptr) {
		/* FIXME - warn compat */
		error(ctx, ERR_SYNTAX, "value for logical memory operation must be followed by a comma, semi-colon, or colon");
		return;
	} 

	ctx->m_Ptr++;
	ctx->m_Operand = ctx->m_Ptr;	/* Move string down to start of buffer */

	amode = GetAddrMode(ctx);

	switch(amode) {
	case INDEXED:
		EmitOpCode(ctx, op, 0x60);
		EmitOpPostByte(ctx, lobyte(logValue));			/* send out logresult */
		ProcIndexed(ctx, NULL, 0);
		break;

	case INDIRECT:
		EmitOpCode(ctx, op, 0x60);
		EmitOpPostByte(ctx, lobyte(logValue));			/* send out logresult */
		ProcIndexed(ctx, NULL, 0);

		break;

	case IMMEDIATE:
		error(ctx, ERR_INVALID_MODE_FOR_OPERATION, "immediate addressing mode not allowed for '%s'", op->mnemonic);
		break;

	case EXTENDED:
	case DIRECT:
		Evaluate(ctx, &result, EVAL_NORMAL, NULL);

		if(true == ctx->m_ForceByte || true == IsAddressInDirectPage(loword(result))) {
			EmitOpCode(ctx, op, 0);
			EmitOpPostByte(ctx, lobyte(logValue));
			EmitOpPostByte(ctx, lobyte(result));
			ctx->m_CycleCount += 2;
		} else {
			EmitOpCode(ctx, op, 0x70);
			EmitOpPostByte(ctx, lobyte(logValue));
			EmitOpAddr(ctx, loword(result));
			ctx->m_CycleCount += 3;
		}
		break;

	default:
		internal((ctx, "ProcLogicalMem(): Unknown addressing mode!"));
		break;
	}
}


