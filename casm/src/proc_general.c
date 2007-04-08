/*****************************************************************************
	proc_general.c	- Process general addressing modes

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
 *	  ProcAddrMode --- process general addressing mode stuff
 */
void ProcGeneral(EnvContext *ctx, const Mneumonic *op)
{
	ADDR_MODE mode;

	/* pickup indicated addressing mode */
	mode = GetAddrMode(ctx);

	switch(op->optype) {
	case IMMEDIATE:
		if(IMMEDIATE != mode) {
			error(ctx, ERR_INVALID_MODE_FOR_OPERATION, "'%s' requires immediate addressing mode", op->mnemonic);
			return;
		}
		break;

	case NOIMM:
		if(IMMEDIATE == mode) {
			error(ctx, ERR_INVALID_MODE_FOR_OPERATION, "'%s' cannot be used with the immediate addressing mode", op->mnemonic);
			return;
		}
		break;

	case INDEXED:
		if(INDEXED != mode && INDIRECT != mode) {
			error(ctx, ERR_INVALID_MODE_FOR_OPERATION, "'%s' requires an indexed addressing mode", op->mnemonic);
			return;
		}
		ProcIndexed(ctx, op, 0);
		return;
		break;

	default:
		break;
	}

	switch(mode) {
	case IMMEDIATE:
		ProcImmediate(ctx, op);
		break;

	case INDEXED:
	case INDIRECT:
		ProcIndexed(ctx, op, 0x20);
		break;

	case DIRECT:
		ProcDirect(ctx, op);
		break;

	case EXTENDED:
		ProcExtended(ctx, op);
		break;

	default:
		internal((ctx, "Unknown Addressing Mode"));
		break;
	}
}
