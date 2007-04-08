/*****************************************************************************
	context.c	- Context helper functions

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
#include "context.h"
#include "error.h"
#include "proto.h"
#include "util.h"
#include "label.h"
#include "table9.h"


bool GetConditionalState(EnvContext *ctx)
{
	bool condition;

	/* If the current if/else/endif condition is */
	if(NO_CONDITION == ctx->m_Cond.m_Count) {
		condition = true;
	} else {
		condition = ctx->m_Cond.m_Cond[ctx->m_Cond.m_Count].m_State;
	}

	return condition;
}


PSEUDO_OP GetConditionalOp(EnvContext *ctx)
{
	ASSERTX(NO_CONDITION != ctx->m_Cond.m_Count);

	return ctx->m_Cond.m_Cond[ctx->m_Cond.m_Count].m_Op;
}


int GetConditionalCount(EnvContext *ctx)
{
	return ctx->m_Cond.m_Count;
}


void PushConditionalState(EnvContext *ctx, const char *opname, PSEUDO_OP op)
{
	int32	result;
	bool	cond;

	if(IFP1 != op && IFP2 != op && IFDEF != op && IFNDEF != op ) {
		Evaluate(ctx, &result, 1 == ctx->m_Pass ? EVAL_NOERRORS : EVAL_NORMAL, NULL);
	} else {
		result = 0;
	}

	switch(op) {
	case IFP1:	/* If assembler pass = 1 */
		cond = (ctx->m_Pass == 1 ? true : false);
		break;

	case IFP2:	/* If assembler pass = 2 */
		cond = (ctx->m_Pass == 2 ? true : false);
		break;

	case IFEQ:
		cond = (result == 0 ? true : false);
		break;

	case IFNE:	/* Not equal */
		cond = (result != 0 ? true : false);
		break;

	case IFLT:	/* Less than */
		cond = (result < 0 ? true : false);
		break;

	case IFLE:	/* Less than or equal */
		cond = (result <= 0 ? true : false);
		break;

	case IFGT:	/* Greater than */
		cond = (result > 0 ? true : false);
		break;

	case IFGE:	/* Creater than or equal */
		cond = (result >= 0 ? true : false);
		break;

	case IF:	/* if value is not 0, assemble */
	case COND:
		cond = (result != 0 ? true : false);
		break;

	case IFN:	/* if value is 0, assemble */
		cond = (result == 0 ? true : false);
		break;

	case IFDEF:
	case IFNDEF:
		{
			char label[MAX_LABELSIZE * 3];
			char *dst;
			Symbol *sym;

			if(false == IsLabelStart(ctx, *ctx->m_Ptr)) {
				error(ctx, ERR_SYNTAX, "invalid label for %s conditional expression", opname);
				return;
			}

			dst = label;
			while(true == IsLabelChar(ctx, *ctx->m_Ptr)) {
				*(dst++) = *(ctx->m_Ptr++);
			}
			*dst = 0;

		
			sym = FindSymbol(ctx, label, true, false);
			if(IFDEF == op) {
				cond = (NULL != sym ? true : false);
			} else {
				cond = (NULL == sym ? true : false);
			}
		}
		break;

	default:
		cond = false;	/* Make compiler happy */
		internal((ctx, "error processing conditional directive for %s", opname));
	}

	if(ctx->m_Cond.m_Count >= MAX_CONDITIONS) {
		error(ctx, ERR_GENERAL, "too many conditional directives");
	}

	ctx->m_Cond.m_Count++;
	ctx->m_Cond.m_Cond[ctx->m_Cond.m_Count].m_Op = op;
	ctx->m_Cond.m_Cond[ctx->m_Cond.m_Count].m_State = cond;
}


void PopConditionalState(EnvContext *ctx)
{
	ASSERTX(NO_CONDITION != ctx->m_Cond.m_Count);

	if(ctx->m_Cond.m_Count >= 0) {
		ctx->m_Cond.m_Count--;
	}
}


bool ToggleConditionalState(EnvContext *ctx)
{
	int count;
	bool state;

	ASSERTX(NO_CONDITION != ctx->m_Cond.m_Count);

	/* Get the count */
	count = ctx->m_Cond.m_Count;

	/* Get the current state */
	state = ctx->m_Cond.m_Cond[count].m_State;

	/* toggle it */
	state = (false == state ? true : false);

	/* Save the new state */
	ctx->m_Cond.m_Cond[count].m_State = state;

	/* Return the new state */
	return state;
}

