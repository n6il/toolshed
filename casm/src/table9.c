/*****************************************************************************
	table9.c	- Instruction tables for 6809/6309 code and pseudo ops

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
#include "table9.h"
#include "cpu.h"
#include "pseudo.h"
#include "symtab.h"
#include "proto.h"


static int16	pseudomap[256];
static int16	opcodemap[256];

static const Mneumonic STLIST = {
	ASM_ALL,
	CPU_6809,
	"structure",
	BIT_0,
	PSEUDO,
	NOPAGE,
	STRUCTDEC
};


#define ASM_M80_CASM	(ASM_CASM | ASM_MACRO80)
#define ASM_M80_CCASM	(ASM_CCASM | ASM_MACRO80)
#define ASM_CASM_CCASM	(ASM_CASM | ASM_CCASM)

#define X	-1

int GetCycleCount(EnvContext *ctx, const Mneumonic *op, const ADDR_MODE mode, const CPUTYPE cpu)
{
	const CycleCount *cycle;
	int count;

	count = 0;

	if(CPU_6809 == cpu) {
		cycle = &op->cycles6809;
	} else {
		cycle = &op->cycles6309;
	}

	switch(mode) {
	case IMMEDIATE:
		count = cycle->m_Immediate;
		break;

	case DIRECT:
		count = cycle->m_Direct;
		break;

	case INDEXED:
		count = cycle->m_Indexed;
		break;

	case EXTENDED:
		count = cycle->m_Extended;
		break;

	case INHERENT:
		count = cycle->m_Inherent;
		break;

	default:
		count = X;
	}

	if(X == count) {
		internal((ctx, "unsupported addressing mode passed to GetCycleCount()"));
	}

	return count;
}

const Mneumonic op_table[] = {
	{ASM_ALL,		CPU_6809,	"abx",		BIT_0,	INHERENT,	NOPAGE,	0x3A,	{X, X, X, X, 3}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"adca",		BIT_8,	GENERAL,	NOPAGE,	0x89,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"adcb",		BIT_8,	GENERAL,	NOPAGE,	0xC9,	{2, 4, 4, 5, X}, {2, 3, 4, 3, X}},		/* Check 6309 extended mode */
	{ASM_ALL,		CPU_6309,	"adcd",		BIT_16,	GENERAL,	PAGE2,	0x89,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6309,	"adcr",		BIT_0,	RTOR,		PAGE2,	0x31,	{4, X, X, X, X}, {4, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"adda",		BIT_8,	GENERAL,	NOPAGE,	0x8B,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"addb",		BIT_8,	GENERAL,	NOPAGE,	0xCB,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"addd",		BIT_16,	GENERAL,	NOPAGE,	0xC3,	{4, 6, 6, 7, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"adde",		BIT_8,	GENERAL,	PAGE3,	0x8B,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"addf",		BIT_8,	GENERAL,	PAGE3,	0xCB,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"addr",		BIT_0,	RTOR,		PAGE2,	0x30,	{4, X, X, X, X}, {4, X, X, X, X}},
	{ASM_ALL,		CPU_6309,	"addw",		BIT_16,	GENERAL,	PAGE2,	0x8B,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6309,	"aim",		BIT_0,	LOGMEM,		NOPAGE,	0x02,	{X, 6, 7, 7, X}, {X, 6, 7, 7, X}},
	{ASM_ALL,		CPU_6809,	"anda",		BIT_8,	GENERAL,	NOPAGE,	0x84,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"andb",		BIT_8,	GENERAL,	NOPAGE,	0xC4,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_MACRO80,	CPU_6809,	"andc",		BIT_8,	IMMEDIATE,	NOPAGE,	0x1C,	{3, X, X, X, X}, {3, X, X, X, X}},	/* Macro-80c */
	{ASM_ALL,		CPU_6809,	"andcc",	BIT_8,	IMMEDIATE,	NOPAGE,	0x1C,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6309,	"andd",		BIT_16,	GENERAL,	PAGE2,	0x84,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6309,	"andr",		BIT_0,	RTOR,		PAGE2,	0x34,	{4, X, X, X, X}, {4, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"asl",		BIT_0,	GRP2,		NOPAGE,	0x08,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"asla",		BIT_0,	INHERENT,	NOPAGE,	0x48,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"aslb",		BIT_0,	INHERENT,	NOPAGE,	0x58,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"asld",		BIT_0,	INHERENT,	PAGE2,	0x48,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6809,	"asr",		BIT_0,	GRP2,		NOPAGE,	0x07,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"asra",		BIT_0,	INHERENT,	NOPAGE,	0x47,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"asrb",		BIT_0,	INHERENT,	NOPAGE,	0x57,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"asrd",		BIT_0,	INHERENT,	PAGE2,	0x47,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"band",		BIT_0,	BTM,		PAGE3,	0x30,	{X, 7, X, X, X}, {X, 6, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bcc",		BIT_0,	REL_SHORT,	NOPAGE,	0x24,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bcs",		BIT_0,	REL_SHORT,	NOPAGE,	0x25,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6309,	"beor",		BIT_0,	BTM,		PAGE3,	0x34,	{X, 7, X, X, X}, {X, 6, X, X, X}},
	{ASM_ALL,		CPU_6809,	"beq",		BIT_0,	REL_SHORT,	NOPAGE,	0x27,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bge",		BIT_0,	REL_SHORT,	NOPAGE,	0x2C,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bgt",		BIT_0,	REL_SHORT,	NOPAGE,	0x2E,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bhi",		BIT_0,	REL_SHORT,	NOPAGE,	0x22,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bhs",		BIT_0,	REL_SHORT,	NOPAGE,	0x24,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6309,	"biand",	BIT_0,	BTM,		PAGE3,	0x31,	{X, 7, X, X, X}, {X, 6, X, X, X}},
	{ASM_ALL,		CPU_6309,	"bieor",	BIT_0,	BTM,		PAGE3,	0x35,	{X, 7, X, X, X}, {X, 6, X, X, X}},
	{ASM_ALL,		CPU_6309,	"bior",		BIT_0,	BTM,		PAGE3,	0x33,	{X, 7, X, X, X}, {X, 6, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bita",		BIT_8,	GENERAL,	NOPAGE,	0x85,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"bitb",		BIT_8,	GENERAL,	NOPAGE,	0xC5,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6309,	"bitd",		BIT_16,	GENERAL,	PAGE2,	0x85,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"bitmd",	BIT_8,	IMMEDIATE,	PAGE3,	0x3C,	{4, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"ble",		BIT_0,	REL_SHORT,	NOPAGE,	0x2F,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"blo",		BIT_0,	REL_SHORT,	NOPAGE,	0x25,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bls",		BIT_0,	REL_SHORT,	NOPAGE,	0x23,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"blt",		BIT_0,	REL_SHORT,	NOPAGE,	0x2D,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bmi",		BIT_0,	REL_SHORT,	NOPAGE,	0x2B,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bne",		BIT_0,	REL_SHORT,	NOPAGE,	0x26,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6309,	"bor",		BIT_0,	BTM,		PAGE3,	0x32,	{X, 7, X, X, X}, {X, 6, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bpl",		BIT_0,	REL_SHORT,	NOPAGE,	0x2A,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bra",		BIT_0,	REL_SHORT,	NOPAGE,	0x20,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"brn",		BIT_0,	REL_SHORT,	NOPAGE,	0x21,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bsr",		BIT_0,	REL_SHORT,	NOPAGE,	0x8D,	{7, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bvc",		BIT_0,	REL_SHORT,	NOPAGE,	0x28,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"bvs",		BIT_0,	REL_SHORT,	NOPAGE,	0x29,	{3, X, X, X, X}, {3, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"clr",		BIT_0,	GRP2,		NOPAGE,	0x0F,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"clra",		BIT_0,	INHERENT,	NOPAGE,	0x4F,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"clrb",		BIT_0,	INHERENT,	NOPAGE,	0x5F,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"clrd",		BIT_0,	INHERENT,	PAGE2,	0x4F,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"clre",		BIT_0,	INHERENT,	PAGE3,	0x4F,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"clrf",		BIT_0,	INHERENT,	PAGE3,	0x5F,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"clrw",		BIT_0,	INHERENT,	PAGE2,	0x5F,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6809,	"cmpa",		BIT_8,	GENERAL,	NOPAGE,	0x81,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"cmpb",		BIT_8,	GENERAL,	NOPAGE,	0xC1,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"cmpd",		BIT_16,	GENERAL,	PAGE2,	0x83,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6309,	"cmpe",		BIT_8,	GENERAL,	PAGE3,	0x81,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"cmpf",		BIT_8,	GENERAL,	PAGE3,	0xC1,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"cmpr",		BIT_0,	RTOR,		PAGE2,	0x37,	{4, X, X, X, X}, {4, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"cmps",		BIT_16,	GENERAL,	PAGE3,	0x8C,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"cmpu",		BIT_16,	GENERAL,	PAGE3,	0x83,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6309,	"cmpw",		BIT_16,	GENERAL,	PAGE2,	0x81,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"cmpx",		BIT_16,	GENERAL,	NOPAGE,	0x8C,	{4, 6, 6, 7, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6809,	"cmpy",		BIT_16,	GENERAL,	PAGE2,	0x8C,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"com",		BIT_0,	GRP2,		NOPAGE,	0x03,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"coma",		BIT_0,	INHERENT,	NOPAGE,	0x43,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"comb",		BIT_0,	INHERENT,	NOPAGE,	0x53,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"comd",		BIT_0,	INHERENT,	PAGE2,	0x43,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"come",		BIT_0,	INHERENT,	PAGE3,	0x43,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"comf",		BIT_0,	INHERENT,	PAGE3,	0x53,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"comw",		BIT_0,	INHERENT,	PAGE2,	0x53,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_EDTASMX,	CPU_6309,	"copy",		BIT_0,	MRTOR2,		PAGE3,	0x38,	{6, X, X, X, X}, {6, X, X, X, X}},	/* TFM r+,r+ */
	{ASM_EDTASMX,	CPU_6309,	"copy-",	BIT_0,	MRTOR2,		PAGE3,	0x39,	{6, X, X, X, X}, {6, X, X, X, X}},	/* TFM r-,r- */
	{ASM_ALL,		CPU_6809,	"cwai",		BIT_8,	IMMEDIATE,	NOPAGE,	0x3C,	{22, X, X, X, X}, {20, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"daa",		BIT_0,	INHERENT,	NOPAGE,	0x19,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"dec",		BIT_0,	GRP2,		NOPAGE,	0x0A,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"deca",		BIT_0,	INHERENT,	NOPAGE,	0x4A,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"decb",		BIT_0,	INHERENT,	NOPAGE,	0x5A,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"decd",		BIT_0,	INHERENT,	PAGE2,	0x4A,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"dece",		BIT_0,	INHERENT,	PAGE3,	0x4A,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"decf",		BIT_0,	INHERENT,	PAGE3,	0x5A,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"decw",		BIT_0,	INHERENT,	PAGE2,	0x5A,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"divd",		BIT_8,	GENERAL,	PAGE3,	0x8D,	{25, 27, 27, 28, X}, {25, 26, 27, 27, X}},
	{ASM_ALL,		CPU_6309,	"divq",		BIT_16,	GENERAL,	PAGE3,	0x8E,	{34, 36, 36, 37, X}, {34, 35, 36, 36, X}},
	{ASM_ALL,		CPU_6309,	"eim",		BIT_0,	LOGMEM,		NOPAGE,	0x05,	{X, 6, 7, 7, X}, {X, 6, 7, 7, X}},
	{ASM_ALL,		CPU_6809,	"eora",		BIT_8,	GENERAL,	NOPAGE,	0x88,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"eorb",		BIT_8,	GENERAL,	NOPAGE,	0xC8,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6309,	"eord",		BIT_16,	GENERAL,	PAGE2,	0x88,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6309,	"eorr",		BIT_0,	RTOR,		PAGE2,	0x36,	{4, X, X, X, X}, {4, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"exg",		BIT_0,	RTOR,		NOPAGE,	0x1E,	{8, X, X, X, X}, {5, X, X, X, X}},
	{ASM_EDTASMX,	CPU_6309,	"exp",		BIT_0,	MRTOR2,		PAGE3,	0x3b,	{6, X, X, X, X}, {6, X, X, X, X}},	/* TFM r,r+ */
	{ASM_EDTASMX,	CPU_6309,	"imp",		BIT_0,	MRTOR2,		PAGE3,	0x3a,	{6, X, X, X, X}, {6, X, X, X, X}},	/* TFM r+,r */
	{ASM_ALL,		CPU_6809,	"inc",		BIT_0,	GRP2,		NOPAGE,	0x0C,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"inca",		BIT_0,	INHERENT,	NOPAGE,	0x4C,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"incb",		BIT_0,	INHERENT,	NOPAGE,	0x5C,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"incd",		BIT_0,	INHERENT,	PAGE2,	0x4C,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"ince",		BIT_0,	INHERENT,	PAGE3,	0x4C,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"incf",		BIT_0,	INHERENT,	PAGE3,	0x5C,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"incw",		BIT_0,	INHERENT,	PAGE2,	0x5C,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6809,	"jmp",		BIT_0,	GRP2,		NOPAGE,	0x0E,	{X, 3, 3, 4, X}, {X, 2, 3, 3, X}},
	{ASM_ALL,		CPU_6809,	"jsr",		BIT_0,	NOIMM,		NOPAGE,	0x8D,	{X, 7, 7, 8, X}, {X, 6, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"lbcc",		BIT_0,	REL_LONG,	PAGE2,	0x24,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbcs",		BIT_0,	REL_LONG,	PAGE2,	0x25,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbeq",		BIT_0,	REL_LONG,	PAGE2,	0x27,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbge",		BIT_0,	REL_LONG,	PAGE2,	0x2C,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbgt",		BIT_0,	REL_LONG,	PAGE2,	0x2E,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbhi",		BIT_0,	REL_LONG,	PAGE2,	0x22,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbhs",		BIT_0,	REL_LONG,	PAGE2,	0x24,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lble",		BIT_0,	REL_LONG,	PAGE2,	0x2F,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lblo",		BIT_0,	REL_LONG,	PAGE2,	0x25,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbls",		BIT_0,	REL_LONG,	PAGE2,	0x23,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lblt",		BIT_0,	REL_LONG,	PAGE2,	0x2D,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbmi",		BIT_0,	REL_LONG,	PAGE2,	0x2B,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbne",		BIT_0,	REL_LONG,	PAGE2,	0x26,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbpl",		BIT_0,	REL_LONG,	PAGE2,	0x2A,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbra",		BIT_0,	REL_LONG,	NOPAGE,	0x16,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbrn",		BIT_0,	REL_LONG,	PAGE2,	0x21,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbsr",		BIT_0,	REL_LONG,	NOPAGE,	0x17,	{9, X, X, X, X}, {9, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbvc",		BIT_0,	REL_LONG,	PAGE2,	0x28,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lbvs",		BIT_0,	REL_LONG,	PAGE2,	0x29,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"lda",		BIT_8,	GENERAL,	NOPAGE,	0x86,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"ldb",		BIT_8,	GENERAL,	NOPAGE,	0xC6,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_CASM,		CPU_6309,	"ldbit",	BIT_0,	BTM,		PAGE3,	0x36,	{X, 8, X, X, X}, {X, 7, X, X, X}},
	{ASM_ALL,		CPU_6309,	"ldbt",		BIT_0,	BTM,		PAGE3,	0x36,	{X, 8, X, X, X}, {X, 7, X, X, X}},
	{ASM_ALL,		CPU_6809,	"ldd",		BIT_16,	GENERAL,	NOPAGE,	0xCC,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"lde",		BIT_8,	GENERAL,	PAGE3,	0x86,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"ldf",		BIT_8,	GENERAL,	PAGE3,	0xC6,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"ldmd",		BIT_8,	IMMEDIATE,	PAGE3,	0x3D,	{5, X, X, X, X}, {5, X, X, X, X}},
	{ASM_ALL,		CPU_6309,	"ldq",		BIT_32,	GENERAL,	PAGE2,	0xCD,	{5, 8, 8, 9, X}, {5, 7, 8, 8, X}},
	{ASM_ALL,		CPU_6809,	"lds",		BIT_16,	GENERAL,	PAGE2,	0xCE,	{4, 6, 6, 7, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"ldu",		BIT_16,	GENERAL,	NOPAGE,	0xCE,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"ldw",		BIT_16,	GENERAL,	PAGE2,	0x86,	{4, 6, 6, 7, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"ldx",		BIT_16,	GENERAL,	NOPAGE,	0x8E,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6809,	"ldy",		BIT_16,	GENERAL,	PAGE2,	0x8E,	{4, 6, 6, 7, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"leas",		BIT_0,	INDEXED,	NOPAGE,	0x32,	{X, X, 4, X, X}, {X, X, 4, X, X}},
	{ASM_ALL,		CPU_6809,	"leau",		BIT_0,	INDEXED,	NOPAGE,	0x33,	{X, X, 4, X, X}, {X, X, 4, X, X}},
	{ASM_ALL,		CPU_6809,	"leax",		BIT_0,	INDEXED,	NOPAGE,	0x30,	{X, X, 4, X, X}, {X, X, 4, X, X}},
	{ASM_ALL,		CPU_6809,	"leay",		BIT_0,	INDEXED,	NOPAGE,	0x31,	{X, X, 4, X, X}, {X, X, 4, X, X}},
	{ASM_ALL,		CPU_6809,	"lsl",		BIT_0,	GRP2,		NOPAGE,	0x08,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"lsla",		BIT_0,	INHERENT,	NOPAGE,	0x48,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"lslb",		BIT_0,	INHERENT,	NOPAGE,	0x58,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"lsr",		BIT_0,	GRP2,		NOPAGE,	0x04,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"lsra",		BIT_0,	INHERENT,	NOPAGE,	0x44,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"lsrb",		BIT_0,	INHERENT,	NOPAGE,	0x54,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"lsrd",		BIT_0,	INHERENT,	PAGE2,	0x44,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"lsrw",		BIT_0,	INHERENT,	PAGE2,	0x54,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6809,	"mul",		BIT_0,	INHERENT,	NOPAGE,	0x3D,	{X, X, X, X, 11}, {X, X, X, X, 10}},
	{ASM_ALL,		CPU_6309,	"muld",		BIT_16,	GENERAL,	PAGE3,	0x8F,	{28, 30, 30, 31, X}, {28, 29, 30, 30}},
	{ASM_ALL,		CPU_6809,	"neg",		BIT_0,	GRP2,		NOPAGE,	0x00,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"nega",		BIT_0,	INHERENT,	NOPAGE,	0x40,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"negb",		BIT_0,	INHERENT,	NOPAGE,	0x50,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"negd",		BIT_0,	INHERENT,	PAGE2,	0x40,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6809,	"nop",		BIT_0,	INHERENT,	NOPAGE,	0x12,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"oim",		BIT_0,	LOGMEM,		NOPAGE,	0x01,	{X, 6, 7, 7, X}, {X, 6, 7, 7, X}},
	{ASM_ALL,		CPU_6809,	"ora",		BIT_8,	GENERAL,	NOPAGE,	0x8A,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"orb",		BIT_8,	GENERAL,	NOPAGE,	0xCA,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"orcc",		BIT_8,	IMMEDIATE,	NOPAGE,	0x1A,	{3, X, X, X, X}, {2, X, X, X, X}},
	{ASM_ALL,		CPU_6309,	"ord",		BIT_16,	GENERAL,	PAGE2,	0x8A,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6309,	"orr",		BIT_0,	RTOR,		PAGE2,	0x35,	{4, X, X, X, X}, {4, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"pshs",		BIT_0,	REGLIST,	NOPAGE,	0x34,	{5, 0, 0, 0, 0}, {4, 0, 0, 0, 0}},
	{ASM_ALL,		CPU_6309,	"pshsw",	BIT_0,	INHERENT,	PAGE2,	0x38,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"pshu",		BIT_0,	REGLIST,	NOPAGE,	0x36,	{5, 0, 0, 0, 0}, {4, 0, 0, 0, 0}},
	{ASM_ALL,		CPU_6309,	"pshuw",	BIT_0,	INHERENT,	PAGE2,	0x3A,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"puls",		BIT_0,	REGLIST,	NOPAGE,	0x35,	{5, 0, 0, 0, 0}, {4, 0, 0, 0, 0}},
	{ASM_ALL,		CPU_6309,	"pulsw",	BIT_0,	INHERENT,	PAGE2,	0x39,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"pulu",		BIT_0,	REGLIST,	NOPAGE,	0x37,	{5, 0, 0, 0, 0}, {4, 0, 0, 0, 0}},
	{ASM_ALL,		CPU_6309,	"puluw",	BIT_0,	INHERENT,	PAGE2,	0x3B,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"rol",		BIT_0,	GRP2,		NOPAGE,	0x09,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"rola",		BIT_0,	INHERENT,	NOPAGE,	0x49,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"rolb",		BIT_0,	INHERENT,	NOPAGE,	0x59,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"rold",		BIT_0,	INHERENT,	PAGE2,	0x49,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"rolw",		BIT_0,	INHERENT,	PAGE2,	0x59,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6809,	"ror",		BIT_0,	GRP2,		NOPAGE,	0x06,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"rora",		BIT_0,	INHERENT,	NOPAGE,	0x46,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"rorb",		BIT_0,	INHERENT,	NOPAGE,	0x56,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"rord",		BIT_0,	INHERENT,	PAGE2,	0x46,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"rorw",		BIT_0,	INHERENT,	PAGE2,	0x56,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6809,	"rti",		BIT_0,	INHERENT,	NOPAGE,	0x3B,	{X, X, X, X, 17}, {X, X, X, X, 15}},
	{ASM_CASM,		CPU_6809,	"rtif",		BIT_0,	INHERENT,	NOPAGE,	0x3B,	{X, X, X, X, 6}, {X, X, X, X, 6}},
	{ASM_ALL,		CPU_6809,	"rts",		BIT_0,	INHERENT,	NOPAGE,	0x39,	{X, X, X, X, 5}, {X, X, X, X, 4}},
	{ASM_ALL,		CPU_6809,	"sbca",		BIT_8,	GENERAL,	NOPAGE,	0x82,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"sbcb",		BIT_8,	GENERAL,	NOPAGE,	0xC2,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6309,	"sbcd",		BIT_16,	GENERAL,	PAGE2,	0x82,	{5, 7, 7, 8, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6309,	"sbcr",		BIT_0,	RTOR,		PAGE2,	0x33,	{4, X, X, X, X}, {4, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"sex",		BIT_0,	INHERENT,	NOPAGE,	0x1D,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_CASM,		CPU_6809,	"sexd",		BIT_0,	INHERENT,	NOPAGE,	0x1D,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"sexw",		BIT_0,	INHERENT,	NOPAGE,	0x14,	{X, X, X, X, 4}, {X, X, X, X, 4}},
	{ASM_ALL,		CPU_6809,	"sta",		BIT_0,	NOIMM,		NOPAGE,	0x87,	{X, 4, 4, 5, X}, {X, 3, 4, 3, X}},
	{ASM_ALL,		CPU_6809,	"stb",		BIT_0,	NOIMM,		NOPAGE,	0xC7,	{X, 4, 4, 5, X}, {X, 3, 4, 3, X}},
	{ASM_CASM,		CPU_6309,	"stbit",	BIT_0,	BTM,		PAGE3,	0x37,	{X, 8, X, X, X}, {X, 7, X, X, X}},
	{ASM_ALL,		CPU_6309,	"stbt",		BIT_0,	BTM,		PAGE3,	0x37,	{X, 8, X, X, X}, {X, 7, X, X, X}},
	{ASM_ALL,		CPU_6809,	"std",		BIT_0,	NOIMM,		NOPAGE,	0xCD,	{X, 5, 5, 6, X}, {X, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"ste",		BIT_0,	NOIMM,		PAGE3,	0x87,	{X, 5, 5, 6, X}, {X, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"stf",		BIT_0,	NOIMM,		PAGE3,	0xC7,	{X, 5, 5, 6, X}, {X, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"stq",		BIT_0,	NOIMM,		PAGE2,	0xCD,	{X, 8, 8, 9, X}, {X, 7, 8, 8, X}},
	{ASM_ALL,		CPU_6809,	"sts",		BIT_0,	NOIMM,		PAGE2,	0xCF,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"stu",		BIT_0,	NOIMM,		NOPAGE,	0xCF,	{X, 5, 5, 6, X}, {X, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"stw",		BIT_0,	NOIMM,		PAGE2,	0x87,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"stx",		BIT_0,	NOIMM,		NOPAGE,	0x8F,	{X, 5, 5, 6, X}, {X, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6809,	"sty",		BIT_0,	NOIMM,		PAGE2,	0x8F,	{X, 6, 6, 7, X}, {X, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"suba",		BIT_8,	GENERAL,	NOPAGE,	0x80,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"subb",		BIT_8,	GENERAL,	NOPAGE,	0xC0,	{2, 4, 4, 5, X}, {2, 3, 4, 4, X}},
	{ASM_ALL,		CPU_6809,	"subd",		BIT_16,	GENERAL,	NOPAGE,	0x83,	{4, 6, 6, 7, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"sube",		BIT_8,	GENERAL,	PAGE3,	0x80,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"subf",		BIT_8,	GENERAL,	PAGE3,	0xC0,	{3, 5, 5, 6, X}, {3, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6309,	"subr",		BIT_0,	RTOR,		PAGE2,	0x32,	{4, X, X, X, X}, {4, X, X, X, X}},
	{ASM_ALL,		CPU_6309,	"subw",		BIT_16,	GENERAL,	PAGE2,	0x80,	{5, 6, 6, 7, X}, {4, 5, 6, 6, X}},
	{ASM_ALL,		CPU_6809,	"swi",		BIT_0,	INHERENT,	NOPAGE,	0x3F,	{X, X, X, X, 19}, {X, X, X, X, 21}},
	{ASM_ALL,		CPU_6809,	"swi2",		BIT_0,	INHERENT,	PAGE2,	0x3F,	{X, X, X, X, 20}, {X, X, X, X, 22}},
	{ASM_ALL,		CPU_6809,	"swi3",		BIT_0,	INHERENT,	PAGE3,	0x3F,	{X, X, X, X, 20}, {X, X, X, X, 22}},
	{ASM_ALL,		CPU_6809,	"sync",		BIT_0,	INHERENT,	NOPAGE,	0x13,	{X, X, X, X, 4}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"tfm",		BIT_0,	MRTOR,		PAGE3,	0x38,	{6, X, X, X, X}, {6, X, X, X, X}},
	{ASM_ALL,		CPU_6809,	"tfr",		BIT_0,	RTOR,		NOPAGE,	0x1F,	{6, X, X, X, X}, {4, X, X, X, X}},
	{ASM_CCASM,		CPU_6309,	"tfrp",		BIT_0,	MRTOR2,		PAGE3,	0x38,	{6, X, X, X, X}, {6, X, X, X, X}},	/* copy */
	{ASM_CCASM,		CPU_6309,	"tfrm",		BIT_0,	MRTOR2,		PAGE3,	0x39,	{6, X, X, X, X}, {6, X, X, X, X}},	/* copy- */
	{ASM_CCASM,		CPU_6309,	"tfrs",		BIT_0,	MRTOR2,		PAGE3,	0x3a,	{6, X, X, X, X}, {6, X, X, X, X}},	/* imp */
	{ASM_CCASM,		CPU_6309,	"tfrr",		BIT_0,	MRTOR2,		PAGE3,	0x3b,	{6, X, X, X, X}, {6, X, X, X, X}},	/* exp */
	{ASM_ALL,		CPU_6309,	"tim",		BIT_0,	LOGMEM,		NOPAGE,	0x0B,	{X, 6, 7, 5, X}, {X, 6, 7, 5, X}},
	{ASM_ALL,		CPU_6809,	"tst",		BIT_0,	GRP2,		NOPAGE,	0x0D,	{X, 6, 6, 7, X}, {X, 4, 5, 5, X}},
	{ASM_ALL,		CPU_6809,	"tsta",		BIT_0,	INHERENT,	NOPAGE,	0x4D,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6809,	"tstb",		BIT_0,	INHERENT,	NOPAGE,	0x5D,	{X, X, X, X, 2}, {X, X, X, X, 1}},
	{ASM_ALL,		CPU_6309,	"tstd",		BIT_0,	INHERENT,	PAGE2,	0x4D,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"tste",		BIT_0,	INHERENT,	PAGE3,	0x4D,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"tstf",		BIT_0,	INHERENT,	PAGE3,	0x5D,	{X, X, X, X, 3}, {X, X, X, X, 2}},
	{ASM_ALL,		CPU_6309,	"tstw",		BIT_0,	INHERENT,	PAGE2,	0x5D,	{X, X, X, X, 3}, {X, X, X, X, 2}},

	{ASM_ALL,		CPU_NONE,	NULL,		BIT_0,	ADDR_ERROR,	NOPAGE,	0x00}
};
#undef X

/*
	proc	functionname
	arg		height:word,width:word
	local	aaa:word,bbb:word
	uses	d,x,y

=
	pshs	d,x,y
	tfr		s,u
	leas	-4,s

	ldd		aaa
	addd	bbb
	addd	height
	addd	width
=
	ldd		-2,u
	addd	-4,u
	addd	2,u
	addd	4,u


	ret
=
	tfr		u,s
	puls	d,x,y,pc
	endproc
*/

const Mneumonic pseudo_table[] = {
	{ASM_CCASM,			CPU_6809,	"=",			BIT_0,	PSEUDO, NOPAGE,	EQU		 	},
	{ASM_CASM,			CPU_6809,	".bss",			BIT_0,	PSEUDO, NOPAGE,	SEGBSS    	},
	{ASM_CASM,			CPU_6809,	".code",		BIT_0,	PSEUDO, NOPAGE,	SEGCODE    	},
	{ASM_CASM,			CPU_6809,	".data",		BIT_0,	PSEUDO, NOPAGE,	SEGDATA    	},
	{ASM_CASM,			CPU_6809,	".export",		BIT_0,	PSEUDO, NOPAGE,	EXPORT    	},
	{ASM_CASM,			CPU_6809,	".import",		BIT_0,	PSEUDO, NOPAGE,	IMPORT    	},
	{ASM_CASM,			CPU_6809,	".extern",		BIT_0,	PSEUDO, NOPAGE,	IMPORT    	},
	{ASM_CASM,			CPU_6809,	".extrn",		BIT_0,	PSEUDO, NOPAGE,	IMPORT    	},
	{ASM_CCASM,			CPU_6809,	"align",		BIT_0,	PSEUDO, NOPAGE,	ALIGN    	},
	{ASM_CCASM,			CPU_6809,	"bsz",			BIT_0,	PSEUDO, NOPAGE,	ZMB    		},
	{ASM_MACRO80,		CPU_6809,	"clrd",			BIT_0,	PSEUDO, NOPAGE,	CLRD_6809  	},
	{ASM_CCASM,			CPU_6809,	"cond",			BIT_0,	PSEUDO, NOPAGE,	COND   		},
	{ASM_MACRO80,		CPU_6809,	"dex",			BIT_0,	PSEUDO, NOPAGE,	OP_DEX		},
	{ASM_CCASM,			CPU_6809,	"else",			BIT_0,	PSEUDO, NOPAGE,	ELSE		},
	{ASM_ALL,			CPU_6809,	"end",			BIT_0,	PSEUDO, NOPAGE,	END    		},
	{ASM_M80_CCASM,		CPU_6809,	"endc",			BIT_0,	PSEUDO, NOPAGE,	ENDC		},
	{ASM_ALL,			CPU_6809,	"endif",		BIT_0,	PSEUDO, NOPAGE,	ENDIF		},
	{ASM_M80_CASM,		CPU_6809,	"endm",			BIT_0,	PSEUDO, NOPAGE,	ENDM   		},
	{ASM_CCASM,			CPU_6809,	"endnamespace",	BIT_0,	PSEUDO, NOPAGE,	ENDNS  		},
	{ASM_CASM,			CPU_6809,	"endns",		BIT_0,	PSEUDO, NOPAGE,	ENDNS   	},
	{ASM_CASM,			CPU_6809,	"endp",			BIT_0,	PSEUDO, NOPAGE,	ENDP   		},
	{ASM_CASM,			CPU_6809,	"ends",			BIT_0,	PSEUDO, NOPAGE,	ENDSTRUCT	},
	{ASM_CASM,			CPU_6809,	"endsect",		BIT_0,	PSEUDO, NOPAGE,	ENDSECT		},
	{ASM_CCASM,			CPU_6809,	"endstruct",	BIT_0,	PSEUDO, NOPAGE,	ENDSTRUCT	},
	{ASM_CASM,			CPU_6809,	"endu",			BIT_0,	PSEUDO, NOPAGE,	ENDUNION	},
	{ASM_CCASM,			CPU_6809,	"endunion",		BIT_0,	PSEUDO, NOPAGE,	ENDUNION	},
	{ASM_ALL,			CPU_6809,	"equ",			BIT_0,	PSEUDO, NOPAGE,	EQU    		},
	{ASM_CCASM,			CPU_6809,	"even",			BIT_0,	PSEUDO, NOPAGE,	ALIGN_EVEN	},
	{ASM_CASM,			CPU_6809,	"export",		BIT_0,	PSEUDO, NOPAGE,	EXPORT		},
	{ASM_M80_CASM,		CPU_6809,	"fail",			BIT_0,	PSEUDO, NOPAGE,	FAIL   		},
	{ASM_ALL,			CPU_6809,	"fcb",			BIT_0,	PSEUDO, NOPAGE,	FCB    		},
	{ASM_ALL,			CPU_6809,	"fcc",			BIT_0,	PSEUDO, NOPAGE,	FCC    		},
	{ASM_CCASM,			CPU_6809,	"fcs",			BIT_0,	PSEUDO, NOPAGE,	FCS			},
	{ASM_CCASM,			CPU_6809,	"fcn",			BIT_0,	PSEUDO, NOPAGE,	FCN			},
	{ASM_CCASM,			CPU_6809,	"fcr",			BIT_0,	PSEUDO, NOPAGE,	FCR			},
	{ASM_CCASM,			CPU_6809,	"fcz",			BIT_0,	PSEUDO, NOPAGE,	FCZ			},
	{ASM_ALL,			CPU_6809,	"fdb",			BIT_0,	PSEUDO, NOPAGE,	FDB    		},
	{ASM_CASM,			CPU_6809,	"fill",			BIT_0,	PSEUDO, NOPAGE,	FILL   		},
	{ASM_CCASM,			CPU_6809,	"fqb",			BIT_0,	PSEUDO, NOPAGE,	FQB    		},
	{ASM_CCASM,			CPU_6809,	"fzb",  		BIT_0,	PSEUDO, NOPAGE,	ZMB    		},
	{ASM_CCASM,			CPU_6809,	"fzd",  		BIT_0,	PSEUDO, NOPAGE,	ZMD    		},
	{ASM_CCASM,			CPU_6809,	"fzq",  		BIT_0,	PSEUDO, NOPAGE,	ZMQ    		},
	{ASM_ALL,			CPU_6809,	"if",			BIT_0,	PSEUDO, NOPAGE,	IF			},
	{ASM_CASM,			CPU_6809,	"ifdef",		BIT_0,	PSEUDO, NOPAGE,	IFDEF		},
	{ASM_M80_CASM,		CPU_6809,	"ifeq",			BIT_0,	PSEUDO, NOPAGE,	IFEQ		},
	{ASM_NOEDTASM,		CPU_6809,	"ifge",			BIT_0,	PSEUDO, NOPAGE,	IFGE		},
	{ASM_NOEDTASM,		CPU_6809,	"ifgt",			BIT_0,	PSEUDO, NOPAGE,	IFGT		},
	{ASM_NOEDTASM,		CPU_6809,	"ifle",			BIT_0,	PSEUDO, NOPAGE,	IFLE		},
	{ASM_NOEDTASM,		CPU_6809,	"iflt",			BIT_0,	PSEUDO, NOPAGE,	IFLT		},
	{ASM_ALL,			CPU_6809,	"ifn",			BIT_0,	PSEUDO, NOPAGE,	IFN			},
	{ASM_CASM,			CPU_6809,	"ifndef",		BIT_0,	PSEUDO, NOPAGE,	IFNDEF		},
	{ASM_NOEDTASM,		CPU_6809,	"ifne",			BIT_0,	PSEUDO, NOPAGE,	IFNE		},
	{ASM_NOEDTASM,		CPU_6809,	"ifp1",			BIT_0,	PSEUDO, NOPAGE,	IFP1		},
	{ASM_NOEDTASM,		CPU_6809,	"ifp2",			BIT_0,	PSEUDO, NOPAGE,	IFP2		},
	{ASM_MACRO80,		CPU_6809,	"incl",			BIT_0,	PSEUDO, NOPAGE,	LIB			},
	{ASM_ALL,			CPU_6809,	"include",		BIT_0,	PSEUDO, NOPAGE,	LIB			},
	{ASM_CCASM,			CPU_6809,	"includebin",	BIT_0,	PSEUDO, NOPAGE,	RAW			},
	{ASM_MACRO80,		CPU_6809,	"inx",			BIT_0,	PSEUDO, NOPAGE,	OP_INX		},
	{ASM_CASM,			CPU_6809,	"lib",			BIT_0,	PSEUDO, NOPAGE,	LIB			},
	{ASM_MACRO80,		CPU_6809,	"list",			BIT_0,	PSEUDO, NOPAGE,	LIST		},
	{ASM_MACRO80,		CPU_6809,	"macr",			BIT_0,	PSEUDO, NOPAGE,	MACRO  		},
	{ASM_M80_CASM,		CPU_6809,	"macro",		BIT_0,	PSEUDO, NOPAGE,	MACRO  		},
	{ASM_RMA,			CPU_6809,	"mod",			BIT_0,	PSEUDO, NOPAGE,	MOD			},
	{ASM_ALL,			CPU_6809,	"nam",			BIT_0,	PSEUDO, NOPAGE,	NAME		},
	{ASM_CASM,			CPU_6809,	"name",			BIT_0,	PSEUDO, NOPAGE,	NAME		},
	{ASM_CCASM,			CPU_6809,	"namespace",	BIT_0,	PSEUDO, NOPAGE,	NAMESPACE	},
	{ASM_MACRO80,		CPU_6809,	"nlst",			BIT_0,	PSEUDO, NOPAGE,	NLST		},
	{ASM_CCASM,			CPU_6809,	"odd",			BIT_0,	PSEUDO, NOPAGE,	ALIGN_ODD	},
	{ASM_ALL,			CPU_6809,	"opt",			BIT_0,	PSEUDO, NOPAGE,	OPT    		},
	{ASM_ALL,			CPU_6809,	"org",			BIT_0,	PSEUDO, NOPAGE,	ORG    		},
	{ASM_RMA,			CPU_6809,	"os9",			BIT_0,	PSEUDO, NOPAGE,	OS9			},
	{ASM_ALL,			CPU_6809,	"pag",			BIT_0,	PSEUDO, NOPAGE,	PAGE   		},
	{ASM_CASM_CCASM,	CPU_6809,	"page",			BIT_0,	PSEUDO, NOPAGE,	PAGE   		},
	{ASM_CASM,			CPU_6809,	"popdp",		BIT_0,	PSEUDO, NOPAGE, POPDP		},
	{ASM_CASM,			CPU_6809,	"printdp",		BIT_0,	PSEUDO, NOPAGE, PRINTDP		},
	{ASM_RMA,			CPU_6809,	"psect",		BIT_0,	PSEUDO, NOPAGE, PSECT		},
	{ASM_CASM,			CPU_6809,	"pushdp",		BIT_0,	PSEUDO, NOPAGE, PUSHDP		},
	{ASM_CASM,			CPU_6809,	"raw",			BIT_0,	PSEUDO, NOPAGE,	RAW    		},
	{ASM_MACRO80,		CPU_6809,	"reorg",		BIT_0,	PSEUDO, NOPAGE,	REORG  		},
	{ASM_ALL,			CPU_6809,	"rmb",			BIT_0,	PSEUDO, NOPAGE,	RMB    		},
	{ASM_CCASM,			CPU_6809,	"rmd",			BIT_0,	PSEUDO, NOPAGE,	RMD    		},
	{ASM_CCASM,			CPU_6809,	"rmq",			BIT_0,	PSEUDO, NOPAGE,	RMQ    		},
	{ASM_CCASM,			CPU_6809,	"rzb",  		BIT_0,	PSEUDO, NOPAGE,	ZMB    		},
	{ASM_CCASM,			CPU_6809,	"rzd",  		BIT_0,	PSEUDO, NOPAGE,	ZMD    		},
	{ASM_CCASM,			CPU_6809,	"rzq",  		BIT_0,	PSEUDO, NOPAGE,	ZMQ    		},
	{ASM_ALL,			CPU_6809,	"set",			BIT_0,	PSEUDO, NOPAGE,	EQU			},	/* OS9 set call */
	{ASM_ALL,			CPU_6809,	"setdp",		BIT_0,	PSEUDO, NOPAGE,	SETDP  		},
	{ASM_ALL,			CPU_6809,	"spc",			BIT_0,	PSEUDO, NOPAGE,	NULL_OP		},
	{ASM_CCASM,			CPU_6809,	"struct",		BIT_0,	PSEUDO, NOPAGE,	STRUCT		},
	{ASM_CCASM,			CPU_6809,	"title",  		BIT_0,	PSEUDO, NOPAGE,	TITLE		},
	{ASM_ALL,			CPU_6809,	"ttl",			BIT_0,	PSEUDO, NOPAGE,	TITLE		},
	{ASM_RMA,			CPU_6809,	"use",			BIT_0,	PSEUDO, NOPAGE,	LIB			},
	{ASM_CCASM,			CPU_6809,	"union",		BIT_0,	PSEUDO, NOPAGE,	UNION		},
	{ASM_RMA,			CPU_6809,	"vsect",		BIT_0,	PSEUDO, NOPAGE, VSECT		},
	{ASM_ALL,			CPU_6809,	"zmb",  		BIT_0,	PSEUDO, NOPAGE,	ZMB    		},
	{ASM_CCASM,			CPU_6809,	"zmd",  		BIT_0,	PSEUDO, NOPAGE,	ZMD    		},
	{ASM_CCASM,			CPU_6809,	"zmq",  		BIT_0,	PSEUDO, NOPAGE,	ZMQ    		},
	{ASM_ALL,			CPU_NONE,	NULL,			BIT_0,	PSEUDO,	NOPAGE,	0x00}
};


#define DOOP(p, op)	\
	case op:		\
	ipage = p;		\
	page = #p;		\
	oname = #op;	\
	break;
	



/*

bsz		same as 'zmb'
call	not implemented
else	condition compiling (not implemented)
end		end of assembler source
endc	end of conditional
endif	end of conditional
endm	end of macro
endp	end of procedure (proc) - not implemented
ends	end of structure
equ		equate
fcb		form constant byte
fcc		form constant character string
fcs		form constant string (1)
fcz		form constant string (null terminated)
fdb		form constant word
fill	fill memory (with x number of specified bytes)
if		conditional assemble
ifeq	conditional assemble
ifge	conditional assemble
ifgt	conditional assemble
ifle	conditional assemble
iflt	conditional assemble
ifn		conditional assemble
ifne	conditional assemble
ifp1	conditional assemble
lib		include file
macro	macro declaration
mod		OS-9 module declaration
nam		source code name
name	same as 'nam'
obj		not implemented
opt		turn options on/off
org		Assembler original
os9		os-9 call
pag		set page
page	same as 'pag'
proc	Procedure entry (not implemented)	
raw		include raw text/binary file
rmb		reserve memory bytes
set		same as 'equ'
setdp	Set the DP expected register contents
spc		output spaces in assembler listing
sprite	include special sprite file (depracated)
struc	Structure declaration
struct	same as 'struc'
ttl		Set title for assembler output
use		same as 'lib'
uses	Used for procedures (not implemented)
zmb		Reserve memory bytes (all set to zero)


(1) the last character of the string has it's high bit set
*/


static void buildmap(const Mneumonic *otable, int16 *etable)
{
	int16	 i, loc;

	for(i = 0; i < 256; i++) {
		etable[i] = -1;
	}

	i = 0;
	while(NULL != otable->mnemonic && EOS != *otable->mnemonic) {
		loc = (int16)(*otable->mnemonic);
		if(etable[loc] == -1) {
			etable[loc] = i;
		}
		i++;
		otable++;
	}
}

void InitOpcodeTable()
{
	buildmap(op_table, opcodemap);
	buildmap(pseudo_table, pseudomap);
}

/*----------------------------------------------------------------------------
	mne_look --- mnemonic lookup

	Return pointer to an oper structure if found.
	Searches both the machine mnemonic table and the pseudo table.
----------------------------------------------------------------------------*/
const Mneumonic *mne_look(EnvContext *ctx, const char *str)
{
	const Mneumonic *mid;
	int pchr;
	char chr;

	chr = (char)tolower(*str);

	/* Search machine mnemonics first */
	if((pchr = opcodemap[(u_char)chr]) != -1) {
		mid = &op_table[(u_char)pchr];	/* point to start */
		while(NULL != mid->mnemonic && *mid->mnemonic == chr) {	/* keep going until end of letter */

			/* Check for compatibility */
			if(0 == (mid->compatMask & ctx->m_Compat.m_AsmOpMask)) {
				mid++;
				continue;
			}

			/* If we are assembling for 6809 only, skip 6309 ops */
			if(CPU_6809 == ctx->m_CPUType && CPU_6309 == mid->cputype) {
				mid++;
				continue;
			}

			if(stricmp(mid->mnemonic, str) == 0) {
				return(mid);
			}
			mid++;
		}
	}

	/* Check for pseudo ops */
	if((pchr = pseudomap[(u_char)chr]) != -1) {
		mid = &pseudo_table[(u_char)pchr];	/* point to start */
		while(NULL != mid->mnemonic && *mid->mnemonic == chr) { 	/* keep going until end of list */
			/* Check for compatibility */
			if(0 == (mid->compatMask & ctx->m_Compat.m_AsmOpMask)) {
				mid++;
				continue;
			}

			if(stricmp(str, mid->mnemonic) == 0) {
				return(mid);
			}
			mid++;
		}
	}

	if(StructLookup(str) != NULL) {
		return(&STLIST);
	}
	/* Search structure list */
	return(NULL);
}



/*
label	MACR	{arg}{,arg}{,arg}			Macro
		blah	\0							Macro argument #0
		blah	\1							Macro argument #1
\.AA	SET		value						????? (Macro only variable?)
		ENDM								ctx->m_Finished macro

		FCC		'presents',0,0,1,2,34
		FCB		' +$80
		FCB		0,0,2[$22],2[$2A],$14,0,0	Repeat stream count[val{,val}{,val}]
COMMA	FCB		','+SPC,0					An odd one
		FCC		'rin','g'+SPC,C.AND			another odd one
sym.dot	equ		*							Allows periods in labels
		
A@		BRA		A@							local label
		LIST								start listing
		NLST								Stop listing
		INCL								Include file
		FAIL	"Message"					File during compile
*/
