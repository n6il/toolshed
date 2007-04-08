/*****************************************************************************
	output_bin.c	- code emitting for RS-DOS BIN files

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

#if 1
#define MAX_BIN_RECORD_SIZE		32768
#else
#define MAX_BIN_RECORD_SIZE		255
#endif

static u_int16	E_pc;							/* PC at beginning of collection	*/
static u_int16	E_Total;						/* total # bytes for one line			*/
static u_char	E_Bytes[MAX_BIN_RECORD_SIZE];	/* Emitted held bytes					*/
static FILE		*outputFile = NULL;
FILEGEN(bin, bin, bin, false);


static void filegen_bin_flush(EnvContext *ctx, u_int16 regpc)
{
	ASSERTX(2 == ctx->m_Pass);
	ASSERTX(NULL != outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	if(0 != E_Total) {
		fprintf(outputFile, "%c%c%c%c%c", 0x00, hibyte(E_Total), lobyte(E_Total), hibyte(E_pc), lobyte(E_pc));
		fwrite(E_Bytes, 1, E_Total, outputFile);
	}
	E_pc = regpc;
	E_Total = 0;
}


static void filegen_bin_addbyte(EnvContext *ctx, u_char val, u_int16 regpc)
{
	ASSERTX(2 == ctx->m_Pass);
	ASSERTX(NULL != outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	E_Bytes[E_Total++] = val;

	if(MAX_BIN_RECORD_SIZE == E_Total) {
		filegen_bin_flush(ctx, regpc);
	}

}

static void filegen_bin_finish(EnvContext *ctx, u_int16 regpc)
{
	filegen_bin_flush(ctx, regpc);
	fprintf(outputFile, "%c%c%c%c%c", 0xff, 0x00, 0x00, hibyte(ctx->m_EntryPoint), lobyte(ctx->m_EntryPoint));
	outputFile = NULL;
}

static void filegen_bin_init(EnvContext *ctx, FILE *outFile)
{
	ASSERTX(NULL == outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	outputFile = outFile;
	E_Total = 0;
	E_pc = 0;

}


