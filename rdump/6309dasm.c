/* this code was hacked out of the fully-featured 6809 disassembler by Sean Riddle */
/* and then mutliated into a 6309 disassembler by tim lindner					   */
/* Then, in April 2005, I striped it out of MAME and made a stand alone version */

/* 6309dasm.c - a 6309 opcode disassembler		*/
/* Version 1.0 5-AUG-2000						*/
/* Copyright © 2000 tim lindner 				*/
/*												*/
/* based on:									*/
/*		6809dasm.c - a 6809 opcode disassembler */
/*		Version 1.4 1-MAR-95					*/
/*		Copyright © 1995 Sean Riddle			*/
/*												*/
/*		thanks to Franklin Bowen for bug fixes, ideas */

/* Freely distributable on any medium given all copyrights are retained */
/* by the author and no charge greater than $7.00 is made for obtaining */
/* this software */

/* Please send all bug reports, update ideas and data files to: */
/* tlindner@ix.netcom.com */

#include <stdio.h>
#include <string.h>

#include "hd6309.h"
#include "rof.h"

#ifndef TRUE
#define TRUE	-1
#define FALSE	0
#endif

#define UINT8 unsigned char
#define INT8 signed char
#define INT16 signed short

#define DASMFLAG_STEP_OVER 0
#define	AMASK 0xffff;
#define ABITS 16
#define dbg_dasm_relative_jumps 0

/* What EA address to set with debug_ea_info (origin) */
enum {
    EA_DST,
    EA_SRC
};

/* Size of the data element accessed (or the immediate value) */
enum {
    EA_DEFAULT,
    EA_INT8,
    EA_UINT8,
    EA_INT16,
    EA_UINT16,
    EA_INT32,
    EA_UINT32,
    EA_SIZE
};

/* Access modes for effective addresses to debug_ea_info */
enum {
    EA_NONE,        /* no EA mode */
    EA_VALUE,       /* immediate value */
    EA_ABS_PC,      /* change PC absolute (JMP or CALL type opcodes) */
    EA_REL_PC,      /* change PC relative (BRA or JR type opcodes) */
	EA_ZPG_RD,		/* read zero page memory */
	EA_ZPG_WR,		/* write zero page memory */
	EA_ZPG_RDWR,	/* read then write zero page memory */
    EA_MEM_RD,      /* read memory */
    EA_MEM_WR,      /* write memory */
    EA_MEM_RDWR,    /* read then write memory */
    EA_PORT_RD,     /* read i/o port */
    EA_PORT_WR,     /* write i/o port */
    EA_COUNT
};

typedef struct {				/* opcode structure */
   UINT8	opcode; 			/* 8-bit opcode value */
   UINT8	numoperands;
   char 	name[6];			/* opcode name */
   UINT8	mode;				/* addressing mode */
   UINT8	size;				/* access size */
   UINT8	access; 			/* access mode */
   UINT8	numcycles;			/* number of cycles - not used */
   unsigned flags;				/* disassembly flags */
} opcodeinfo;

