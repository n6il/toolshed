/*****************************************************************************
	proto.h	- Function prototypes

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
#ifndef PROTO_H
#define PROTO_H

#include "context.h"
#include "table9.h"
#include "symtab.h"


void Params(EnvContext *ctx, int argc, char **argv);

/* prototypes for functions in as.c */


/*------------------------------------------------------------------------
	eval.c
------------------------------------------------------------------------*/
typedef enum {
	EVAL_NORMAL,
	EVAL_NOERRORS,
	EVAL_INDEXED_FCB
} EVAL_TYPE;

#ifdef __cplusplus
bool Evaluate(EnvContext *line, u_int32 *retResult, EVAL_TYPE evalType, Symbol **retSym);
#endif

bool Evaluate(EnvContext *line, int32 *result, EVAL_TYPE evalType, Symbol **retSym);


/*------------------------------------------------------------------------
	ffwd.c
------------------------------------------------------------------------*/
void FwdRefInit(void);
void FwdRefReinit(EnvContext *ctx);
void FwdRefMark(EnvContext *ctx);
void FwdRefNext(EnvContext *ctx);
void FwdRefDone(void);
bool FwdRefIsRecord(const u_int32 lineNumber, const int fileno);

void ProcOpcode(EnvContext *line, const Mneumonic *mnu);
void ProcGeneral(EnvContext *line, const Mneumonic *op);
void ProcDirect(EnvContext *line, const Mneumonic *op);
void ProcIndexed(EnvContext *line, const Mneumonic *opinfo, u_char modifier);
void ProcInherent(EnvContext *line, const Mneumonic *op);
void ProcImmediate(EnvContext *line, const Mneumonic *op);
void ProcExtended(EnvContext *line, const Mneumonic *op);
void ProcLogicalMem(EnvContext *line, const Mneumonic *op);
void ProcPushPull(EnvContext *line, const Mneumonic *op);
void ProcBitTransfer(EnvContext *line, const Mneumonic *op);
void ProcRegToReg(EnvContext *line, const Mneumonic *op);
void ProcShortBranch(EnvContext *line, const Mneumonic *op);
void ProcLongBranch(EnvContext *line, const Mneumonic *op);
void ProcMemXfer(EnvContext *line, const Mneumonic *op);
void ProcMemXfer2(EnvContext *line, const Mneumonic *op);


void ProcPseudo(EnvContext *line, const Mneumonic *op);


bool ParseLine(LineBuffer *line, EnvContext *operandPtr);
bool ProcessLine(EnvContext *operandPtr);

#endif	/* PROTO_H */
