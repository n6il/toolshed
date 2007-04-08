/*****************************************************************************
	output_rom.c	- code emitting for ROM and RAW files

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

#define MAX_ROM_RECORD_SIZE	512

static FILE			*outputFile = NULL;
static u_int16		E_Total;						/* total # bytes for one line			*/
static u_char		E_Bytes[MAX_ROM_RECORD_SIZE];	/* Emitted held bytes					*/



FILEGEN(rom, rom, rom, true);
FILEGEN(rom, raw, raw, true);



static void filegen_rom_init(EnvContext *ctx, FILE *outFile)
{
	ASSERTX(NULL == outputFile);
	outputFile = outFile;
	E_Total = 0;
}


static void filegen_rom_addbyte(EnvContext *ctx, u_char val, u_int16 regpc)
{
	ASSERTX(2 == ctx->m_Pass);
	ASSERTX(NULL != outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	E_Bytes[E_Total++] = val;

	if(MAX_ROM_RECORD_SIZE == E_Total) {
		filegen_rom_flush(ctx, regpc);
	}
}


static void filegen_rom_flush(EnvContext *ctx, u_int16 regpc)
{
	ASSERTX(2 == ctx->m_Pass);
	ASSERTX(NULL != outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	if(E_Total > 0) {
		if(true == ctx->m_Misc.Oflag) {
			fwrite(E_Bytes, 1, E_Total, outputFile);
		}
		E_Total = 0;
	}
}


static void filegen_rom_finish(EnvContext *ctx, u_int16 regpc)
{
	ASSERTX(2 == ctx->m_Pass);
	ASSERTX(NULL != outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	/* Fluish the output */
	filegen_rom_flush(ctx, regpc);

	if(ROMBIN == ctx->m_OutputType) {
		int length;

		length = ftell(outputFile);
		if(length > outputROMSize) {
			warning(ctx, WARN_ROMSIZE, "ROM file is %d bytes larger than requested ROM size", length - outputROMSize);
		}
		
		else {
			while(length < outputROMSize) {
				fputc(0, outputFile);
				length++;
			}
		}
	}

	outputFile = NULL;
}