/* page 1 ops */
static opcodeinfo pg1opcodes[] =
{
	{ 0x00,1,"neg",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x01,2,"oim",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x02,2,"aim",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x03,1,"com",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x04,1,"lsr",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x05,2,"eim",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x06,1,"ror",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x07,1,"asr",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x08,1,"asl",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x09,1,"rol",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x0a,1,"dec",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x0b,2,"tim",     DIR, EA_UINT8,  EA_ZPG_RD,    6},  /* Unsure about this cycle count (TL)*/
	{ 0x0c,1,"inc",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x0d,1,"tst",     DIR, EA_UINT8,  EA_ZPG_RDWR,  6},
	{ 0x0e,1,"jmp",     DIR, EA_UINT8,  EA_ABS_PC,    3},
	{ 0x0f,1,"clr",     DIR, EA_UINT8,  EA_ZPG_WR,    6},

	{ 0x10,1,"page2",   PG2, 0,        0,            0},
	{ 0x11,1,"page3",   PG3, 0,        0,            0},
	{ 0x12,0,"nop",     INH, 0,        0,            2},
	{ 0x13,0,"sync",    INH, 0,        0,            4},
	{ 0x14,0,"sexw",    INH, 0,        0,            4},
	{ 0x16,2,"lbra",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x17,2,"lbsr",    LREL,EA_INT16, EA_REL_PC,    9, DASMFLAG_STEP_OVER},
	{ 0x19,0,"daa",     INH, 0,        0,            2},
	{ 0x1a,1,"orcc",    IMM, 0,        0,            3},
	{ 0x1c,1,"andcc",   IMM, 0,        0,            3},
	{ 0x1d,0,"sex",     INH, 0,        0,            2},
	{ 0x1e,1,"exg",     IMM, 0,        0,            8},
	{ 0x1f,1,"tfr",     IMM, 0,        0,            6},

	{ 0x20,1,"bra",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x21,1,"brn",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x22,1,"bhi",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x23,1,"bls",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x24,1,"bcc",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x25,1,"bcs",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x26,1,"bne",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x27,1,"beq",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x28,1,"bvc",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x29,1,"bvs",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x2a,1,"bpl",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x2b,1,"bmi",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x2c,1,"bge",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x2d,1,"blt",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x2e,1,"bgt",     REL, EA_INT8,  EA_REL_PC,    3},
	{ 0x2f,1,"ble",     REL, EA_INT8,  EA_REL_PC,    3},

	{ 0x30,1,"leax",    IND, EA_UINT16,EA_VALUE,     2},
	{ 0x31,1,"leay",    IND, EA_UINT16,EA_VALUE,     2},
	{ 0x32,1,"leas",    IND, EA_UINT16,EA_VALUE,     2},
	{ 0x33,1,"leau",    IND, EA_UINT16,EA_VALUE,     2},
	{ 0x34,1,"pshs",    INH, 0,        0,            5},
	{ 0x35,1,"puls",    INH, 0,        0,            5},
	{ 0x36,1,"pshu",    INH, 0,        0,            5},
	{ 0x37,1,"pulu",    INH, 0,        0,            5},
	{ 0x39,0,"rts",     INH, 0,        0,            5},
	{ 0x3A,0,"abx",     INH, 0,        0,            3},
	{ 0x3B,0,"rti",     INH, 0,        0,            6},
	{ 0x3C,1,"cwai",    IMM, 0,        0,           20},
	{ 0x3D,0,"mul",     INH, 0,        0,           11},
	{ 0x3F,0,"swi",     INH, 0,        0,           19},

	{ 0x40,0,"nega",    INH, 0,        0,            2},
	{ 0x43,0,"coma",    INH, 0,        0,            2},
	{ 0x44,0,"lsra",    INH, 0,        0,            2},
	{ 0x46,0,"rora",    INH, 0,        0,            2},
	{ 0x47,0,"asra",    INH, 0,        0,            2},
	{ 0x48,0,"asla",    INH, 0,        0,            2},
	{ 0x49,0,"rola",    INH, 0,        0,            2},
	{ 0x4A,0,"deca",    INH, 0,        0,            2},
	{ 0x4C,0,"inca",    INH, 0,        0,            2},
	{ 0x4D,0,"tsta",    INH, 0,        0,            2},
	{ 0x4F,0,"clra",    INH, 0,        0,            2},

	{ 0x50,0,"negb",    INH, 0,        0,            2},
	{ 0x53,0,"comb",    INH, 0,        0,            2},
	{ 0x54,0,"lsrb",    INH, 0,        0,            2},
	{ 0x56,0,"rorb",    INH, 0,        0,            2},
	{ 0x57,0,"asrb",    INH, 0,        0,            2},
	{ 0x58,0,"aslb",    INH, 0,        0,            2},
	{ 0x59,0,"rolb",    INH, 0,        0,            2},
	{ 0x5A,0,"decb",    INH, 0,        0,            2},
	{ 0x5C,0,"incb",    INH, 0,        0,            2},
	{ 0x5D,0,"tstb",    INH, 0,        0,            2},
	{ 0x5F,0,"clrb",    INH, 0,        0,            2},

	{ 0x60,1,"neg",     IND, EA_UINT8,  EA_MEM_RDWR,  6},
	{ 0x61,2,"oim",     IND, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x62,2,"aim",     IND, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x63,1,"com",     IND, EA_UINT8,  EA_MEM_RDWR,  6},
	{ 0x64,1,"lsr",     IND, EA_UINT8,  EA_MEM_RDWR,  6},
	{ 0x65,2,"eim",     IND, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x66,1,"ror",     IND, EA_UINT8,  EA_MEM_RDWR,  6},
	{ 0x67,1,"asr",     IND, EA_UINT8,  EA_MEM_RDWR,  6},
	{ 0x68,1,"asl",     IND, EA_UINT8,  EA_MEM_RDWR,  6},
	{ 0x69,1,"rol",     IND, EA_UINT8,  EA_MEM_RDWR,  6},
	{ 0x6A,1,"dec",     IND, EA_UINT8,  EA_MEM_RDWR,  6},
	{ 0x6B,2,"tim",     IND, EA_UINT8,  EA_MEM_RD,    7},
	{ 0x6C,1,"inc",     IND, EA_UINT8,  EA_MEM_RDWR,  6},
	{ 0x6D,1,"tst",     IND, EA_UINT8,  EA_MEM_RD,    6},
	{ 0x6E,1,"jmp",     IND, EA_UINT8,  EA_ABS_PC,    3},
	{ 0x6F,1,"clr",     IND, EA_UINT8,  EA_MEM_WR,    6},

	{ 0x70,2,"neg",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x71,3,"oim",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x72,3,"aim",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x73,2,"com",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x74,2,"lsr",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x75,3,"eim",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x76,2,"ror",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x77,2,"asr",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x78,2,"asl",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x79,2,"rol",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x7A,2,"dec",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x7B,3,"tim",     EXT, EA_UINT8,  EA_MEM_RD,    7},
	{ 0x7C,2,"inc",     EXT, EA_UINT8,  EA_MEM_RDWR,  7},
	{ 0x7D,2,"tst",     EXT, EA_UINT8,  EA_MEM_RD,    7},
	{ 0x7E,2,"jmp",     EXT, EA_UINT8,  EA_ABS_PC,    4},
	{ 0x7F,2,"clr",     EXT, EA_UINT8,  EA_MEM_WR,    7},

	{ 0x80,1,"suba",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x81,1,"cmpa",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x82,1,"sbca",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x83,2,"subd",    IMM, EA_UINT16,EA_VALUE,     4},
	{ 0x84,1,"anda",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x85,1,"bita",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x86,1,"lda",     IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x88,1,"eora",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x89,1,"adca",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x8A,1,"ora",     IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x8B,1,"adda",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0x8C,2,"cmpx",    IMM, EA_UINT16,EA_VALUE,     4},
	{ 0x8D,1,"bsr",     REL, EA_INT8,  EA_REL_PC,    7, DASMFLAG_STEP_OVER},
	{ 0x8E,2,"ldx",     IMM, EA_UINT16,EA_VALUE,     3},

	{ 0x90,1,"suba",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x91,1,"cmpa",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x92,1,"sbca",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x93,1,"subd",    DIR, EA_UINT16,EA_ZPG_RD,    6},
	{ 0x94,1,"anda",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x95,1,"bita",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x96,1,"lda",     DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x97,1,"sta",     DIR, EA_UINT8, EA_ZPG_WR,    4},
	{ 0x98,1,"eora",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x99,1,"adca",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x9A,1,"ora",     DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x9B,1,"adda",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0x9C,1,"cmpx",    DIR, EA_UINT16,EA_ZPG_RD,    6},
	{ 0x9D,1,"jsr",     DIR, EA_UINT8, EA_ABS_PC,    7, DASMFLAG_STEP_OVER},
	{ 0x9E,1,"ldx",     DIR, EA_UINT16,EA_ZPG_RD,    5},
	{ 0x9F,1,"stx",     DIR, EA_UINT16,EA_ZPG_WR,    5},

	{ 0xA0,1,"suba",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xA1,1,"cmpa",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xA2,1,"sbca",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xA3,1,"subd",    IND, EA_UINT16,EA_MEM_RD,    6},
	{ 0xA4,1,"anda",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xA5,1,"bita",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xA6,1,"lda",     IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xA7,1,"sta",     IND, EA_UINT8, EA_MEM_WR,    4},
	{ 0xA8,1,"eora",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xA9,1,"adca",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xAA,1,"ora",     IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xAB,1,"adda",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xAC,1,"cmpx",    IND, EA_UINT16,EA_MEM_RD,    6},
	{ 0xAD,1,"jsr",     IND, EA_UINT8, EA_ABS_PC,    7, DASMFLAG_STEP_OVER},
	{ 0xAE,1,"ldx",     IND, EA_UINT16,EA_MEM_RD,    5},
	{ 0xAF,1,"stx",     IND, EA_UINT16,EA_MEM_WR,    5},

	{ 0xB0,2,"suba",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xB1,2,"cmpa",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xB2,2,"sbca",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xB3,2,"subd",    EXT, EA_UINT16,EA_MEM_RD,    7},
	{ 0xB4,2,"anda",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xB5,2,"bita",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xB6,2,"lda",     EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xB7,2,"sta",     EXT, EA_UINT8, EA_MEM_WR,    5},
	{ 0xB8,2,"eora",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xB9,2,"adca",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xBA,2,"ora",     EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xBB,2,"adda",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xBC,2,"cmpx",    EXT, EA_UINT16,EA_MEM_RD,    7},
	{ 0xBD,2,"jsr",     EXT, EA_UINT8, EA_ABS_PC,    8, DASMFLAG_STEP_OVER},
	{ 0xBE,2,"ldx",     EXT, EA_UINT16,EA_MEM_RD,    6},
	{ 0xBF,2,"stx",     EXT, EA_UINT16,EA_MEM_WR,    6},

	{ 0xC0,1,"subb",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xC1,1,"cmpb",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xC2,1,"sbcb",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xC3,2,"addd",    IMM, EA_UINT16,EA_VALUE,     4},
	{ 0xC4,1,"andb",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xC5,1,"bitb",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xC6,1,"ldb",     IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xC8,1,"eorb",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xC9,1,"adcb",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xCA,1,"orb",     IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xCB,1,"addb",    IMM, EA_UINT8, EA_VALUE,     2},
	{ 0xCC,2,"ldd",     IMM, EA_UINT16,EA_VALUE,     3},
	{ 0xCD,4,"ldq",     IMM, EA_UINT32,EA_VALUE,     5},
	{ 0xCE,2,"ldu",     IMM, EA_UINT16,EA_VALUE,     3},

	{ 0xD0,1,"subb",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xD1,1,"cmpb",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xD2,1,"sbcb",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xD3,1,"addd",    DIR, EA_UINT8, EA_ZPG_RD,    6},
	{ 0xD4,1,"andb",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xD5,1,"bitb",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xD6,1,"ldb",     DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xD7,1,"stb",     DIR, EA_UINT8, EA_ZPG_WR,    4},
	{ 0xD8,1,"eorb",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xD9,1,"adcb",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xDA,1,"orb",     DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xDB,1,"addb",    DIR, EA_UINT8, EA_ZPG_RD,    4},
	{ 0xDC,1,"ldd",     DIR, EA_UINT16,EA_ZPG_RD,    5},
	{ 0xDD,1,"std",     DIR, EA_UINT16,EA_ZPG_WR,    5},
	{ 0xDE,1,"ldu",     DIR, EA_UINT16,EA_ZPG_RD,    5},
	{ 0xDF,1,"stu",     DIR, EA_UINT16,EA_ZPG_WR,    5},

	{ 0xE0,1,"subb",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xE1,1,"cmpb",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xE2,1,"sbcb",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xE3,1,"addd",    IND, EA_UINT8, EA_MEM_RD,    6},
	{ 0xE4,1,"andb",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xE5,1,"bitb",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xE6,1,"ldb",     IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xE7,1,"stb",     IND, EA_UINT8, EA_MEM_WR,    4},
	{ 0xE8,1,"eorb",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xE9,1,"adcb",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xEA,1,"orb",     IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xEB,1,"addb",    IND, EA_UINT8, EA_MEM_RD,    4},
	{ 0xEC,1,"ldd",     IND, EA_UINT16,EA_MEM_RD,    5},
	{ 0xED,1,"std",     IND, EA_UINT16,EA_MEM_WR,    5},
	{ 0xEE,1,"ldu",     IND, EA_UINT16,EA_MEM_RD,    5},
	{ 0xEF,1,"stu",     IND, EA_UINT16,EA_MEM_WR,    5},

	{ 0xF0,2,"subb",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xF1,2,"cmpb",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xF2,2,"sbcb",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xF3,2,"addd",    EXT, EA_UINT8, EA_MEM_RD,    7},
	{ 0xF4,2,"andb",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xF5,2,"bitb",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xF6,2,"ldb",     EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xF7,2,"stb",     EXT, EA_UINT8, EA_MEM_WR,    5},
	{ 0xF8,2,"eorb",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xF9,2,"adcb",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xFA,2,"orb",     EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xFB,2,"addb",    EXT, EA_UINT8, EA_MEM_RD,    5},
	{ 0xFC,2,"ldd",     EXT, EA_UINT16,EA_MEM_RD,    6},
	{ 0xFD,2,"std",     EXT, EA_UINT16,EA_MEM_WR,    6},
	{ 0xFE,2,"ldu",     EXT, EA_UINT16,EA_MEM_RD,    6},
	{ 0xFF,2,"stu",     EXT, EA_UINT16,EA_MEM_WR,    6},
};

