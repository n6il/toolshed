/*****************************************************************************
	error.c	- Error reporting routines

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
#include "error.h"
#include "as.h"
#include "input.h"

/*----------------------------------------------------------------------------
	fatal --- fatal error handler
----------------------------------------------------------------------------*/

NORETURN static void dofatal(EnvContext *ctx, int error, const char *fmt, va_list list)
{
	if(GetOpenFileCount() > 0 && NULL != ctx) {
		fprintf(stderr, "%s(%lu) : ", GetCurrentFilePathname(), ctx->m_LineNumber);
	}
	vfprintf(stderr, fmt, list);
	fprintf(stderr, "\n");
	exit(CRITICAL_FATAL);
}

NORETURN void fatal(EnvContext *ctx, const char *str, ...)
{
	va_list list;

	va_start(list, str);

	dofatal(ctx, CRITICAL_FATAL, str, list);
}


/*----------------------------------------------------------------------------
	error --- error in a line. print line number and error
----------------------------------------------------------------------------*/
const char *internal_fname;
int internal_lineno;
NORETURN void internal_ex(EnvContext *ctx, const char *fmt, ...)
{
	va_list list;

	va_start(list, fmt);

	if(NULL != ctx) {
		fprintf(stderr, "%s\n", ctx->m_Line);
	}
	fprintf(stderr, "%s(%lu) : Internal assembler error: ", GetCurrentFilePathname(), ctx->m_LineNumber);
	vfprintf(stderr, fmt, list);
	fputc('\n', stderr);
	if(NULL != internal_fname) {
		fprintf(stderr, "casm file %s line %d\n", internal_fname, internal_lineno);
	}
	fprintf(stderr, "\n");
	exit(CRITICAL_INTER);
}



/*----------------------------------------------------------------------------
	error --- error in a line. print line number and error
----------------------------------------------------------------------------*/
void error(EnvContext *ctx, ERROR_NUMBER errnum, const char *str, ...)
{
	va_list list;


	va_start(list, str);

	if(NULL != ctx) {
		fprintf(stderr, "%s(%lu) : error A%4d : ", GetCurrentFilePathname(), ctx->m_LineNumber, errnum + 2100);
	}

	
	switch(errnum) {
	case ERR_SYNTAX:
		fprintf(stderr, "syntax error : ");
		break;

	case ERR_INVALID_MODE_FOR_OPERATION:
		fprintf(stderr, "invalid mode for operation : ");
		break;

	case ERR_PHASING:
		fprintf(stderr, "phasing error : ");
		break;

	case ERR_INVALID_REG_FOR_OPERATION:
		fprintf(stderr, "invalid register ");
		break;

	case ERR_INVALID_MODE_FOR_REGISTER:
		fprintf(stderr, "invalid mode for register : ");
		break;

	case ERR_MISMATCHED_COND:
		fprintf(stderr, "mismatched conditional directive : ");
		break;

	default:
		break;
	}

	vfprintf(stderr, str, list);
	fputc('\n', stderr);

	if(NULL != ctx) {
		fprintf(stderr, "%s\n", ctx->m_Line);
	}	ctx->m_ErrorCount++;

	if(ctx->m_ErrorCount > MAX_ERRORS) {
		va_list list;

		va_start(list, str);
		dofatal(ctx, CRITICAL_ERRORS, "Too many errors.  Stopping assembly", list);
	}
}




/*----------------------------------------------------------------------------
	warn --- trivial error in a line. print line number and error
----------------------------------------------------------------------------*/
void warning(EnvContext *ctx, WARNING_NUMBER warn, const char *str, ...)
{
	va_list list;

	/* Check for disabled warnings */
	if(false == ctx->m_Compat.m_Warn && WARN_COMPAT == warn) {
		return;
	}

	if(true == ctx->m_DisableWarnings) {
		if(WARN_COMPAT != warn) {
			return;
		}
	}

	if(2 == ctx->m_Pass && false == ctx->m_ListingFlags.m_OptEnabled) {
		return;    /* if not listing, don't */
	}

	va_start(list, str);

	/* repeat the warnings   */
	fprintf(stderr, "%s(%lu): warning : ", GetCurrentFilePathname(), ctx->m_LineNumber);
	vfprintf(stderr, str, list);
	fprintf(stderr, "\n");
	ctx->m_WarningCount++;
}


/*----------------------------------------------------------------------------
	note --- puts note and cumulative line # in listing
----------------------------------------------------------------------------*/
void note(EnvContext *ctx, const char *str, ...)
{
	if(true == ctx->m_ListingFlags.m_OptEnabled && 2 == ctx->m_Pass) {
		va_list list;
		va_start(list, str);

		fprintf(stderr, "%s(%lu): note : ", GetCurrentFilePathname(), ctx->m_LineNumber);
		vfprintf(stderr, str, list);
		fprintf(stderr, "\n");
	}
}
