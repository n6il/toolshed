/*****************************************************************************
	proc_regtoreg.c	- Process register to register ops

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


void ProcRegToReg(EnvContext *ctx, const Mneumonic *op)
{
	REGISTER	src;
	REGISTER	dst;
	u_char		byte;

	EmitOpCode(ctx, op, 0);

	src = GetRegister(ctx, true, true);
	if(REG_ERROR == src) {
		error(ctx, ERR_SRCREG_REQUIRED, "invalid source register for '%s'", op->mnemonic);
		EmitOpPostByte(ctx, 0);
		return;
	}

	if(*ctx->m_Ptr++ != ',') {
		error(ctx, ERR_SYNTAX, "missing comma");
		EmitOpPostByte(ctx, 0);
		return;
	}

	ctx->m_Ptr = SkipConstWS(ctx->m_Ptr);

	dst = GetRegister(ctx, true, true);
	if(REG_ERROR == dst) {
		error(ctx, ERR_SRCREG_REQUIRED, "invalid destination register for '%s'", op->mnemonic);
		EmitOpPostByte(ctx, 0);
		return;
	}

	/* Check to warn of PCR/PC compatibility */
	if(REG_PCR == src) {
		warning(ctx, WARN_REGTOREG_PCR, "PCR as source register assumed to mean PC");
		src = REG_PC;
	}

	/* Check to warn of PCR/PC compatibility */
	if(REG_PCR == dst) {
		warning(ctx, WARN_REGTOREG_PCR, "PCR as destination register assumed to mean PC");
		dst = REG_PC;
	}

	/* Check for 0 register as dest usage */
	if(REG_ZERO == dst) {
		warning(ctx, WARN_REGTOREG_CONSTDST, "cannot transfer value into read only register '0'");
	}
	
	/* Check for 00 register as dest usage */
	if(REG_ZEROZERO == dst) {
		warning(ctx, WARN_REGTOREG_CONSTDST, "cannot transfer value into read only register '00'");
	}

	if(REG_ZERO != src && REG_ZEROZERO != src) {
		if((src >= 8 && dst <= 7) || (src <= 7 && dst >= 8)) {
			warning(ctx, WARN_REGTOREG_SIZEMISMATCH, "register size mismatch for '%s'", op->mnemonic);
		}
	} else {
		if(REG_PC != dst && REG_V != dst) {
			warning(ctx, WARN_OPTIMIZE, "optimization warning for using '0' register");
		}
	}

	byte = (u_char)(((src & REG_LSN_MASK) << 4) | (dst & REG_LSN_MASK));

	EmitOpPostByte(ctx, byte);
}