/* page 2 ops 10xx*/
static opcodeinfo pg2opcodes[] =
{
	{ 0x21,3,"lbrn",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x22,3,"lbhi",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x23,3,"lbls",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x24,3,"lbcc",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x25,3,"lbcs",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x26,3,"lbne",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x27,3,"lbeq",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x28,3,"lbvc",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x29,3,"lbvs",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x2A,3,"lbpl",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x2B,3,"lbmi",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x2C,3,"lbge",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x2D,3,"lblt",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x2E,3,"lbgt",    LREL,EA_INT16, EA_REL_PC,    5},
	{ 0x2F,3,"lble",    LREL,EA_INT16, EA_REL_PC,    5},

	{ 0x30,2,"addr",     IMM,        0,        0,    4},
	{ 0x31,2,"adcr",     IMM,        0,        0,    4},
	{ 0x32,2,"subr",     IMM,        0,        0,    4},
	{ 0x33,2,"sbcr",     IMM,        0,        0,    4},
	{ 0x34,2,"andr",     IMM,        0,        0,    4},
	{ 0x35,2,"orr",      IMM,        0,        0,    4},
	{ 0x36,2,"eorr",     IMM,        0,        0,    4},
	{ 0x37,2,"cmpr",     IMM,        0,        0,    4},

	{ 0x38,0,"pshsw",    INH,        0,        0,    6},
	{ 0x39,0,"pulsw",    INH,        0,        0,    6},
	{ 0x3A,0,"pshuw",    INH,        0,        0,    6},
	{ 0x3B,0,"puluw",    INH,        0,        0,    6},

	{ 0x3F,2,"swi2",     INH,        0,        0,   20},

	{ 0x40,0,"negd",     INH,        0,        0,    3},
	{ 0x43,0,"comd",     INH,        0,        0,    3},
	{ 0x44,0,"lsrd",     INH,        0,        0,    3},
	{ 0x46,0,"rord",     INH,        0,        0,    3},
	{ 0x47,0,"asrd",     INH,        0,        0,    3},
	{ 0x48,0,"asld",     INH,        0,        0,    3},
	{ 0x49,0,"rold",     INH,        0,        0,    3},

	{ 0x4A,0,"decd",     INH,        0,        0,    3},
	{ 0x4C,0,"incd",     INH,        0,        0,    3},
	{ 0x4D,0,"tstd",     INH,        0,        0,    3},
	{ 0x4f,0,"clrd",     INH,        0,        0,    3},

	{ 0x53,0,"comw",     INH,        0,        0,    3},
	{ 0x54,0,"lsrw",     INH,        0,        0,    3},
	{ 0x56,0,"rorw",     INH,        0,        0,    3},
	{ 0x59,0,"rolw",     INH,        0,        0,    3},
	{ 0x5A,0,"decw",     INH,        0,        0,    3},
	{ 0x5C,0,"incw",     INH,        0,        0,    3},
	{ 0x5D,0,"tstw",     INH,        0,        0,    3},
	{ 0x5F,0,"clrw",     INH,        0,        0,    3},
	{ 0x80,3,"subw",     IMM, EA_UINT16, EA_VALUE,    5},
	{ 0x81,3,"cmpw",     IMM, EA_UINT16, EA_VALUE,    5},
	{ 0x82,3,"sbcd",     IMM, EA_UINT16, EA_VALUE,    5},

	{ 0x83,3,"cmpd",     IMM, EA_UINT16, EA_VALUE,    5},

	{ 0x84,3,"andd",     IMM, EA_UINT16, EA_VALUE,    5},
	{ 0x85,3,"bitd",     IMM, EA_UINT16, EA_VALUE,    5},
	{ 0x86,3,"ldw",      IMM, EA_UINT16, EA_VALUE,    5},
	{ 0x88,3,"eord",     IMM, EA_UINT16, EA_VALUE,    5},
	{ 0x89,3,"adcd",     IMM, EA_UINT16, EA_VALUE,    5},
	{ 0x8A,3,"ord",      IMM, EA_UINT16, EA_VALUE,    5},
	{ 0x8B,3,"addw",     IMM, EA_UINT16, EA_VALUE,    5},

	{ 0x8C,3,"cmpy",     IMM, EA_UINT16, EA_VALUE,    5},
	{ 0x8E,3,"ldy",      IMM, EA_UINT16, EA_VALUE,    4},

	{ 0x90,2,"subw",     DIR, EA_UINT16, EA_ZPG_RD,   7},
	{ 0x91,2,"cmpw",     DIR, EA_UINT16, EA_ZPG_RD,   7},
	{ 0x92,2,"sbcd",     DIR, EA_UINT16, EA_ZPG_RD,   7},

	{ 0x93,2,"cmpd",     DIR, EA_UINT16, EA_ZPG_RD,   7},

	{ 0x94,2,"andd",     DIR, EA_UINT16, EA_ZPG_RD,   7},
	{ 0x95,2,"bitd",     DIR, EA_UINT16, EA_ZPG_RD,   7},
	{ 0x96,2,"ldw",      DIR, EA_UINT16, EA_ZPG_RD,   6},
	{ 0x97,2,"stw",      DIR, EA_UINT16, EA_ZPG_RD,   6},
	{ 0x98,2,"eord",     DIR, EA_UINT16, EA_ZPG_RD,   7},
	{ 0x99,2,"adcd",     DIR, EA_UINT16, EA_ZPG_RD,   7},
	{ 0x9A,2,"ord",      DIR, EA_UINT16, EA_ZPG_RD,   7},
	{ 0x9B,2,"addw",     DIR, EA_UINT16, EA_ZPG_RD,   7},


	{ 0x9C,2,"cmpy",     DIR, EA_UINT16, EA_ZPG_RD,   7},
	{ 0x9E,2,"ldy",      DIR, EA_UINT16, EA_ZPG_RD,   6},
	{ 0x9F,2,"sty",      DIR, EA_UINT16, EA_ZPG_RD,   6},

	{ 0xA0,2,"subw",     IND, EA_UINT16, EA_MEM_RD,   7},
	{ 0xA1,2,"cmpw",     IND, EA_UINT16, EA_MEM_RD,   7},
	{ 0xA2,2,"sbcd",     IND, EA_UINT16, EA_MEM_RD,   7},

	{ 0xA3,2,"cmpd",     IND, EA_UINT16, EA_MEM_RD,   7},

	{ 0xA4,2,"andd",     IND, EA_UINT16, EA_MEM_RD,   7},
	{ 0xA5,2,"bitd",     IND, EA_UINT16, EA_MEM_RD,   7},

	{ 0xA6,2,"ldw",      IND, EA_UINT16, EA_MEM_RD,   6},
	{ 0xA7,2,"stw",      IND, EA_UINT16, EA_MEM_RD,   6},
	{ 0xA8,2,"eord",     IND, EA_UINT16, EA_MEM_RD,   7},
	{ 0xA9,2,"adcd",     IND, EA_UINT16, EA_MEM_RD,   7},
	{ 0xAA,2,"ord",      IND, EA_UINT16, EA_MEM_RD,   7},
	{ 0xAB,2,"addw",     IND, EA_UINT16, EA_MEM_RD,   7},

	{ 0xAC,2,"cmpy",     IND, EA_UINT16, EA_MEM_RD,   7},
	{ 0xAE,2,"ldy",      IND, EA_UINT16, EA_MEM_RD,   6},
	{ 0xAF,2,"sty",      IND, EA_UINT16, EA_MEM_RD,   6},

	{ 0xB0,3,"subw",     EXT, EA_UINT16, EA_MEM_RD,   8},
	{ 0xB1,3,"cmpw",     EXT, EA_UINT16, EA_MEM_RD,   8},
	{ 0xB2,3,"sbcd",     EXT, EA_UINT16, EA_MEM_RD,   8},

	{ 0xB3,3,"cmpd",     EXT, EA_UINT16, EA_MEM_RD,   8},

	{ 0xB4,3,"andd",     EXT, EA_UINT16, EA_MEM_RD,   8},
	{ 0xB5,3,"bitd",     EXT, EA_UINT16, EA_MEM_RD,   8},
	{ 0xB6,3,"ldw",      EXT, EA_UINT16, EA_MEM_RD,   7},
	{ 0xB7,3,"stw",      EXT, EA_UINT16, EA_MEM_RD,   7},
	{ 0xB8,3,"eord",     EXT, EA_UINT16, EA_MEM_RD,   8},
	{ 0xB9,3,"adcd",     EXT, EA_UINT16, EA_MEM_RD,   8},
	{ 0xBA,3,"ord",      EXT, EA_UINT16, EA_MEM_RD,   8},
	{ 0xBB,3,"addw",     EXT, EA_UINT16, EA_MEM_RD,   8},

	{ 0xBC,3,"cmpy",     EXT, EA_UINT16, EA_MEM_RD,   8},
	{ 0xBE,3,"ldy",      EXT, EA_UINT16, EA_MEM_RD,   7},
	{ 0xBF,3,"sty",      EXT, EA_UINT16, EA_MEM_RD,   7},
	{ 0xCE,3,"lds",      IMM, EA_UINT16, EA_VALUE,    4},

	{ 0xDC,2,"ldq",      DIR, EA_UINT16, EA_ZPG_RD,   8},
	{ 0xDD,2,"stq",      DIR, EA_UINT16, EA_ZPG_RD,   8},

	{ 0xDE,2,"lds",      DIR, EA_UINT16, EA_ZPG_RD,   6},
	{ 0xDF,2,"sts",      DIR, EA_UINT16, EA_ZPG_WR,   6},

	{ 0xEC,2,"ldq",      IND, EA_UINT16, EA_MEM_RD,   8},
	{ 0xED,2,"stq",      IND, EA_UINT16, EA_MEM_WR,   8},
	{ 0xEE,2,"lds",      IND, EA_UINT16, EA_MEM_RD,   8},

	{ 0xEE,2,"lds",      IND, EA_UINT16, EA_MEM_RD,   6},
	{ 0xEF,2,"sts",      IND, EA_UINT16, EA_MEM_WR,   6},

	{ 0xFC,3,"ldq",      EXT, EA_UINT16, EA_MEM_RD,   9},
	{ 0xFD,3,"stq",      EXT, EA_UINT16, EA_MEM_WR,   9},

	{ 0xFE,3,"lds",      EXT, EA_UINT16, EA_MEM_RD,   7},
	{ 0xFF,3,"sts",      EXT, EA_UINT16, EA_MEM_WR,   7},
};

