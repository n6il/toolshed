/* rstruct.h : Structure definitions for rma */

#define SYMLEN 64

/* some helpful defines */
#define PASS1 !Pass2
#define endof(s) &(s[sizeof(s)/sizeof(s[0])])
#define ON 1
#define OFF 0

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#ifndef COCO
#define _ushort unsigned short
#else
#define _ushort unsigned
#define void			/* no "void" in CoCo compiler */
#endif

/* define ltypes */
#define LT_SET 5
#define LT_EQU 6

/* define tofst flags */

#define NO_IMMED 0x80
#define PRE_10  0x20
#define PRE_11  0x40

/* symblstruct->w1 bit field definitions */

#define GLBL 4
#define XTRNL $10

/* LblType bit field  definitions */

#define DPG 2
#define COD 4

typedef struct asm_cod
{
	char           *ccmd,
	                opcode,
	                tofst;
}               CODFORM;

/* typedef struct jmptbl { int (*fn)(); } JMPTBL; */
typedef int     (*JMPTBL) (void);

/* Structure definitions   */
struct optns
{
	char            optn;
	char           *optadr;
};

struct ref_ent
{
	char            ETyp;
	int            *RAddr;
};

struct symblstr
{
	char            smbltyp,/* Symbol type flag             */
	                w1;
	_ushort         s_ofst;	/* Offset into code for sbl def */
	struct symblstr *left,
	               *right;
	struct ref_str *wrd;
	char            symblnam[SYMLEN];	/* Symbol Name */
};

/*
 * Delete this temporarily.. tra to create a structure to hold all of command
 * bytes
 */

/*
 * union storer1 { char C_Byt[4]; #ifdef COCO
 *//* Defined this way because of 68K byte alignment */

/*
 * short C_Int[2]; long q_long; struct { char f_byt; short f_int; } shrtlng;
 * #endif };
 */

struct cmdbytes
{
	unsigned char   prebyte,
	                opcode,
	                opers[5];
};

/* This may be a temporary structure ??? */
struct ref_str
{
	char            RfTyp;
	struct symblstr *r_offset;
	struct ref_str *NxtRef;
};

/* Macro Descriptor */
typedef struct macdescr
{
	struct macdescr *LastMac;	/* Address of previous macro
					 * descriptor   */
	struct symblstr *MSblPtr;	/* Ptr to symbol table   ?????????        */
	FILE           *MWrkFil;/* File descriptor for macro work file    */
	long            MacStart;	/* Address in work file for macro's
					 * begin */
}               MACDSCR;

/* define functions types within r63 */

struct symblstr *WlkTreLft(), *TreSetUp();

/* Now define all data here */

#ifdef MAIN
#define bb
#else
#define bb extern
#endif
