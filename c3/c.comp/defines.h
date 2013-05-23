/* ************************************************************* *
 * defines.h - Contains definitions that may make things clearer *
 * ************************************************************* */

/* $Id: defines.h 73 2008-10-03 20:20:29Z dlb $ */

/* COCO variable sizes */

#define CHARSIZ 1
#define INTSIZ 2
#define UNSGNSIZ 2
#define LONGSIZ 4
#define FLOATSIZ 4
#define DBLSIZ 8

/* Define substitute for '\n'. We may later allow for output compatible
 * with different systems
 */

#define NEWLINE 0x0d

#define ispointer(s) (((s) & 0x30) == 0x10)
#define inbrkets(s) (((s) & 0x30) == 0x20)
#define inparentheses(s) (((s) & 0x30) == 0x30)

/* Label definition structure */

typedef struct lbl_def {
    int gentyp;
    int vsize;
    /* arrmbrs: array member defs */
#ifdef COCO
    struct brktdef *arrmbrs;
#else
    void *arrmbrs; /* +04 */
#endif
    int lbl_nbr;
    int fnccode;
    /*void *w8;*/
    struct memberdef *mbrlist;
    int isfunction;
    struct lbl_def **ftop;      /* +14 */
    struct lbl_def *lblprev;        /* +16 */
    struct lbl_def *fnext;      /* +18 */
    char fnam[LBLLEN];          /* +20 */
} LBLDEF;

/* This is a "cmdref" structure.  This is a structure that is defined for the
 * current cmd, and is distinct from the "lbl_def" structure.  It is a 22(?)
 * byte structure, as opposed to the 28-byte "lbldef" structure */

typedef struct cmd_ref {
    /* The first 3 members are often pretty much copies of the first 3
     * members of the corresponding lbl_def struct
     */
    int ft_Ty;
    int varsize;
    struct brktdef *arrdefs;
    int vartyp;     /* +6 */
    /* cmdval MAY be a pointer to the value of the expression
     * See pass1_07.c around line 375 (after else L4f8b) */
    int cmdval;
    struct cmd_ref *cr_Left;
    struct cmd_ref *cr_Right;
    int _cline;     /* Current line # */            /* +14 */
    char *_lpos;      /* Current position in line */  /* +8 */
    int __cr18;
    int ptrdstval;
} CMDREF;

/* Case reference */
struct case_ref {
    struct case_ref *case_nxt;
    int cas02;
    int cas04;
};

/* Storage for a double.  This structure stores a double in the native
 * format for the host, (native) plus a coco array stored as a set of
 * 4 ints.  This is not a direct representation of a true coco double,
 * due to big-endian storages, and size of int, but in the format we use,
 * we can directly write the 4 int's as "fdb"'s
 */

struct dbltree {
    double native;      /* Host native value          */
    int cocoarr[4];     /* Coco version of the double */
};

/* Struct/union member definition */

struct memberdef
{
    struct memberdef *mbrPrev;
    LBLDEF *mmbrlbl;
};

/* Array member definition */

struct brktdef
{
    int elcount;                /* count of elements for this member */
    struct brktdef *brPrev;    /* ptr to prev member of this array  */
};

/* Another structure of some sort found in p1_01.c
 * - We'll call it g18 for now */
/*struct g18 {
    struct g18 *g18Nxt;
    int g18_2;
    int g18_4;
    int g18_6;
    int g18_8;
    int g18_10;
    int g18_12;
    struct g18 * g18_14;
    struct g18 *g18Prev;
};*/

struct lng_something {
    int smptr;
    long sm_val;
};

/* The following is a structure found in p2_01.c */

/*struct nwref {
    int ftyp;
    int nw02;
    union {
        char st[9];
        int wrd;
    } rfdat;
}; */


/* lbltype->fnccodes */