/* page 3 ops 11xx */
static opcodeinfo pg3opcodes[] =
{
	{ 0x30,3,"band",    DIR, EA_UINT8, EA_ZPG_RDWR,  7},
	{ 0x31,3,"biand",   DIR, EA_UINT8, EA_ZPG_RDWR,  7},
	{ 0x32,3,"bor",     DIR, EA_UINT8, EA_ZPG_RDWR,  7},
	{ 0x33,3,"bior",    DIR, EA_UINT8, EA_ZPG_RDWR,  7},
	{ 0x34,3,"beor",    DIR, EA_UINT8, EA_ZPG_RDWR,  7},
	{ 0x35,3,"bieor",   DIR, EA_UINT8, EA_ZPG_RDWR,  7},

	{ 0x36,3,"ldbt",    DIR, EA_UINT8, EA_ZPG_RD,    7},
	{ 0x37,3,"stbt",    DIR, EA_UINT8, EA_ZPG_RDWR,  7},

	{ 0x38,2,"tfm",     IMM, 0,        0,            6},
	{ 0x39,2,"tfm",     IMM, 0,        0,            6},
	{ 0x3A,2,"tfm",     IMM, 0,        0,            6},
	{ 0x3B,2,"tfm",     IMM, 0,        0,            6},

	{ 0x3C,2,"bitmd",   IMM, EA_UINT8, EA_VALUE,     4},
	{ 0x3D,2,"ldmd",    IMM, EA_UINT8, EA_VALUE,     5},

	{ 0x3F,1,"swi3",    INH, 0,        0,           20},

	{ 0x43,1,"come",    INH, 0,        0,            3},
	{ 0x4A,1,"dece",    INH, 0,        0,            3},
	{ 0x4C,1,"ince",    INH, 0,        0,            3},
	{ 0x4D,1,"tste",    INH, 0,        0,            3},
	{ 0x4F,1,"clre",    INH, 0,        0,            3},
	{ 0x53,1,"comf",    INH, 0,        0,            3},
	{ 0x5A,1,"decf",    INH, 0,        0,            3},
	{ 0x5C,1,"incf",    INH, 0,        0,            3},
	{ 0x5D,1,"tstf",    INH, 0,        0,            3},
	{ 0x5F,1,"clrf",    INH, 0,        0,            3},

	{ 0x80,2,"sube",    IMM, EA_UINT8, EA_VALUE,     3},
	{ 0x81,2,"cmpe",    IMM, EA_UINT8, EA_VALUE,     3},

	{ 0x83,3,"cmpu",    IMM, EA_UINT16,EA_VALUE,     5},

	{ 0x86,2,"lde",     IMM, EA_UINT8, EA_VALUE,     3},
	{ 0x8b,2,"adde",    IMM, EA_UINT8, EA_VALUE,     3},

	{ 0x8C,3,"cmps",    IMM, EA_UINT16,EA_VALUE,     5},

	{ 0x8D,2,"divd",    IMM, EA_UINT8, EA_VALUE,    25},
	{ 0x8E,3,"divq",    IMM, EA_UINT16,EA_VALUE,    36},
	{ 0x8F,3,"muld",    IMM, EA_UINT16,EA_VALUE,    28},
	{ 0x90,2,"sube",    DIR, EA_UINT8, EA_ZPG_RD,    5},
	{ 0x91,2,"cmpe",    DIR, EA_UINT8, EA_ZPG_RD,    5},

	{ 0x93,2,"cmpu",    DIR, EA_UINT16,EA_ZPG_RD,    7},

	{ 0x96,2,"lde",     DIR, EA_UINT8, EA_ZPG_RD,    5},
	{ 0x97,2,"ste",     DIR, EA_UINT8, EA_ZPG_RD,    5},
	{ 0x9B,2,"adde",    DIR, EA_UINT8, EA_ZPG_RD,    5},

	{ 0x9C,2,"cmps",    DIR, EA_UINT16,EA_ZPG_RD,    7},

	{ 0x9D,2,"divd",    DIR, EA_UINT8 ,EA_ZPG_RD,   27},
	{ 0x9E,2,"divq",    DIR, EA_UINT16,EA_ZPG_RD,   36},
	{ 0x9F,2,"muld",    DIR, EA_UINT16,EA_ZPG_RD,   30},

	{ 0xA0,2,"sube",    IND, EA_UINT8, EA_MEM_RD,    5},
	{ 0xA1,2,"cmpe",    IND, EA_UINT8, EA_MEM_RD,    5},

	{ 0xA3,2,"cmpu",    IND, EA_UINT16,EA_MEM_RD,    7},

	{ 0xA6,2,"lde",     IND, EA_UINT8, EA_MEM_RD,    5},
	{ 0xA7,2,"ste",     IND, EA_UINT8, EA_MEM_WR,    5},
	{ 0xAB,2,"adde",    IND, EA_UINT8, EA_MEM_WR,    5},

	{ 0xAC,2,"cmps",    IND, EA_UINT16,EA_MEM_RD,    7},

	{ 0xAD,2,"divd",    IND, EA_UINT8 ,EA_MEM_RD,   27},
	{ 0xAE,2,"divq",    IND, EA_UINT16,EA_MEM_RD,   36},
	{ 0xAF,2,"muld",    IND, EA_UINT16,EA_MEM_RD,   30},
	{ 0xB0,3,"sube",    EXT, EA_UINT8, EA_MEM_RD,    6},
	{ 0xB1,3,"cmpe",    EXT, EA_UINT8, EA_MEM_RD,    6},

	{ 0xB3,3,"cmpu",    EXT, EA_UINT16,EA_MEM_RD,    8},

	{ 0xB6,3,"lde",     EXT, EA_UINT8, EA_MEM_RD,    6},
	{ 0xB7,3,"ste",     EXT, EA_UINT8, EA_MEM_WR,    6},

	{ 0xBB,3,"adde",    EXT, EA_UINT8, EA_MEM_RD,    6},
	{ 0xBC,3,"cmps",    EXT, EA_UINT16,EA_MEM_RD,    8},

	{ 0xBD,3,"divd",    EXT, EA_UINT8 ,EA_MEM_RD,   28},
	{ 0xBE,3,"divq",    EXT, EA_UINT16,EA_MEM_RD,   37},
	{ 0xBF,3,"muld",    EXT, EA_UINT16,EA_MEM_RD,   31},

	{ 0xC0,2,"subf",    IMM, EA_UINT8, EA_VALUE,     3},
	{ 0xC1,2,"cmpf",    IMM, EA_UINT8, EA_VALUE,     3},
	{ 0xC6,2,"ldf",     IMM, EA_UINT8, EA_VALUE,     3},
	{ 0xCB,2,"addf",    IMM, EA_UINT8, EA_VALUE,     3},

	{ 0xD0,2,"subf",    DIR, EA_UINT8, EA_ZPG_RD,    5},
	{ 0xD1,2,"cmpf",    DIR, EA_UINT8, EA_ZPG_RD,    5},
	{ 0xD6,2,"ldf",     DIR, EA_UINT8, EA_ZPG_RD,    5},
	{ 0xD7,2,"stf",     DIR, EA_UINT8, EA_ZPG_WR,    5},
	{ 0xDB,2,"addf",    DIR, EA_UINT8, EA_ZPG_RD,    5},

	{ 0xE0,2,"subf",    IND, EA_UINT8, EA_MEM_RD,    5},
	{ 0xE1,2,"cmpf",    IND, EA_UINT8, EA_MEM_RD,    5},
	{ 0xE6,2,"ldf",     IND, EA_UINT8, EA_MEM_RD,    5},
	{ 0xE7,2,"stf",     IND, EA_UINT8, EA_MEM_WR,    5},
	{ 0xEB,2,"addf",    IND, EA_UINT8, EA_MEM_RD,    5},

	{ 0xF0,3,"subf",    EXT, EA_UINT8, EA_MEM_RD,    6},
	{ 0xF1,3,"cmpf",    EXT, EA_UINT8, EA_MEM_RD,    6},
	{ 0xF6,3,"ldf",     EXT, EA_UINT8, EA_MEM_RD,    6},
	{ 0xF7,3,"stf",     EXT, EA_UINT8, EA_MEM_WR,    6},
	{ 0xFB,3,"addf",    EXT, EA_UINT8, EA_MEM_RD,    6},

};

