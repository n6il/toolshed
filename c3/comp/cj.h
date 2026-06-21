#include <stdio.h>

#ifdef MAIN
#define global
#else
#define global extern
#endif


/*  Machine/Operating System dependent definitions.
 *
 *  see also 'local.c'.
 */

/* `direct` requests direct-page storage for hot globals, and on the OS-9 target
   it now reaches c3 as the real DIRECT storage class (the keyword is NOT defined
   away below).  c3 emits those globals into `vsect dp`; the c3lw backend gives
   them a genuine direct page: c3lw.awk collects every `vsect dp` into one
   dedicated `directpage` bss section, os9c3.ld places that section FIRST in the
   data area (offsets 0..255 from the data base Y), and crt0 sets DP = high(Y).
   A `<name` access (offset & 0xFF, page DP) then resolves to Y+offset - the
   variable's real, zeroed home.  The page must stay within 256 bytes (currently
   ~90 bytes of direct globals); the link step checks l_directpage at link time.

   The UNIX host has no such page, so `direct` is defined away there (clang does
   not know the keyword) and every host global stays Y-relative - which is also
   what keeps the host two-pass output byte-identical to single-pass. */
#ifdef OS9
#define		direct
#define		INTTYPE		int
#define		LONGTYPE	long
#define		FLOATYPE	float
#define		DBLETYPE	double
#endif

#ifdef OSK
#define		INTTYPE		short
#define		LONGTYPE	long
#define		FLOATYPE	float
#define		DBLETYPE	double
#endif

#ifdef UNIX
#define		direct
#define		INTTYPE		short
#define		LONGTYPE	long
#define		FLOATYPE	float
#define		DBLETYPE	double
#endif

/* A pointer-width integer (this codebase's intptr_t) for K&R parameters that
   genuinely carry EITHER an int or a pointer depending on a mode flag - gen's
   `arg`, deref/doref's `val`, eprintf's forwarded `s2`/`s3`.  It MUST be exactly
   pointer-width on each target, for two reasons: (1) the K&R stacked-argument
   ABI - the param's width must match what callers push (a 2-byte int/pointer on
   the 6809), so a wider `long` would misalign the following args; (2) on the
   big-endian 6809 a too-wide member is read from its high half, so the value
   would come back shifted (the same trap that bit val.lval - see expnode).
   Hence `int` on the 6809 (int == pointer == 2 bytes) and `long` on the LP64
   host (pointer == 8).  This is the right abstraction, NOT a workaround: a
   single fixed C type cannot be pointer-width on both, and call-arg coercion
   cannot help a parameter that has no single type. */
#ifdef UNIX
typedef long argword;
#else
typedef int argword;
#endif

#define		MICRO				/* suppresses word boundary forcing */
#define		DOFLOATS			/*  include floating point code */
#define		FNAMESIZE	256		/* file name size (host paths can be long) */

#define		LINESIZE	256
#define		MODNAMSIZE  15	  /* module name */

#define		NAMESIZE	8		/* identifiers for assembler */
#define		UNIQUE		'_'		/* for local labels etc. */
#define		NEWLINE		13

typedef struct structstruct {
	struct structstruct *strnext;	/* next element in structure */
	struct symstr *element;			/* ptr to mos entry */
} elem;

typedef struct dimstr {
	short dim;				/* size of this dimension */
	struct dimstr *dptr;	/* ptr to next dimension */
} dimnode;

typedef struct downsym {
	short type;				/* type of entry */
	long size;				/* size of entry (or a struct-tag ptr for USTRUCT) */
	dimnode *dimptr;		/* ptr to list of dimensions */
	int offset;				/* general purpose */
	int storage;			/* storage class */
	union {
		elem *elems;		/* chain of structure members */
		int labflg;			/* label flags */
	} x;
	short blklev;			/* block level of declaration */
	struct symstr *downptr, /* ptr to 'hidden' outer declaration */
				  *snext;	/* ptr to entry in various lists */
} pshnode;

#define DOWNSIZE	sizeof(pshnode)

