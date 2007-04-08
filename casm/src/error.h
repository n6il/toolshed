/*****************************************************************************
	error.h	- Declarations for error reporting

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
#ifndef ERROR_H
#define ERROR_H

#include "config.h"
#include "context.h"
/*
	Critical errors

*/

#define CRITICAL_USER	1	/* An error from the user */
#define CRITICAL_FATAL	2	/* Fatal error encountered */
#define CRITICAL_INTER	3	/* Internal compiler error */
#define CRITICAL_ERRORS	4	/* Too many errors reported */


typedef enum {
	ERR_ERROR = -1,
	ERR_SUCCESS = 0,
	ERR_GENERAL,
	ERR_SYNTAX,
	ERR_SUBSCRIPT_OUT_OF_RANGE,
	ERR_INVALID_LABEL,
	ERR_LABEL_TOO_LONG,
	ERR_VALUE_TOO_BIG,
	ERR_UNDEFINED,
	ERR_REDEFINED,
	ERR_PHASING,	
	ERR_INVALID_MODE_FOR_OPERATION,
	ERR_INVALID_MODE_FOR_REGISTER,
	ERR_INVALID_REG_FOR_OPERATION,
	ERR_INVALID_SRCREG,
	ERR_INVALID_DSTREG,
	ERR_SRCREG_REQUIRED,
	ERR_DSTREG_REQUIRED,
	ERR_ILLEGAL_REGISTER_NAME,
	ERR_MISMATCHED_COND,
	ERR_INVALID_MEMBER,
} ERROR_NUMBER;

typedef enum {
	WARN_LINESIZE,
	WARN_MISMATCHED_COND,
	WARN_COMPAT,		/* Compatibility issues */
	WARN_OS9SYSCALL,
	WARN_BUGALERT,
	WARN_PSECTUSAGE,
	WARN_VSECTUSAGE,
	WARN_DBLEXPORT,
	WARN_ENDSECTUSAGE,
	WARN_NONAMESPACE,
	WARN_DPRANGE,
	WARN_DPSTACK,
	WARN_EMPTYSTRUCT,
	WARN_ENTRYPOINT,
	WARN_LABELSIZE,
	WARN_OPTIMIZE,
	WARN_REGTOREG_SIZEMISMATCH,
	WARN_REGTOREG_CONSTDST,
	WARN_REGTOREG_PCR,
	WARN_OS9HEADER,
	WARN_VALUE_TRUNCATED,
	WARN_PREDEF,
	WARN_ROMSIZE,
	WARN_UNINITUSAGE,
	WARN_MACROPARAMS,
	WARN_SEGMENTUSAGE, 
} WARNING_NUMBER;

NORETURN void internal_ex(EnvContext *ctx, const char *str, ...);
extern const char *internal_fname;
extern int internal_lineno;
#define internal(x)	\
	{ internal_fname = __FILE__;\
	  internal_lineno = __LINE__;\
	  internal_ex x; }
#define ASSERTX(x)		if(!(x)) { internal((ctx, "%s", #x)); }


NORETURN void fatal(EnvContext *ctx, const char *str, ...);
void error(EnvContext *ctx, const ERROR_NUMBER errorno, const char *str, ...);
void warning(EnvContext *ctx, const WARNING_NUMBER warn, const char *str, ...);
void note(EnvContext *ctx, const char *str, ...);

#endif	/* ERROR_H */