int numops6309[3] =
{
	sizeof(pg1opcodes)/sizeof(opcodeinfo),
	sizeof(pg2opcodes)/sizeof(opcodeinfo),
	sizeof(pg3opcodes)/sizeof(opcodeinfo)
};

static opcodeinfo *pgpointers[3] =
{
   pg1opcodes,pg2opcodes,pg3opcodes,
};

static const UINT8 regid_6309[5] = {
	HD6309_X, HD6309_Y, HD6309_U, HD6309_S, HD6309_PC
};

static const char *regs_6309[5] = { "x","y","y","s","pc" };

static const UINT8 btmRegs_id[] = { HD6309_CC, HD6309_A, HD6309_B, 0 };

static const char *btwRegs[5] = { "cc", "a", "b", "inv" };

static const char *teregs[16] = {
	"d","x","y","u","s","pc","w","v",
	"a","b","cc","dp","0","0","e","f"
};

static const char *tfmregs[16] = {
	"d","x","y","u","s","inv","inv","inv",
	"inv","inv","inv","inv","inv","inv","inv","inv"
};

static const char *tfm_s[] = { "%s+,%s+", "%s-,%s-", "%s+,%s", "%s,%s+" };

const char *set_ea_info( int what, unsigned value, int size, int access );
static char *hexstring (int address);
int	activecpu_get_reg( int reg);
int program_read_byte_8( int address );
extern char *get_label( int pc, unsigned char flag_on, unsigned char flag_off );
extern char *get_external_ref( int pc, unsigned char flag_on, unsigned char flag_off, unsigned char *flag );
extern char *remove_colon( char *s );
extern void add_code_label( int address );