typedef struct symstr {
	short type;				/* type of entry */
	long size;				/* size of entry (or a struct-tag ptr for USTRUCT) */
	dimnode *dimptr;		/* ptr to list of dimensions */
	int offset;				/* general purpose */
	int storage;			/* storage class */
	union {
		elem *elems;		/* chain of structure members */
		int labflg;			/* label flag */
	} x;
	short blklev;			/* block level of declaration */
	struct symstr *downptr, /* ptr to 'hidden' outer declaration */
				  *snext,	/* ptr to entry in various lists */
				  *hlink;	/* ptr to next symbol in hash chain */
	char sname[NAMESIZE+1]; /* name entry */
	struct symstr *args;	/* FUNCTION ONLY: ANSI-prototype parameter list
				   (snext-chained, types only) for call-argument
				   coercion; NULL = unprototyped (K&R "()" or
				   implicit).  A dedicated field, NOT in the x union:
				   a function's x.elems holds its return-type's
				   struct members, so the two must not alias - and
				   keeping args separate lets a prototype survive the
				   function's later definition.  Placed after sname so
				   it is outside the DOWNSIZE (pushdown) copy region;
				   functions are top-level (blklev 0) and never pushed
				   down, so pshnode need not mirror it. */
} symnode;

#define SYMSIZE	   sizeof(symnode)

#define MAXCARGS	24	/* max call arguments handled by prototype coercion
				   (optim.c coerceargs / cmain2.c IL reader) */

typedef struct expstr {
	short type;				/* type of node */
	long size;				/* size of basic type (long to match symnode.size:
				   c3 shares one member namespace, so a member name must have
				   the same type/offset in every struct that uses it) */
	dimnode *dimptr;		/* ptr to list of dimensions */
	short op;				/* operator */
	union {
		int num;			/* numeric value */
		symnode *sp;		/* symbol pointer (also the pointer-width carry:
					   newnode() stores every value here as (symnode *),
					   at the union's offset 0, so all readers agree on
					   both endiannesses - do NOT re-add a wider `long`
					   member and store into it, that put the value in the
					   wrong half on the big-endian target) */
		long *lp;			/* long constant ptr */
#ifdef DOFLOATS
		FLOATYPE *fp;		/* float constant ptr */
		DBLETYPE *dp;		/* double constant ptr */
#endif
	} val;					/* value or symbol ptr */
	struct expstr *left,	/* left node ptr */
				  *right;	/* right node ptr */
	short lno;				/* line # of token causing this node */
	char *pnt;				/* ptr to line position of token */
	short sux;				/* pseudo Sethi-Ullman number */
	short modifier;			/* added to NAME references */
} expnode;

#define NODESIZE	sizeof(expnode)

typedef struct initstruct {
	struct initstruct *initnext;	/* next initializer */
	expnode *initp;					/* ptr to initializer expression */
	symnode *initname;				/* ptr to dude being initialized */
} initnode;

/* object sizes - char is by definition 1 */
#define			INTSIZE			2
#define			LONGSIZE		4

#ifdef  DOFLOATS
#define			FLOATSIZE		4
#define			DOUBLESIZE		8
#endif

#define			POINTSIZE		2

/*  symbols  */
#define			POINTER			0x10
#define			ARRAY			0x20
#define			FUNCTION		0x30

#define			XTYPE		   0x30
#define			BASICT		   0x0F

#define			INT			   1
#define			CHAR		   2
#define			UNION		   3
#define			STRUCT		   4
#ifdef  DOFLOATS
#define			FLOAT		   5
#define			DOUBLE		   6
#endif
#define			UNSIGN		   7
#define			LONG		   8
#define			LABEL		   9
#define			USTRUCT		   10

#define	   STRTAG  8
#define	   SHORT   10
#define	   ARG	   11
#define	   EXTDEF  12
#define	   AUTO	   13
#define	   EXTERN  14
#define	   STATIC  15
#define	   REG	   16
#define	   RETURN  18
#define	   MOS	   17
#define	   IF	   19
#define	   WHILE   20
#define	   ELSE	   21
#define	   SWITCH  22
#define	   CASE	   23
#define	   BREAK   24
#define	   CONTIN  25
#define	   DO	   26
#define	   DEFAULT 27
#define	   FOR	   28
#define	   GOTO	   29
#define	   TYPEDEF 30
#define	   DEFTYPE 31
#define	   CAST	   32
#define	   DIRECT  33	 /* direct */
#define	   EXTERND 34	 /* extern direct */
#define	   STATICD 35	 /* static direct */

