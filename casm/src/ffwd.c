/*****************************************************************************
	ffwd.c	- forward reference manager

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


#define MAX_FORWARD_REFS	32768

typedef struct {
	u_int16		cfn;
	u_int32		line;
} ForwardRef;

static ForwardRef refs[MAX_FORWARD_REFS];
static int		refsCount = 0;
static int		refsMax = 0;
static u_int16	Ffn = 0;						/* forward ref file #					*/
static u_int32	F_ref = 0;						/* next line with forward ref			*/



bool FwdRefIsRecord(const u_int32 lineNumber, const int fileno)
{
	if(lineNumber == F_ref && fileno == Ffn) {
		return true;
	}
	return false;

}


void FwdRefInit(void)
{
	refsCount = 0;
	refsMax = 0;
}

/*
 *      FwdRefReinit --- reinitialize forward ref file
 */
void FwdRefReinit(EnvContext *ctx)
{
	refsCount = 0;
	if(0 != refsMax) {
		FwdRefNext(ctx);
	}
}

/*
 *      FwdRefMark --- mark current file/line as containing a forward ref
 */
void FwdRefMark(EnvContext *ctx)
{
	ASSERT(refsCount < MAX_FORWARD_REFS);
	refs[refsCount].cfn = Cfn;
	refs[refsCount].line = ctx->m_LineNumber;

	refsCount++;
	refsMax++;
}

/*
 *      FwdRefNext --- get next forward ref
 */
void FwdRefNext(EnvContext *ctx)
{
	ASSERTX(refsCount <= refsMax);
	Ffn = refs[refsCount].cfn;
	F_ref = refs[refsCount].line;
	refsCount++;
}

/*
 *  FwdRefDone --- closes & deletes forward reference file
 */
void FwdRefDone(void)
{
	refsCount = 0;
	refsMax = 0;
}


