/*****************************************************************************
	proc_pushpull.c	- Process stack push/pull ops

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



/* convert tfr/exg reg number into psh/pul format */
static int	 ppRegBits[17] =	{ 0x06,		/* REG_ACCD */
								  0x10,		/* REG_INDX */
								  0x20,		/* REG_INDY */
								  0x40,		/* REG_USTACK */
								  0x40,		/* REG_SSTACK */
								  0x80,		/* REG_PC */
								  0x00,		/* REG_ACCW *INVALID */
								  0x00,		/* REG_V *INVALID */
								  0x02,		/* REG_ACCA */
								  0x04,		/* REG_ACCB */
								  0x01,		/* REG_CC */
								  0x08,		/* REG_DP */
								  0x00,		/* REG_ZERO		* Invalid */
								  0x00,		/* REG_ZEROZERO	* Invalid */
								  0x00,		/* REG_ACCE		* Invalid */
								  0x00,		/* REG_ACCF		* Invalid */
								  0x80		/* REG_PCR		* Invalid */
								};
static int	 ppRegCycles[17]=	{ 0x02,
								  0x02,
								  0x02,
								  0x02,
								  0x02,
								  0x02,
								  0x00,
								  0x00,
								  0x01,
								  0x01,
								  0x01,
								  0x01,
								  0x00,
								  0x00,
								  0x00,
								  0x00,
								  0x00
								};



 void ProcPushPull(EnvContext *ctx, const Mneumonic *op)
{
	REGISTER	reg;
	u_char		pbyte;

	ctx->m_CycleCount = GetCycleCount(ctx, op, IMMEDIATE, ctx->m_CPUType);

	if(EOS == *ctx->m_Ptr) {
		error(ctx, ERR_SYNTAX, "No registers specified for %s operation", op->mnemonic);
		return;
	}

	pbyte = 0;
	do {
		/* ctx->m_Ptr = SkipConstWS(ctx->m_Ptr); */

		reg = GetRegister(ctx, false, false);
		switch(reg) {
		case REG_ERROR:
			error(ctx, ERR_ILLEGAL_REGISTER_NAME, "invalid register supplied for '%s'", op->mnemonic);
			return;

		case REG_PCR:
			if(false == ctx->m_Compat.m_DisablePCIndex) {
				warning(ctx,
						WARN_COMPAT,
						"Some assemblers do not accept '%s' as a valid register for '%s' operations",
						GetRegisterName(reg),
						op->mnemonic);
				break;
			}

		case REG_ACCW:
		case REG_V:
		case REG_ZERO:
		case REG_ZEROZERO:
		case REG_ACCE:
		case REG_ACCF:
			error(ctx, ERR_ILLEGAL_REGISTER_NAME, "illegal register '%s' supplied for '%s'", GetRegisterName(reg), op->mnemonic);
			break;

		case REG_SSTACK:
			if(op->opcode == OP_PSHS || op->opcode == OP_PULS) {
				error(ctx, ERR_ILLEGAL_REGISTER_NAME, "illegal register '%s' supplied for '%s'", GetRegisterName(reg), op->mnemonic);
			}

			break;

		case REG_USTACK:
			if(op->opcode == OP_PSHU || op->opcode == OP_PULU) {
				error(ctx, ERR_ILLEGAL_REGISTER_NAME, "illegal register '%s' supplied for '%s'", GetRegisterName(reg), op->mnemonic);
			}

			break;

		default:
			/* Make the compiler happy */
			break;
		}

		pbyte |= ppRegBits[reg];
		ctx->m_CycleCount += ppRegCycles[reg];

	} while(*(ctx->m_Ptr++) == ',' );

	EmitOpCode(ctx, op, 0);
	EmitOpPostByte(ctx, pbyte);
}


