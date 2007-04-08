/*****************************************************************************
	proc_extended.c	- Process extended addressing ops

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


void ProcExtended(EnvContext *ctx, const Mneumonic *op)
{
	int32 result;
	u_char modifier;
	bool status;

	status = Evaluate(ctx, &result, EVAL_NORMAL, NULL);
	if(false == status && 2 == ctx->m_Pass) {
		ctx = ctx;
	}

	if(LDQ_OPIMM == op->opcode && PAGE2 == op->page) {
		modifier = lobyte((LDQ_OPOTH - LDQ_OPIMM) - 0x10);
	} else {
		modifier = 0;
	}

	if(false == ctx->m_ForceWord && true == IsAddressInDirectPage(loword(result))) {
		ctx->m_ForceByte = true;
	}

	else if(false == ctx->m_ForceWord && (result >= 0 && result <= 0xFF)) {
		ctx->m_ForceByte = true;
	}
	
	else if(false == ctx->m_ForceByte) {
		ctx->m_ForceWord = true;
	}
	
	else if(true == ctx->m_ForceByte && true == ctx->m_ForceWord) {
		warning(ctx, WARN_BUGALERT, "possible assembler bug! conflict deciding between direct or extended mode.");
		warning(ctx, WARN_BUGALERT, "forcing to extended addressing mode");
		ctx->m_ForceWord = true;
		ctx->m_ForceByte = false;
	}


	if(true == ctx->m_ForceByte) {
		ASSERTX(false == ctx->m_ForceWord);
		EmitOpCode(ctx, op, modifier + 0x10);
		EmitOpPostByte(ctx, lobyte(result));	/* FIXME - emitopaddrbyte */
		ctx->m_CycleCount += 2;
	} else {
		ASSERTX(false == ctx->m_ForceByte);
		EmitOpCode(ctx, op, modifier + 0x30);
		EmitOpAddr(ctx, loword(result));
		ctx->m_CycleCount += 3;
	}
}
