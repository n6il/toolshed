/*****************************************************************************
	proc_immediate.c	- Process immediate addressing modes

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




void ProcImmediate(EnvContext *ctx, const Mneumonic *op)
{
	int32 result;

	ctx->m_Ptr++;
	Evaluate(ctx, &result, EVAL_NORMAL, NULL);


	/* Emit the opcode */
	if(LDQ_OPIMM == op->opcode && PAGE2 == op->page) {
		EmitOpCodeEx(ctx, op->opcode);
	} else {
		EmitOpCode(ctx, op, 0);
	}

	switch(op->immsize) {
	case BIT_8:
		if(result > 0xff) {
			warning(ctx, WARN_VALUE_TRUNCATED, "immediate value too large for operation - value truncated");
		}
		EmitOpDataByte(ctx, lobyte(result));
		break;

	case BIT_16:
		if(0 != hiword(result) && 0xffff != hiword(result)) {
			warning(ctx, WARN_VALUE_TRUNCATED, "immediate value is too large for operation - value truncated");
		}
		EmitOpDataWord(ctx, loword(result));
		break;

	case BIT_32:
		EmitOpDataLong(ctx, result);
		break;

	case BIT_0:
		ASSERT(0);
		internal((ctx, "ProcImmediate(): Unknown error encountered!"));
		break;
	}
}