#define	   SEMICOL 40
#define	   LBRACE  41
#define	   RBRACE  42
#define	   LBRACK  43
#define	   RBRACK  44
#define	   LPAREN  45
#define	   RPAREN  46
#define	   COLON   47
#define	   COMMA   48
#define	   KEYWORD 51
#define	   NAME	   52
#define	   CONST   54
#define	   STRING  55
#define	   SIZEOF  59
#define	   INCBEF  60
#define	   DECBEF  61
#define	   INCAFT  62
#define	   DECAFT  63
#define	   NOT	   64
#define	   AMPER   65
#define	   STAR	   66
#define	   NEG	   67
#define	   COMPL   68
#define	   DOT	   69
#define	   ARROW   70
#define	   DBLAND  71
#define	   DBLOR   72
#define	   LCONST  74
#ifdef DOFLOATS
#define	   FCONST  75
#endif
#define	   UMOD	   76
#define	   USHR	   77
#define	   UDIV	   78
#define	   RSUB	   79
#define	   PLUS	   80
#define	   MINUS   81
#define	   TIMES   82
#define	   DIV	   83
#define	   MOD	   84
#define	   SHR	   85
#define	   SHL	   86
#define	   AND	   87
#define	   OR	   88
#define	   XOR	   89

#define	   EQ	   90
#define	   NEQ	   91
#define	   LEQ	   92
#define	   LT	   93
#define	   GEQ	   94
#define	   GT	   95
#define	   ULEQ	   96
#define	   ULT	   97
#define	   UGEQ	   98
#define	   UGT	   99

#define	   ASSIGN  120

#define	   QUERY   100
#define	   CALL	   101
#define	   BSLASH  102
#define	   SHARP   103
#define	   PRIME   104
#define	   QUOTE   105
#define	   LETTER  106
#define	   DIGIT   107
#define	   NEWLN   108
#define	   SPACE   109

/* code generator symbols */
#define	  STACK 110
#define	  UREG  111
#define	  DREG  112
#define	  XREG  113
#define	  EXG	115
#define	  LEA	116
#define	  LOAD  117
#define	  YREG  118
#define	  NODE  119
#define	  STORE 121
#define	  PUSH  122
#define	  LEAX  123
#define	  JMP	124
#define	  JMPEQ  125
#define	  JMPNE  126
#define	  LOADIM 127
#define	  FREG	 128

#define	  COMPARE	   129
#define	  CNDJMP	   130
#define	  ITOL		   131
#define	  LTOI		   132
#define	  CTOI		   133
#define	  UTOL		   134
#define	  DBLOP		   135
#define	  LONGOP	   136
#define	  MOVE		   137
#define	  STOI		   138
#define	  TEST		   139
#ifdef  DOFLOATS
#define	  FTOD		   140
#define	  DTOF		   141
#define	  ITOD		   142
#define	  DTOI		   143
#define	  LTOD		   144
#define	  DTOL		   145
#define	  UTOD		   146
#endif
#define	  XIND		   147
#define	  YIND		   148
#define	  UIND		   149

#define	  HALVE		   150
#define	  UHALVE	   151
#define	  IDOUBLE	   152

#define	  ASSPLUS	160
#define	  ASSMIN	161
#define	  ASSMUL	162
#define	  ASSDIV	163
#define	  ASSMOD	164
#define	  ASSSHR	165
#define	  ASSSHL	166
#define	  ASSAND	167
#define	  ASSOR		168
#define	  ASSXOR	169

/* indirection masks */
#define	  INDIRECT 0x8000
#define	  NOTIND 0x7FFF

/* label tests */
#define		   DEFINED 1
#define		   GONETO  2

/* dp indicators */
#define DDP 1  /* on dp */
#define NDP 0  /* not on dp */

#define	   UNKN	   0
#define	   UNDECL  0


#define			TRUE			1
#define			FALSE			0
#define			ERR				0xFFFF

#define			TAB_CHAR		9
#define			LINEFEED		10
#define			FORMFEED		12
#define			RET_CHAR		13
#define			VTAB_CHAR		11
#define			BACKSPACE		8