#define FT_INT 1
#define FT_CHAR 2
#define FT_UNION 3
#define FT_STRUCT 4
#define FT_FLOAT 5
#define FT_DOUBLE 6
#define FT_UNSIGNED 7
#define FT_LONG 8
#define C_GOTO 9
#define FT_SHORT 10
#define FT_PARAM 11
#define FT_NONDPDAT 12
#define FT_AUTO 13
#define FT_EXTERN 14
#define FT_STATIC 15
#define FT_REGISTER 16
#define FT_STRCMBR 17
#define FT_RETURN 18
#define FT_IF 19
#define FT_WHILE 20
#define FT_ELSE 21
#define FT_SWITCH 22
#define FT_CASE 23
#define FT_BREAK 24
#define FT_CONTINUE 25
#define FT_DO 26
#define FT_DEFAULT 27
#define FT_FOR 28
#define FT_GOTO 29
#define FT_TYPEDEF 30
#define FT_DIRECT 33
#define FT_DPXTRN 34
#define FT_DPSTATIC 35
#define FT_SIZEOF 59
/* the following defines _might_ be "C_" types */

/* _chcode values found in D005f */

#define C_SEMICOLON 40
#define C_LBRACE 41
#define C_RBRACE 42
#define C_LBRKET 43
#define C_RBRKET 44
#define C_LPAREN 45
#define C_RPAREN 46
#define C_COLON 47
#define C_COMMA 48
#define C_BUILTIN 51
#define C_USRLBL 52
#define C_INT 54
#define C_DQUOT 55
#define C_SIZEOF 59
#define C_PLUSPLUS 60
#define C_MINMINUS 61
#define C_INCREMENT 62
#define C_DECREMENT 63
#define C_EXCLAM 64
#define C_AMPERSAND 65
#define C_ASTERISK 66
#define C_MINUS 67
#define C_TILDE 68
#define C_PERIOD 69
#define C_PTRREF 70
#define C_ANDAND 71
#define C_OROR 72
#define C_LONG 74
#define C_DOUBLE 75
#define C_UMOD 76
#define C_URSHFT 77
#define C_UDIV 78
#define C_PLUS 80
#define C_NEG 81
#define C_MULT 82
#define C_SLASH 83
#define C_PERCENT 84
#define C_RSHIFT 85
#define C_LSHIFT 86
#define C_AND 87
#define C_VBAR 88
#define C_CARET 89
#define C_EQEQ 90
#define C_NOTEQ 91
#define C_LT_EQ 92
#define C_LT 93
#define C_GT_EQ 94
#define C_GT 95
#define C_U_LTEQ 96
#define C_U_LT 97
#define C_U_GTEQ 98
#define C_U_GT 99
#define C_QUESTION 100
#define C_PARENS 101
#define C_BKSLSH 102
#define C_SQUOT 104
/*#define C_DQUOT 105*/
#define C_LBLCHR 106
#define C_DIGIT 107
#define C_SPACE 109
/* get_regname register codes */
#define C_RgStk 110
#define C_REG_U 111
#define C_REG_D 112
#define C_REG_X 113
#define C_LEA_RG 116
#define C_REG_Y 118
/*#define C_RGWRD 111
#define C_X_RGWRD 118*/
#define C_EQUAL 120
#define C_REGOFST 123
#define C_FLACC 128   /* Flag Need FLACC */
#define C_I2LNG 131
#define C_LNG2INT 132
#define C_CHR2INT 133
#define C_U2LNG 134
/* Flags specific to gencode () */
#define C_TYFLOAT 135
#define C_TYLONG 136
#define C_BIGMOV 137    /* flag call to _fmove, _dmove, or _lmove */
#define C_FLT2DBL 140
#define C_TOFLOAT 141
#define C_I2DBL 142
#define C_DBL2INT 143
#define C_L2DBL 144
#define C_DBL2LNG 145
#define C_U2DBL 146
#define C_isRgX 147
#define C_isRgY 148
#define C_isRgU 149
#define C_PLUSEQ  160
#define C_MINEQU 161
#define C_X_EQ 162
#define C_DIV_EQ 163
#define C_PCT_EQ 164
#define C_RSH_EQ 165
#define C_LSHEQ 166
#define C_ANDEQ 167
#define C_OREQ 168
#define C_EOREQ 169

/* lbldef->gentype codes */

#define G_ERRNO 1
#define G_STRCT 4
#define G_STRCNAM 10
#define G_LSEEK 56

/* pointer depth/parenthesis masks */
#define P_POINTER 0x10
#define P_BRACE 0x20
#define P_PARENTHESES 0x30