unsigned Dasm6309 (char *buffer, int pc, unsigned char *memory, size_t memory_size)
{
	int i, j, k, page=0, opcode, numoperands, mode, size, access;
	UINT8 operandarray[4];
	const char *sym1, *sym2;
	int rel, pb, offset, reg, pb2;
	unsigned ea = 0;
	int p = 0;
	unsigned flags;
	int pc_start = pc;
	
	*buffer = '\0';

	opcode = memory[pc++];
	for( i = 0; i < numops6309[0]; i++ )
		if (pg1opcodes[i].opcode == opcode)
			break;

	if( i < numops6309[0] )
	{
		if( pg1opcodes[i].mode >= PG2 )
		{
			opcode = memory[pc++];
			page = pg1opcodes[i].mode - PG2 + 1;		  /* get page # */
			for( k = 0; k < numops6309[page]; k++ )
				if (opcode == pgpointers[page][k].opcode)
					break;

			if( k != numops6309[page] )
			{	/* opcode found */
				numoperands = pgpointers[page][k].numoperands - 1;
				for (j = 0; j < numoperands; j++)
					operandarray[j] = memory[pc++];
				mode = pgpointers[page][k].mode;
				size = pgpointers[page][k].size;
				access = pgpointers[page][k].access;
				flags = pgpointers[page][k].flags;
				buffer += sprintf (buffer, "%-6s", pgpointers[page][k].name);
			 }
			 else
			 {	/* not found in alternate page */
				buffer += sprintf (buffer, "fcb %d,%d", (page-1) ? 0x10 : 0x11, opcode );
				return 2;
			 }
		}
		else
		{	/* page 1 opcode */
			numoperands = pg1opcodes[i].numoperands;
			for( j = 0; j < numoperands; j++ )
				operandarray[j] = memory[pc++];
			mode = pg1opcodes[i].mode;
			size = pg1opcodes[i].size;
			access = pg1opcodes[i].access;
			flags = pg1opcodes[i].flags;
			buffer += sprintf (buffer, "%-6s", pg1opcodes[i].name);
		}
	}
	else
	{
		buffer += sprintf (buffer, "fcb %d", opcode );
		return 1;
	}

	pc += p;

	if( opcode != 0x1f &&	// reg <-> reg instructions
		opcode != 0x1e &&
		opcode != 0x31 &&
		opcode != 0x30 &&
		opcode != 0x34 &&
		opcode != 0x37 &&
		opcode != 0x36 &&
		opcode != 0x35 &&
		opcode != 0x33 &&
		opcode != 0x32 &&
		opcode != 0x38 &&
		opcode != 0x39 &&
		opcode != 0x3a &&
		opcode != 0x3b )
	{
		if( mode == IMM )
			buffer += sprintf (buffer, "#");
	}

	switch (mode)
	{
	case REL:	  /* 8-bit relative */
		rel = operandarray[0];
		sym1 = set_ea_info(0, pc, (INT8)rel, access);
		buffer += sprintf (buffer, "%s", sym1);
		break;

	case LREL:	  /* 16-bit long relative */
		rel = (operandarray[0] << 8) + operandarray[1];
		sym1 = set_ea_info(0, pc, (INT16)rel, access);
		buffer += sprintf (buffer, "%s", sym1);
		break;

	case IND:	  /* indirect- many flavors */

		if (numoperands == 2 )
		{
			buffer += sprintf (buffer, "#");
			ea = operandarray[0];
			sym1 = set_ea_info(0, ea, EA_UINT8, EA_VALUE );
			buffer += sprintf (buffer, "%s", sym1 );
			buffer += sprintf (buffer, ",");
			pb = operandarray[1];
		}
		else
		{
			pb = operandarray[0];
		}

		if( pb == 0x92 || pb == 0xb2 || pb == 0xbf || pb == 0xd2 || pb == 0xdf || pb == 0xf2 || pb == 0xff )
		{
			buffer += sprintf (buffer, "illegal postbyte (jmp [$FFF0])" );
			break;
		}

		reg = (pb >> 5) & 3;
		pb2 = pb & 0x8f;
		if( pb2 == 0x88 || pb2 == 0x8c )
		{	/* 8-bit offset */

			/* KW 11/05/98 Fix of indirect opcodes		*/
			offset = (INT8)memory[pc++];
			p++;
			if( pb == 0x8c || pb == 0xac || pb == 0xcc || pb == 0xec || pb == 0x9c || pb == 0xbc || pb == 0xdc || pb == 0xfc  ) reg = 4;
			if( (pb & 0x90) == 0x90 ) buffer += sprintf (buffer, "[");
			if( pb == 0x8c || pb == 0xac || pb == 0xcc || pb == 0xec || pb == 0x9c || pb == 0xbc || pb == 0xdc || pb == 0xfc)
			{
				sym1 = set_ea_info(1, pc, (INT8)offset, EA_REL_PC);
				ea = (pc + (INT8)offset + activecpu_get_reg(regid_6309[reg])) & 0xffff;
				if( strcmp( get_external_ref(pc-1, CODENT, 0, NULL ), "" ) == 0 )
					buffer += sprintf (buffer, "%d,%s", offset, regs_6309[reg]);
				else
					buffer += sprintf (buffer, "%s,%s", get_external_ref(pc-1, CODENT, 0, NULL ), regs_6309[reg]);
				
			}
			else
			{
				sym1 = set_ea_info(1, offset, EA_INT8, EA_VALUE);
				ea = (activecpu_get_reg(regid_6309[reg]) + offset) & 0xffff;
				if( strcmp( get_external_ref(pc-1, CODENT, 0, NULL ), "" ) == 0 )
					buffer += sprintf (buffer, "%d,%s", offset, regs_6309[reg]);
				else
					buffer += sprintf (buffer, "%s,%s", get_external_ref(pc-1, CODENT, 0, NULL ), regs_6309[reg]);
			}
		}
		else
		if ( pb == 0x8f || pb == 0x90 )
		{
			if( (pb & 0x90) == 0x90 ) buffer += sprintf (buffer, "[");
			ea = (activecpu_get_reg(HD6309_E) << 8) + activecpu_get_reg(HD6309_F);
			buffer += sprintf (buffer, ",w");
		}
		else
		if( pb2 == 0x89 || pb2 == 0x8d || pb2 == 0x8f || pb == 0xb0 || pb == 0xd0 || pb == 0xf0 )
		{	/* 16-bit */

			/* KW 11/05/98 Fix of indirect opcodes		*/
			
			if( !((pb == 0xcf) || (pb == 0xd0) || (pb == 0xef) || (pb == 0xf0) ) )
			{
				offset = (INT16)( (memory[pc++] << 8) );
				offset += memory[pc++];
			}
			
			if( pb == 0x8d || pb == 0xad || pb == 0xcd || pb == 0xed || pb == 0x9d || pb == 0xbd || pb == 0xdd || pb == 0xfd )
				reg = 4;
			if( (pb&0x90) == 0x90 )
				buffer += sprintf(buffer, "[");
			if( pb == 0x8d || pb == 0xad || pb == 0xcd || pb == 0xed || pb == 0x9d || pb == 0xbd || pb == 0xdd || pb == 0xfd )
			{
				sym1 = set_ea_info(1, pc, (INT16)offset, EA_REL_PC);
				if( strcmp( get_external_ref(pc-2, CODENT, 0, NULL ), "" ) == 0 )
					buffer += sprintf (buffer, "%d,%s", offset, regs_6309[reg]);
				else
					buffer += sprintf (buffer, "%s,%s", get_external_ref(pc-2, CODENT, 0, NULL ), regs_6309[reg]);
			}
			else if ( pb == 0x9f || pb == 0xbf || pb == 0xdf || pb == 0xff ) /* hmm, bf, df, and ff are marked as illegal on the 6309 */
			{
				sym1 = set_ea_info(1, offset, EA_UINT16, EA_VALUE);
                buffer += sprintf (buffer, "%s", sym1);
			}
			else
			if ( pb == 0xaf || pb == 0xb0 )
			{
				offset = (INT16)( (memory[pc] << 8) + memory[pc+1] );
				p += 2;

				sym1 = set_ea_info(1, offset, EA_INT16, EA_MEM_WR);
				ea = ((activecpu_get_reg(HD6309_E) << 8) + (activecpu_get_reg(HD6309_F)) + offset) & 0xffff;
				buffer += sprintf (buffer, "%s,w", sym1);
			}
			else
			if ( pb == 0xcf || pb == 0xd0 )
			{
				ea = (activecpu_get_reg(HD6309_E) << 8) + activecpu_get_reg(HD6309_F);
				buffer += sprintf (buffer, ",w++");
			}
			else
			if ( pb == 0xef || pb == 0xf0 )
			{
				ea = (activecpu_get_reg(HD6309_E) << 8) + activecpu_get_reg(HD6309_F);
				buffer += sprintf (buffer, ",--w");
			}
			else
			{
				sym1 = get_external_ref( pc-2, CODLOC, DIRLOC|LOC1BYT, NULL );
				if( strcmp( sym1, "" ) == 0 )
					sym1 = set_ea_info(1, offset, EA_INT16, EA_VALUE);
				ea = (activecpu_get_reg(regid_6309[reg]) + offset) & 0xffff;
				buffer += sprintf (buffer, "%s,%s", sym1, regs_6309[reg]);
			}
		}
		else
		if( (pb & 0x80) )
		{
			if( (pb & 0x90) == 0x90 )
				buffer += sprintf (buffer, "[");
			
			switch( pb & 0x8f )
			{
			case 0x80:
				ea = activecpu_get_reg(regid_6309[reg]);
				buffer += sprintf (buffer, ",%s+", regs_6309[reg]);
				break;
			case 0x81:
				ea = activecpu_get_reg(regid_6309[reg]);
				buffer += sprintf (buffer, ",%s++", regs_6309[reg]);
				break;
			case 0x82:
				ea = activecpu_get_reg(regid_6309[reg]);
				buffer += sprintf (buffer, ",-%s", regs_6309[reg]);
				break;
			case 0x83:
				ea = activecpu_get_reg(regid_6309[reg]);
				buffer += sprintf (buffer, ",--%s", regs_6309[reg]);
				break;
			case 0x84:
				ea = activecpu_get_reg(regid_6309[reg]);
				buffer += sprintf (buffer, ",%s", regs_6309[reg]);
				break;
			case 0x85:
				ea = (activecpu_get_reg(regid_6309[reg]) + (INT8) activecpu_get_reg(HD6309_B)) & 0xffff;
				buffer += sprintf (buffer, "b,%s", regs_6309[reg]);
				break;
			case 0x86:
				ea = (activecpu_get_reg(regid_6309[reg]) + (INT8) activecpu_get_reg(HD6309_A)) & 0xffff;
				buffer += sprintf (buffer, "a,%s", regs_6309[reg]);
				break;
			case 0x87:
				ea = (activecpu_get_reg(regid_6309[reg]) + (INT8) activecpu_get_reg(HD6309_E)) & 0xffff;
				buffer += sprintf (buffer, "e,%s", regs_6309[reg]);
				break;
			case 0x8a:
				ea = (activecpu_get_reg(regid_6309[reg]) + (INT8) activecpu_get_reg(HD6309_F)) & 0xffff;
				buffer += sprintf (buffer, "f,%s", regs_6309[reg]);
				break;
			case 0x8b:
				ea = (activecpu_get_reg(regid_6309[reg]) + (activecpu_get_reg(HD6309_A) << 8) + activecpu_get_reg(HD6309_B)) & 0xffff;
				buffer += sprintf (buffer, "d,%s", regs_6309[reg]);
				break;
			case 0x8e:
				ea = (activecpu_get_reg(regid_6309[reg]) + (activecpu_get_reg(HD6309_E) << 8) + activecpu_get_reg(HD6309_F)) & 0xffff;
				buffer += sprintf (buffer, "w,%s", regs_6309[reg]);
				break;
			}
		}
		else
		{										   /* 5-bit offset */
			offset = pb & 0x1f;
			if (offset > 15)
				offset = offset - 32;
			buffer += sprintf (buffer, "%d,%s", offset, regs_6309[reg]);
		}
		/* indirect */
			if( (pb & 0x90) == 0x90 )
			{
			ea = ( program_read_byte_8( ea ) << 8 ) + program_read_byte_8( (ea+1) & 0xffff );
			buffer += sprintf (buffer, "]");
			}
		sym2 = set_ea_info(0, ea, size, access);
		break;
	default:
		if( page == 2 && ( opcode == 0x30 || opcode == 0x31 || opcode == 0x32 || opcode == 0x33 || opcode == 0x34 || opcode == 0x35 || opcode == 0x36 || opcode == 0x37 ) )
		{
			/* BAND, BIAND, BOR, BIOR, BEOR, BIEOR, LDBT, STBT */

			/* Decode register */

			pb = operandarray[0];

			buffer += sprintf (buffer, "%s", btwRegs[ ((pb & 0xc0) >> 6) ]);
			buffer += sprintf (buffer, ",");
			buffer += sprintf (buffer, "%d", ((pb & 0x38) >> 3) );
			buffer += sprintf (buffer, ",");
			buffer += sprintf (buffer, "%d", (pb & 0x07) );
			buffer += sprintf (buffer, ",");

			/* print zero page access */

			ea = operandarray[1];
			sym1 = set_ea_info(0, ea, size, access );
			buffer += sprintf (buffer, "%s", sym1 );
		}
		else
		if( page == 2 && ( opcode == 0x38 || opcode == 0x39 || opcode == 0x3a || opcode == 0x3b ) )
		{
			/* TFM instructions */
			buffer += sprintf (buffer, tfm_s[opcode & 0x07], tfmregs[ (operandarray[0] >> 4) & 0xf], tfmregs[operandarray[0] & 0xf]);
		}
		else
		if( opcode == 0x1f || opcode == 0x1e || ( page == 1 && (opcode == 0x31 || opcode == 0x30 || opcode == 0x34 || opcode == 0x37 || opcode == 0x35 || opcode == 0x33 || opcode == 0x1e || opcode == 0x32 ) ) )
		{	/* TFR/EXG + new 2nd page reg<->reg instructions*/
			buffer += sprintf (buffer, "%s,%s", teregs[ (operandarray[0] >> 4) & 0xf], teregs[operandarray[0] & 0xf]);
		}
		else
		if( opcode == 0x34 || opcode == 0x36 )
		{	/* PUSH */
			pb2 = operandarray[0];
			if( pb2 & 0x80 )
			{
				buffer += sprintf (buffer, "pc");
			}
			if( pb2 & 0x40 )
			{
				if( pb2 & 0x80 ) buffer += sprintf (buffer, ",");
				if( opcode == 0x34 || opcode == 0x35 )
				   buffer += sprintf (buffer, "u");
				else
				   buffer += sprintf (buffer, "s");
			}
			if( pb2 & 0x20 )
			{
				if( pb2 & 0xc0 ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "y");
			}
			if( pb2 & 0x10 )
			{
				if( pb2 & 0xe0 ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "x");
			}
			if( pb2 & 0x08 )
			{
				if( pb2 & 0xf0 ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "dp");
			}
			if( pb2 & 0x04 )
			{
				if( pb2 & 0xf8 ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "b");
			}
			if( pb2 & 0x02 )
			{
				if( pb2 & 0xfc ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "a");
			}
			if( pb2 & 0x01 )
			{
				if( pb2 & 0xfe ) buffer += sprintf (buffer, ",");
				strcat (buffer, "cc");
			}
		}
		else
		if( opcode == 0x35 || opcode == 0x37 )
		{	/* PULL */
			pb2 = operandarray[0];
			if( pb2 & 0x01 )
			{
				buffer += sprintf (buffer, "cc");
			}
			if( pb2 & 0x02 )
			{
				if( pb2 & 0x01 ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "a");
			}
			if( pb2 & 0x04 )
			{
				if( pb2 & 0x03 ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "b");
			}
			if( pb2 & 0x08 )
			{
				if( pb2 & 0x07 ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "dp");
			}
			if( pb2 & 0x10 )
			{
				if( pb2 & 0x0f ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "x");
			}
			if( pb2 & 0x20 )
			{
				if( pb2 & 0x1f ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "y");
			}
			if( pb2 & 0x40 )
			{
				if( pb2 & 0x3f ) buffer += sprintf (buffer, ",");
				if( opcode == 0x34 || opcode == 0x35 )
					buffer += sprintf (buffer, "u");
				else
					buffer += sprintf (buffer, "s");
			}
			if( pb2 & 0x80 )
			{
				if( pb2 & 0x7f ) buffer += sprintf (buffer, ",");
				buffer += sprintf (buffer, "pc");
			}
		}
		else
		{
			if ( numoperands == 4)
			{
				if( strcmp( get_external_ref( pc-2, CODLOC, DIRLOC|LOC1BYT, NULL ), "" ) != 0 )
					sym1 = remove_colon( get_external_ref( pc-2, CODLOC, DIRLOC|LOC1BYT, NULL ) );
				else
				{
					ea = (operandarray[0] << 24) + (operandarray[1] << 16) + (operandarray[2] << 8) + operandarray[3];
					sym1 = set_ea_info(0, ea, size, access );
				}
				
				buffer += sprintf (buffer, "%s", sym1 );
			}
			else
			if ( numoperands == 3)
			{
				if( strcmp( get_external_ref( pc-2, CODLOC, DIRLOC|LOC1BYT, NULL ), "" ) != 0 )
					sym1 = remove_colon( get_external_ref( pc-2, CODLOC, DIRLOC|LOC1BYT, NULL ) );
				else
				{
					buffer += sprintf (buffer, "#");
					ea = operandarray[0];
					sym1 = set_ea_info(0, ea, EA_INT8, EA_VALUE );
					buffer += sprintf (buffer, "%s", sym1 );
	
					buffer += sprintf (buffer, ",");
	
					ea = (operandarray[1] << 8) + operandarray[2];
					sym1 = set_ea_info(0, ea, size, access );
				}
				buffer += sprintf (buffer, "%s", sym1 );
			}
			else
			if( numoperands == 2 )
			{
				if( opcode == 0x01 || opcode == 0x02 || opcode == 0x05 || opcode == 0x0B )
				{
					buffer += sprintf (buffer, "#");
					ea = operandarray[0];
					sym1 = set_ea_info(0, ea, EA_UINT8, EA_VALUE );
					buffer += sprintf (buffer, "%s", sym1 );

					buffer += sprintf (buffer, ",");

					ea = operandarray[1];
					sym1 = set_ea_info(0, ea, size, access );
					buffer += sprintf (buffer, "%s", sym1 );
				}
				else
				{
					if( strcmp( get_external_ref( pc-2, CODLOC, DIRLOC|LOC1BYT, NULL ), "" ) != 0 )
						sym1 = remove_colon( get_external_ref( pc-2, CODLOC, DIRLOC|LOC1BYT, NULL ) );
					else
					{
						ea = (operandarray[0] << 8) + operandarray[1];
						sym1 = set_ea_info(0, ea, size, access );
					}

					buffer += sprintf (buffer, "%s", sym1 );
				}
			}
			else
			if( numoperands == 1 )
			{
				switch( access )
				{
					case EA_ZPG_RD:
					case EA_ZPG_WR:
					case EA_ZPG_RDWR:
						if( strcmp( get_external_ref( pc-1, CODLOC|LOC1BYT, DIRLOC, NULL ), "" ) != 0 )
							sym1 = remove_colon( get_external_ref( pc-1, CODLOC|LOC1BYT, DIRLOC, NULL ) );
						else
						{
							ea = operandarray[0];
							sym1 = set_ea_info(0, ea, size, access );
						}
						break;
					default:
						ea = operandarray[0];
						sym1 = set_ea_info(0, ea, size, access );
						break;
				}

				buffer += sprintf (buffer, "%s", sym1 );
			}
		}
		break;
	}

	return pc - pc_start;
}

