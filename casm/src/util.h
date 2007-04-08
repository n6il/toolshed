/*****************************************************************************
	util.h	- Declarations for various utility functions

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
#ifndef UTIL_H
#define UTIL_H

#include <assert.h>

#ifdef _DEBUG
#define ASSERT(x)	assert(x)
#else
#define ASSERT(x)
#endif


#ifndef _WIN32
int fnsplit (const char *path, char *drive, char *dir, char *name, char *ext);
void fnmerge (char *path, const char *drive, const char *dir, const char *name, const char *ext);
int filelength(int fno);
char *strlwr(char *str);
#endif


void *AllocMem(const size_t nbytes);
void *CAllocMem(const size_t count, const size_t nbytes);

char *SkipWS(char *ptr);
#define SkipConstWS(x)	((const char*)SkipWS((char*)x))

bool IsWS(const char c);
bool head(const char *str1, const char *str2);

/*----------------------------------------------------------------------------
	lobyte --- return low byte of an int
----------------------------------------------------------------------------*/
#define lobyte(i)	(((u_char)(i)) & 0xff)
#define loword(i)	(((u_int16)(i)) & 0xffff)

/*----------------------------------------------------------------------------
	hibyte --- return high byte of an int
----------------------------------------------------------------------------*/
#define hibyte(i)	((u_char)(((i) >> 8) & 0xff))
#define hiword(i)	((u_int16)(((i) >> 16) & 0xffff))


#define IsBinary(x)			('0' == (x) || '1' == (x))
#define IsOctal(x)			((x) >= '0' && (x) <= '7')
#define IsDigit(x)			(0 != isdigit(x))
#define IsHex(x)			(((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F'))
#define IsHexDecimal(x)		(true == IsDigit(x) || true == IsHex(x))
bool IsCommentChar(const EnvContext *ctx, const char ch);

#endif	/* UTIL_H */
