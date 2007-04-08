/*****************************************************************************
	export.c	- Symbol export management

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

static Export	*exportList = NULL;
static Export	*exportTail = NULL;
static int		exportCount = 0;
static int		exportMMUPage = -1;
static u_int16	exportAddress;


void SetExportAddress(const int mmuPage, const u_int16 address)
{
	exportMMUPage = mmuPage;
	exportAddress = address;
}

void InitExports(EnvContext *ctx)
{
	ASSERT(MODBIN == ctx->m_OutputType);
	AddExport(NULL, "initmod");
}

void AddExport(EnvContext *ctx, const char *name)
{
	Export *exp;

	ASSERTX(1 == ctx->m_Pass);

	/* See if the export already exists */
	exp = exportList;
	while(NULL != exp) {
		if(0 == strcmp(exp->name, name)) {
			warning(ctx, WARN_DBLEXPORT, "%s already exported");
			return;
		}
		exp = exp->next;
	}

	/* Allocate a new export struct */
	exp = (Export*)AllocMem(sizeof(Export));
	exp->name = (char*)AllocMem(strlen(name) + 1);

	/* Create it */
	strcpy(exp->name, name);
	exp->next = NULL;


	/* Add it to the list */
	if(NULL == exportTail) {
		exportList = exp;
		exportTail = exp;
	} else {
		exportTail->next = exp;
		exportTail = exp;
	}

	exportCount++;
}


static void EmitExportWord(FILE *output, u_int16 word)
{
	fputc((word & 0xff00) >> 8, output);
	fputc((word & 0xff), output);
}

void EmitExports(EnvContext *ctx, FILE *output)
{
	Export *exp;

	ASSERTX(MODBIN == ctx->m_OutputType);

	if(-1 == exportMMUPage) {
		fputc(0x00, output);
		fputc(0x00, output);
	} else {
		fputc(0xff, output);
		fputc(exportMMUPage, output);
	}
	EmitExportWord(output, exportAddress);
	EmitExportWord(output, loword(exportCount));

	/* Write the exports */
	exp = exportList;
	while(NULL != exp) {
		Symbol *sym;

		sym = FindSymbol(NULL, exp->name, true, true);
		if(NULL == sym) {
			error(ctx, ERR_UNDEFINED, "symbol '%s' was declared for export but is not defined", exp->name);
		} else {
			/* Write the export */
			EmitExportWord(output, (u_int16)sym->value);
			fputc((int)strlen(exp->name), output);
			fputs(exp->name, output);
		}

		exp = exp->next;
	}


}