const char *set_ea_info( int what, unsigned value, int size, int access )
{
	static char buffer[8][63+1];
	static int which = 0;
	const char *sign = "";
	unsigned width, result;

	which = (which+1) % 8;
	
	if( access == EA_REL_PC )
		/* PC relative calls set_ea_info with value = PC and size = offset */
		result = value + size;
	else
		result = value;

	/* set source EA? */
	if( what == EA_SRC )
	{
//		DBGDASM.src_ea_access = access;
//		DBGDASM.src_ea_value = result;
//		DBGDASM.src_ea_size = size;
	}
	else
	if( what == EA_DST )
	{
//		DBGDASM.dst_ea_access = access;
//		DBGDASM.dst_ea_value = result;
//		DBGDASM.dst_ea_size = size;
	}
	else
	{
		return "set_ea_info: invalid <what>!";
	}

	switch( access )
	{
	case EA_VALUE:	/* Immediate value */
		switch( size )
		{
		case EA_INT8:
		case EA_UINT8:
			width = 2;
			break;
		case EA_INT16:
		case EA_UINT16:
			width = 4;
			break;
		case EA_INT32:
		case EA_UINT32:
			width = 8;
			break;
		default:
			return "set_ea_info: invalid <size>!";
		}

		switch( size )
		{
		case EA_INT8:
		case EA_INT16:
		case EA_INT32:
			if( result & (1 << ((width * 4) - 1)) )
			{
				sign = "-";
				result = (unsigned)-result;
			}
			break;
		}

		if (width < 8)
			result &= (1 << (width * 4)) - 1;
		break;

	case EA_ZPG_RD:
	case EA_ZPG_WR:
	case EA_ZPG_RDWR:
		result &= 0xff;
		width = 2;
		break;

	case EA_ABS_PC: /* Absolute program counter change */
		result &= AMASK;
		if( size == EA_INT8 || size == EA_UINT8 )
			width = 2;
		else
		if( size == EA_INT16 || size == EA_UINT16 )
			width = 4;
		else
		if( size == EA_INT32 || size == EA_UINT32 )
			width = 8;
		else
			width = (ABITS + 3) / 4;
		break;

	case EA_REL_PC: /* Relative program counter change */
		if( strcmp( get_external_ref( value-2, CODLOC|RELATIVE, 0, NULL ), "" ) != 0 )
		{
			sprintf( buffer[which], "%s", get_external_ref( value-2, CODLOC|RELATIVE, 0, NULL ) );
			return buffer[which];
		}
			
		if( strcmp( get_external_ref( value-1, CODLOC|RELATIVE|LOC1BYT, 0, NULL ), "" ) != 0 )
		{
			sprintf( buffer[which], "%s", get_external_ref( value-1, CODLOC|RELATIVE|LOC1BYT, 0, NULL ) );
			return buffer[which];
		}
		
		/* Check for global label */
		if( strcmp( get_label( value + size, CODENT, CONENT|SETENT ), "" ) != 0 )
			sprintf( buffer[which], "%s " , remove_colon( get_label( value + size, CODENT, CONENT|SETENT ) ) );
		else
		{
			add_code_label( value + size );
			sprintf( buffer[which], "_%04x", value + size );
		}
			
		return buffer[which];
		
		if( dbg_dasm_relative_jumps )
		{
			
			if( size == 0 )
				return "$";
			if( size < 0 )
			{
				sign = "-";
				result = (unsigned) -size;
			}
			else
			{
				sign = "+";
				result = (unsigned) size;
			}
			sprintf( buffer[which], "$%s%u", sign, result );
			return buffer[which];
		}
		/* fall through */
	default:
		result &= AMASK;
		width = (ABITS + 3) / 4;
	}
	sprintf( buffer[which], "%s$%0*X", sign, width, result );
	return buffer[which];
}

static char *hexstring (int address)
{
	static char labtemp[10];
	sprintf (labtemp, "$%04hX", (unsigned short)address);
	return labtemp;
}

int	activecpu_get_reg( int reg )
{
	return 0;
}

int program_read_byte_8( int address )
{
	return 0;
}

