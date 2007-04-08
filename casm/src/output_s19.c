/*****************************************************************************
	output_s19.c	- code emitting for Motorola S files

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

#define MAX_S19_RECORD_SIZE		32

static FILE		*outputFile = NULL;
static u_int16	E_Total;						/* total # bytes for one line			*/
static u_int16	E_pc;							/* PC at beginning of collection	*/
static u_char	E_Bytes[MAX_S19_RECORD_SIZE];	/* Emitted held bytes					*/
FILEGEN(S19, S19, s19, true);


static void HexOut(EnvContext *ctx, FILE *out, const u_char byte)
{
	if(NULL != out) {
		static char *hexstr = { "0123456789ABCDEF" } ;

		if(ctx->m_Misc.Oflag == true) {
			fprintf(out, "%c%c", hexstr[(byte >> 4) & 0x0f], hexstr[byte & 0x0f]);
		}
	}
}


static void filegen_S19_init(EnvContext *ctx, FILE *outFile)
{
	outputFile = outFile;
	E_Total = 0;
	E_pc = 0;
}


static void filegen_S19_flush(EnvContext *ctx, u_int16 pcreg)
{
	if(E_Total > 0) {
		int stat;
		int i;
		int chksum;

		chksum =  E_Total + 3;    /* total bytes in this record */
		chksum += lobyte(E_pc);
		chksum += hibyte(E_pc);

		stat = fprintf(outputFile, "S1");   /* record header preamble */
		if(stat != 2) {
			fatal(ctx, "Error writing file");
			return;
		}

		HexOut(ctx, outputFile, lobyte(E_Total + 3));	/* byte count +3 */
		HexOut(ctx, outputFile, hibyte(E_pc));	/* high byte of PC */
		HexOut(ctx, outputFile, lobyte(E_pc));	/* low byte of PC */

		for(i=0;i<E_Total;i++) {
			chksum += E_Bytes[i];
			HexOut(ctx, outputFile, E_Bytes[i]);    /* data byte */
		}

		/* Output the checksum */
		chksum =~ chksum;			/* one's complement */
		HexOut(ctx, outputFile, lobyte(chksum));	/* checksum */

		stat = fprintf(outputFile, "\n");
		if( stat < 0 ) {
			fatal(ctx, "error writing file");
		}
	}

	E_Total = 0;
	E_pc = pcreg;
}



/*----------------------------------------------------------------------------
	filegen_s19_record --- flush record out in `S1' format
----------------------------------------------------------------------------*/
static void filegen_S19_addbyte(EnvContext *ctx, u_char val, u_int16 pcreg)
{
	ASSERTX(2 == ctx->m_Pass);
	ASSERTX(NULL != outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	E_Bytes[E_Total++] = val;

	if(MAX_S19_RECORD_SIZE == E_Total) {
		filegen_S19_flush(ctx, pcreg);
	}

}


/*----------------------------------------------------------------------------
	filegen_S19_end --- write the final 'S9' record
----------------------------------------------------------------------------*/
static void filegen_S19_finish(EnvContext *ctx, u_int16 regpc)
{
	int     stat;
	int     chksum;

	ASSERTX(2 == ctx->m_Pass);
	ASSERTX(NULL != outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	stat = fprintf(outputFile, "S9");		/* record header preamble */
	if( stat != 2 ) {
		fatal(ctx, "error writing object file");
	}

	chksum = 3;
	chksum += hibyte(ctx->m_EntryPoint);
	chksum += lobyte(ctx->m_EntryPoint);
	chksum =~ chksum;				/* one's complement */
	HexOut(ctx, outputFile, 3);					/* byte count +1 */
	HexOut(ctx, outputFile, hibyte(ctx->m_EntryPoint));	/* high byte of entry addres */
	HexOut(ctx, outputFile, lobyte(ctx->m_EntryPoint));	/* low byte of entry address */
	HexOut(ctx, outputFile, lobyte(chksum)); /* checksum */

	stat = fprintf(outputFile, "\n");
	if(stat < 0) {
		fatal(ctx, "Error writing object file");
		return;
	}
}