#define		   btype(t)		   ((t) & BASICT)
#define		   isptr(t)		   (((t) & XTYPE) == POINTER)
#define		   isary(t)		   (((t) & XTYPE) == ARRAY)
#define		   isftn(t)		   (((t) & XTYPE) == FUNCTION)
#define		   modtype(t,b)	   (((t) & (~BASICT)) + b)

#define		   getlabel(a)	   (++curlabel)

#define		   islong(t)	   (t->type == LONG)
#ifdef DOFLOATS
#define		   isfloat(t)	   (t->type == FLOAT || t->type == DOUBLE)
#endif

global direct FILE  *in,	/* input file pointer */
					*code;  /* code file pointer */

global direct int	lineno,		/* current line number */
					errcount,
					curlabel,	/* next internal label no. */
					sp,			/* current stack pointer offset */
					stlev,		/* stack reservation level */
					callflag,	/* function call flag */
					sflag,		/* suppress stack checking code */
#ifdef PROF
					pflag,		/* if set generate profile calls */
#endif
					nocode,		/* if set no code output */
					ftype,		/* type of current function */
					lastst,		/* last statement type */
					blklev,		/* current block level */
					breaklab,	/* break label */
					cntlab,		/* continue label */
					swflag,		/* current switch level */
					mosflg,		/* member of structure flag */
					sym,		/* current lexical token */
					scount,		/* chars on current line of string */
					symline,	/* lineno of current symbol */
					maxpush;	/* maximum push level */

/* symval holds the current token's value: an int for a CONST, a symnode * for a
   NAME, a pointer to the constant for long/double.  It is therefore the same
   dual int/pointer quantity as argword, and MUST be argword (pointer-width) -
   not long.  As `long` (4 bytes on the 6809) it is pushed 2 bytes too wide when
   passed to newnode()'s value parameter, misaligning newnode's later args
   (lno/pnt) on the stack ABI; argword (int on the 6809) matches what callers
   actually push.  On the LP64 host argword == long, so this is a no-op there. */
global argword	symval;		/* value associated with sym */

global char	filename[FNAMESIZE];	/* current filename buffer */

global direct symnode *freedown,  /* list of free push-down entries */
					*arglist,	/* list of arguments */
					*labelist,  /* list of labels	 */
					*vlist;		/* list of declared variables */

global direct char	*symptr,	/* start of current symbol */
					curch,			/* current character */
					*lobrk,		/* lower store bound */
					*hibrk,		/* upper store bound */
					*lptr;		/* input line pointer */

global direct FILE	*strfile;	/* strings output file */

extern char		chartab[128],	/* character translater table */
				valtab[128];	/* operator precedence table */

global symnode	*hashtab[128],  /* main hash table */
				*mostab[128];	/* structure hash table */

#ifdef PTREE
extern int  kw[200];		/* pointers to symbol names */
#endif

global direct int	dflag,  /* debug flag; if set causes tree prints */
					lflag,  /* if set, source lines copied to code file */
					tflag,  /* got an input file indicator */
					tcount;

global symnode	sdummy;			/* dummy expression node */

global expnode	*freenode;		/* free expression node list */

global char	line[LINESIZE],		/* input line buffer */
			lastline[LINESIZE]; /* saved last line */

extern  char	strname[];		/* string file name */

/*  label state structure  */
typedef struct {
	int		labnum,
			labsp;
	expnode *labdreg,
			*labxreg;
} labstruc;

/*  switch case-list node (shared: built by the parser, consumed by codegen)  */
typedef struct casestct {
	struct casestct *clink;		/* next case in case list */
	int cval;					/* case value */
	short clab;					/* case label */
} casnode;

#define CASESIZE	sizeof(casnode)


/*  function type definitions  */

extern  expnode *parsexp(), *primary(), *explist(), *getcast(),
				*newnode(), *optim(), *fold(), *chtype(), *fixup(),
				*tranexp(), *tranbool(), *treecopy();

extern  dimnode *dimwalk();

/* Declarations for the remaining non-int-returning functions, so their
   results are not truncated to int on a 64-bit host.  See proto.h. */
#include "proto.h"

