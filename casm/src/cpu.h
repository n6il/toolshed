/*****************************************************************************
	cpu.h	- Declarations for CPU specific information

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
#ifndef CPU_H
#define CPU_H

#include "context.h"

#define BIT_0	0
#define BIT_8	8
#define BIT_16	16
#define BIT_32	32

/*      Opcode Classes          */
typedef enum {
	ADDR_ERROR = ERROR_BASE,
	INDIRECT = 0,		/* Indirect addressing [$xxxx]		*/
	EXTENDED,			/* extended							*/
	DIRECT,				/* Direct Page access				*/
	INDEXED,			/* Indexed only						*/
	INHERENT,			/* Inherent							*/
	IMMEDIATE,			/* Immediate only					*/
	GENERAL,			/* General addressing				*/
	REL_SHORT,			/* Short Relative					*/
	REL_LONG,			/* Long Relative					*/
	NOIMM,				/* General except for Immediate		*/
	RTOR,				/* Register To Register				*/
	REGLIST,			/* Register List					*/
	GRP2,				/* Group 2 (Read/Modify/Write)		*/
	LOGMEM,				/* 6309 logical memory ops			*/
	BTM,				/* Bit transfer and manipulation	*/
	MRTOR,				/* TFM memory transfers				*/
	MRTOR2,				/* TFM memory transfers - CCASM		*/
	PSEUDO				/* Pseudo ops						*/
} ADDR_MODE;




/* MC6809 specific processing */

/*
0000 - D (A:B)    1000 - A
0001 - X          1001 - B
0010 - Y          1010 - CCR
0011 - U          1011 - DPR
0100 - S          1100 - 0
0101 - PC         1101 - 0
0110 - W          1110 - E
0111 - V          1111 - F
*/
/* register names */
typedef enum {
	REG_ERROR = ERROR_BASE,
	REG_ACCD = 0,
	REG_INDX,
	REG_INDY,
	REG_USTACK,
	REG_SSTACK,
	REG_PC,
	REG_ACCW,
	REG_V,
	REG_ACCA,
	REG_ACCB,
	REG_CC,
	REG_DP,
	REG_ZERO,
	REG_ZEROZERO,
	REG_ACCE,
	REG_ACCF,
	REG_PCR,
	REG_LSN_MASK = 0x0f,
	REG_MSN_MASK = 0xf0
} REGISTER;

#define NOPAGE			0x00
#define PAGE2			0x10
#define PAGE3			0x11
#define IPBYTE			0x9F	/* extended indirect postbyte */
#define OP_SWI			0x3F
#define LDQ_OPIMM		0xcd
#define	LDQ_OPOTH		0xdc
#define OP_PSHS			0x34
#define OP_PSHU			0x36
#define OP_PULS			0x35
#define OP_PULU			0x37

#define OP_PSHS_U_1		OP_PSHS
#define OP_PSHS_U_2		0x40
#define OP_TFR_S_U1		0x1f
#define OP_TFR_S_U2		0x43
#define OP_TFR_U_S1		0x1f
#define OP_TFR_U_S2		0x34
#define OP_PULS_U_PC1	OP_PULS
#define OP_PULS_U_PC2	0xc0
#define OP_LEAS_8BIT_1	0x32
#define OP_LEAS_8BIT_2	0xe8



void InitCPU(void);
u_int16 BumpPCReg(const u_int16 val);
u_char GetDPReg();
u_int16 GetPCReg(void);
u_int16 GetOldPCReg(void);
void SetPCReg(const u_int16 val);
void SetOldPCReg(const u_int16 val);
void SetDPReg(const u_char val);
void PushDPReg(EnvContext *ctx);
void PopDPReg(EnvContext *ctx);
bool IsAddressInDirectPage(const u_int16 addr);


#endif	/* CPU_H */
