/*****************************************************************************
	output_listing.c	- formatted listing output routines

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

/*----------------------------------------------------------------------------
	PrintLine --- pretty print input line
----------------------------------------------------------------------------*/
void PrintLine(EnvContext *ctx, const char *lineBuffer, bool forceComment)
{
	int			i;
	const char	*ptr;
	char		optchar;

	i = 0;

	if(true == forceComment) {
		return;
	}

	if(ctx->m_OptimizeLine == true) {
		optchar = '>';
	} else {
		optchar = SPACE;
	}

	if(false == ctx->m_ListingFlags.OptNoLineNumbers) {
		fprintf(stdout, "%5d%c ", (int)ctx->m_LineNumber, optchar);
	}

	if(false == ctx->m_ListingFlags.OptNoOpData) {
		if(ctx->m_ListingFlags.P_total || true == ctx->m_ListingFlags.P_force || true == IsMacroOpen()) {
			fprintf(stdout, "%04x ", GetOldPCReg());
		}

		else {
			fprintf(stdout, "     ");
		}

		i = 0;

		if(i < ctx->m_ListingFlags.P_total) {
			if(ctx->m_ListingFlags.P_total > 1 && (PAGE2 == ctx->m_ListingFlags.P_bytes[0] || PAGE3 == ctx->m_ListingFlags.P_bytes[0])) {
				fprintf(stdout, "%02X", ctx->m_ListingFlags.P_bytes[i++]);
			}
			fprintf(stdout, "%02X", ctx->m_ListingFlags.P_bytes[i++]);

			/*
			fputc(SPACE, stdout);
			if(1 == i) {
				fputc(SPACE, stdout);
				fputc(SPACE, stdout);
			}
			*/
		}

		for(;i < ctx->m_ListingFlags.P_total && i < 6 ;i++) {
			fprintf(stdout, "%02x", lobyte(ctx->m_ListingFlags.P_bytes[i]));
		}

		for(; i < 6; i++) {
			fprintf(stdout, "  ");
		}

		fprintf(stdout, "  ");

		if(ctx->m_Misc.Cflag && ctx->m_CycleCount) {
			fprintf(stdout, "[%2d] ", (int)ctx->m_CycleCount);
		}

		else {
			fprintf(stdout, "       ");
		}
	}

	if(true == forceComment) {
		putchar('*');
	}

	ptr = lineBuffer;
	while( *ptr != EOS ) {
		putchar(*ptr++);   /* just echo the line back out */
	}

	if(false == ctx->m_ListingFlags.OptNoOpData) {
		for(; i < ctx->m_ListingFlags.P_total; i++) {
			if(0 == (i % 6)) {
				fprintf(stdout, "\n            ");
			}
			fprintf(stdout, "%02x", ctx->m_ListingFlags.P_bytes[i]);
		}
	}
	fprintf(stdout, "\n");
}

/*----------------------------------------------------------------------------
	PrintCycles --- print # of cycles counted so far
----------------------------------------------------------------------------*/
void PrintCycles(EnvContext *ctx, const int cycles)
{
	if(ctx->m_Pass == 2 && 0 != cycles) {
		fprintf(stdout, "  ctx->m_CycleCount Counted:  %d\n\n", cycles);
	}
}


