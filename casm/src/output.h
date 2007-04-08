/*****************************************************************************
	output.h	- Declarations for output handlers

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
#ifndef OUTPUT_H
#define OUTPUT_H

#include "config.h"
#include "table9.h"
#include "context.h"



typedef enum {
	SEGMENT_INVALID = -1,
	SEGMENT_CODE,
	SEGMENT_DATA,
	SEGMENT_BSS,
} SEGMENT;


typedef void (*FILEGEN_INIT_FUNC)(EnvContext *ctx, FILE *outFile);
typedef void (*FILEGEN_ADDBYTE_FUNC)(EnvContext *ctx, const u_char val,const  u_int16 pcreg);
typedef void (*FILEGEN_FINISH_FUNC)(EnvContext *ctx, const u_int16 regpc);
typedef void (*FILEGEN_FLUSH_FUNC)(EnvContext *ctx, const u_int16 regpc);

#define FILEGEN(type, iface, ext, fill)													\
	static void filegen_##type##_init(EnvContext *ctx, FILE *outFile);					\
	static void filegen_##type##_addbyte(EnvContext *ctx, u_char val, u_int16 pcreg);	\
	static void filegen_##type##_finish(EnvContext *ctx, u_int16 regpc);				\
	static void filegen_##type##_flush(EnvContext *ctx, u_int16 regpc);					\
	const OutputIFace filegen_##iface = {												\
		#ext,																			\
		fill,																			\
		filegen_##type##_init,															\
		filegen_##type##_addbyte,														\
		filegen_##type##_finish,														\
		filegen_##type##_flush,															\
	};


typedef struct {
	const char				*extension;
	bool					fill_uninitdata;
	FILEGEN_INIT_FUNC		filegen_init;
	FILEGEN_ADDBYTE_FUNC	filegen_addbyte;
	FILEGEN_FINISH_FUNC		filegen_finish;
	FILEGEN_FLUSH_FUNC		filegen_flush;
} OutputIFace;

extern const OutputIFace filegen_S19;
extern const OutputIFace filegen_bin;
extern const OutputIFace filegen_os9;
extern const OutputIFace filegen_raw;
extern const OutputIFace filegen_rom;
extern const OutputIFace filegen_mod;
extern const OutputIFace filegen_rof;
extern const OutputIFace filegen_obj;

void SetSegment(EnvContext *info, SEGMENT segment);
void BeginSegment(EnvContext *info, SEGMENT seg);
void EndSegment(EnvContext *info);
SEGMENT GetCurrentSegment(EnvContext *ctx);


void EmitOpCode(EnvContext *ctx, const Mneumonic *op, const u_char modifier);
void EmitOpCodeEx(EnvContext *ctx, const u_char op);
void EmitOpCode2(EnvContext *ctx, const u_char op1, const u_char op2);
void EmitOpPostByte(EnvContext *ctx, const u_char postop);
void EmitOpDataByte(EnvContext *ctx, const u_char data);
void EmitOpDataWord(EnvContext *ctx, const u_int16 data);
void EmitOpDataLong(EnvContext *ctx, const u_int32 data);
void EmitOpAddr(EnvContext *ctx, const u_int16 addr);
void EmitOpRelAddrByte(EnvContext *ctx, const u_char addr);
void EmitOpRelAddrWord(EnvContext *ctx, const u_int16 addr);



void EmitDataByte(EnvContext *ctx, const u_char data);
void EmitDataWord(EnvContext *ctx, const u_int16 data);
void EmitDataLong(EnvContext *ctx, const u_int32 data);
void EmitUninitData(EnvContext *ctx, const u_int16 size);


bool OpenOutput(EnvContext *ctx, const char *name, const bool forceExt);
void FlushOutput(EnvContext *ctx);
void CloseOutput(EnvContext *ctx);
void PrintLine(EnvContext *ctx, const char *lineBuffer, const bool forceComment);
void PrintCycles(EnvContext *ctx, const int cycles);


#endif	/* OUTPUT_H */
