/* **************************************************************** *
 * copt.h - general header file for c.opt data                      *
 * **************************************************************** */

#ifdef COCO
#   define void int
#else
#   define direct
#endif

#ifdef MAIN
#   define GLOBAL
#else
#   define GLOBAL extern
#endif

#ifdef CMDFLGS
#   define LBRANCH 0x40
#   define LBR 0x60
#   define LBR2BR 0x80
#   define PCREL 0x100
#   define GOTLBLENT 0x200
#   define isbrnch(s) (s)->cmdflgs & 0x20
#   define islngbrnch(s) (s)->cmdflgs & 0x60
#   define ispcrel(s) (s)->cmdflgs & 0x100
#   define oprndislblent(s) (s)->cmdflgs & 0x200
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* A 16-byte structure that is set up as an array in D0146.
 * It is for command substution
 */

typedef struct ww {
    int w0;
    char *ocod01;    /* The opcode for the first command  */
    char *oprnd01;       /* The operand for the first command  */
    char *ocod02;       /* The opcode for the second command  */
    char *oprnd02;       /* The operand for the second command */

            /* altopcod: Determines how the opcode substitution takes place:
             * See altoprnd to see how this works
             * (0 is not used for this field)
             */

    char *altopcod;
    
            /* altoprnd: Determines how the operand substitution takes place.
             * If altoprnd ==
             *      0 - Operand becomes null string
             *      1 - Do nothing - original operand stays the same
             *      2 - Copy operand from second cmd to first
             *      Default: copy string in this field to operand
             *               (opcode for altopcod) of first cmd
             */

    char *altoprnd;
    
            /* The offset into WWList is determined by the str_sum of the
             * chars in the mnemonic.  Some may have the str_sum.  The
             * following member is a pointer to a previous entry at this
             * point, which can be walked backward.
             * If there was no previous entry, this member will be zero */
    
    struct ww *wwprior;    /* ptr to previous entry with same str_sum */
} WW;

/* A 4-byte structure in array LblList initialized in L091f
 * Each element of the list is a tree of labels whose str_sum ()
 * equals the offset into the list
 */

struct t04 {
    struct lblentry *t04_00;
    struct lblentry *finalsum;     /* Last entry in sum list */
};

/* This is an abbreviated form of struct cmdentry (which follows)
 * These two values are stored in D0015-LbrTot */
struct cmdhead {
    struct cmdentry *nxtcmd;
    struct cmdentry *prevcmd;
};

/* A 21-byte structure defining a command line (assembly source) */

typedef struct cmdentry {
    struct cmdentry *nxtcmd;
    struct cmdentry *prevcmd;
    struct lblentry *cmlbl;     /* The label associated with this cmd       */
       int cmdflgs;             /* 0x20 = any branch, 0x40 => long branch
                                 * 0x100 = simple "bra", or pc-rel
                                 * 0x200 = conditional branch               */
    char cmdop[11];    /* The opcode string  (or part of it)      (*08)    */
    void *oprandptr;    /* can be either a char * or ptr to LBLENT (+19)    */
} CMDENT;

/* A 24-byte structure handled in L0946()
 * This structure defines a label name, and sets up a tree of all
 * labels sharing a common address
 */

typedef struct lblentry {
    struct lblentry *sumnxt;    /* Next lblentry in sum list or sumlist entry
                                 * in List if first */
    struct lblentry *sumprev;   /* Prev lblentry in sum list or sumlist entry
                                 * in List if last  */
    CMDENT *lblcmd;             /* (+04)            */
    CMDENT *brdstcmd;             /* (+06)            */        
    struct lblentry *nextme;    /* The next label at this address (+08)     */
    int globlflg;               /* If bit 0 is set, label is global (+10)   */
    char lablnam[12];           /* The label name (+12)                     */
} LBLENT;

#include "proto.h"

/* On COCO version, D0015 was only the first two words of struct cmdentry,
 * probably for memory conservation. However, to avoid numerous warnings
 * without casting, just make it a full struct cmdentry on other systems
 */

#ifdef COCO
GLOBAL direct struct cmdhead D0015;
#else
GLOBAL direct CMDENT D0015;
#endif
GLOBAL direct int LbrTot;
GLOBAL direct unsigned int LbrToBr;
GLOBAL direct CMDENT *CmdDelNxt;
GLOBAL direct int D001f;
GLOBAL direct int CmdRemovd;

/* Init non-DP data */
GLOBAL char P_RegD[]        /* D004b */
#ifdef MAIN
= "d"
#endif
;

GLOBAL char P_RegX[]        /* D004d */
#ifdef MAIN
= "x"
#endif
;

GLOBAL char P_RegY[]        /* D004f */
#ifdef MAIN
= "y"
#endif
;

GLOBAL char P_RegU[]        /* D0051 */
#ifdef MAIN
= "u"
#endif
;

GLOBAL char P_RegD_X[]      /* D0053 */
#ifdef MAIN
= "d,x"
#endif
;

GLOBAL char P_RegD_Y[]      /* D0057 */
#ifdef MAIN
= "d,y"
#endif
;

GLOBAL char P_RegD_U[]      /* D005b */
#ifdef MAIN
= "d,u"
#endif
;

GLOBAL char P_RgDXU[]       /* D005f */
#ifdef MAIN
= "d,x,u"
#endif
;

GLOBAL char P_RgDXYU[]     /* D0065 */ 
#ifdef MAIN
= "d,x,y,u"
#endif
;

GLOBAL char P_RgXUPc[]      /* D006d */
#ifdef MAIN
= "x,u,pc"
#endif
;

GLOBAL char P_RgY_D[]       /* D0074 */
#ifdef MAIN
= "y,d"
#endif
;

GLOBAL char P_RgY_U[]       /* D0078 */
#ifdef MAIN
= "y,u"
#endif
;

GLOBAL char P_RgU_D[]       /* D007c */
#ifdef MAIN
= "u,d"
#endif
;

GLOBAL char P_RgU_Pc[]      /* D0080 */
#ifdef MAIN
= "u,pc"
#endif
;

GLOBAL char P_clra[]        /* D0085 */
#ifdef MAIN
= "clra"
#endif
;

GLOBAL char P_cmpb[]        /* D008a */
#ifdef MAIN
= "cmpb"
#endif
;

GLOBAL char P_cmpd[]         /* D008f */
#ifdef MAIN
= "cmpd"
#endif
;

GLOBAL char P_cmpx[]        /* D0094 */
#ifdef MAIN
= "cmpx"
#endif
;

GLOBAL char P_cmpy[]         /* D0099 */
#ifdef MAIN
= "cmpy"
#endif
;

GLOBAL char P_cmpu[]         /* D009e */
#ifdef MAIN
= "cmpu"
#endif
;

GLOBAL char P_ldb[]         /* D00a3 */
#ifdef MAIN
= "ldb"
#endif
;

GLOBAL char P_ldd[]         /* D00a7 */
#ifdef MAIN
= "ldd"
#endif
;

GLOBAL char P_ldx[]         /* D00ab */
#ifdef MAIN
= "ldx"
#endif
;

GLOBAL char P_ldy[]         /* D00af */
#ifdef MAIN
= "ldy"
#endif
;

GLOBAL char P_ldu[]         /* D00b3 */
#ifdef MAIN
= "ldu"
#endif
;

GLOBAL char P_stb[]         /* D00b7 */
#ifdef MAIN
= "stb"
#endif
;

GLOBAL char P_std[]         /* D00bb */
#ifdef MAIN
= "std"
#endif
;

GLOBAL char P_stx[]         /* D00bf */
#ifdef MAIN
= "stx"
#endif
;

GLOBAL char P_sty[]         /* D00c3 */
#ifdef MAIN
= "sty"
#endif
;

GLOBAL char P_stu[]         /* D00c7 */
#ifdef MAIN
= "stu"
#endif
;

GLOBAL char P_pshs[]         /* D00cb */
#ifdef MAIN
= "pshs"
#endif
;

GLOBAL char P_puls[]         /* D00d0 */
#ifdef MAIN
= "puls"
#endif
;

GLOBAL char P_leax[]         /* D00d5 */
#ifdef MAIN
= "leax"
#endif
;

GLOBAL char P_leay[]        /* D00da[] */
#ifdef MAIN
= "leay"
#endif
;

GLOBAL char P_leau[]        /* D00df[] */
#ifdef MAIN
= "leau"
#endif
;

GLOBAL char P_leas[]        /* D00e4[] */
#ifdef MAIN
= "leas"
#endif
;

GLOBAL char P_sex[]        /* D00e9[] */
#ifdef MAIN
= "sex"
#endif
;

GLOBAL char P_tfr[]        /* D00ed[] */
#ifdef MAIN
= "tfr"
#endif
;

GLOBAL char P_num_0[]        /* D00f1[] */
#ifdef MAIN
= "#0"
#endif
;

GLOBAL char P_two_X[]        /* D00f4[] */
#ifdef MAIN
= "2,x"
#endif
;

GLOBAL char P_one_X[]        /* D00f8[] */
#ifdef MAIN
= "1,x"
#endif
;

GLOBAL char P_zero_X[]        /* D00fc[] */
#ifdef MAIN
= "0,x"
#endif
;

GLOBAL char P_min1_X[]        /* D0100[] */
#ifdef MAIN
= "-1,x"
#endif
;

GLOBAL char P_min2_X[]        /* D0105[] */
#ifdef MAIN
= "-2,x"
#endif
;

GLOBAL char P_two_Y[]        /* D010a[] */
#ifdef MAIN
= "2,y"
#endif
;

GLOBAL char P_one_Y[]        /* D010e[] */
#ifdef MAIN
= "1,y"
#endif
;

GLOBAL char P_zero_Y[]        /* D0112[] */
#ifdef MAIN
= "0,y"
#endif
;

GLOBAL char P_min1_Y[]        /* D0116[] */
#ifdef MAIN
= "-1,y"
#endif
;

GLOBAL char P_min2_Y[]        /* 011b[] */
#ifdef MAIN
= "-2,y"
#endif
;

GLOBAL char P_two_U[]        /* D0120[] */
#ifdef MAIN
= "2,u"
#endif
;

GLOBAL char P_one_U[]        /* D0124[] */
#ifdef MAIN
= "1,u"
#endif
;

GLOBAL char P_0_U[]        /* D0128[] */
#ifdef MAIN
= "0,u"
#endif
;

GLOBAL char P_min1_U[]        /* D012c[] */
#ifdef MAIN
= "-1,u"
#endif
;

GLOBAL char P_min2_U[]        /* D0131[] */
#ifdef MAIN
= "-2,u"
#endif
;

GLOBAL char P_two_S[]        /* D0136[] */
#ifdef MAIN
= "2,s"
#endif
;

GLOBAL char P_0_S[]        /* D013a[] */
#ifdef MAIN
= "0,s"
#endif
;

GLOBAL char P_min2_S[]        /* D013e[] */
#ifdef MAIN
= "-2,s"
#endif
;

GLOBAL char P_immedDP[]        /* D0143[] */
#ifdef MAIN
= "#<"
#endif
;

/* The array D0146 provides alternate commands to substitute */

GLOBAL struct ww D0146[]
#ifdef MAIN
= {
 /* w0   ocod01  oprnd01   ocod02     oprnd02   altopcod   altoprnd   wwprior
   ---   ------  -------   ------    ---------  ---------  ---------  -------
 */
    {2,   P_std,  0,        P_ldd,    (char *)1, (char *)1, (char *)1,   0},
    {2,   P_stx,  0,        P_ldx,    (char *)1, (char *)1, (char *)1,   0},
    {2,   P_stb,  0,        P_ldb,    (char *)1, (char *)1, (char *)1,   0},
    {2,   P_ldd,  0,        P_std,    (char *)1, (char *)1, (char *)1,   0},
    {2,   P_ldx,  0,        P_stx,    (char *)1, (char *)1, (char *)1,   0},
    {1,   P_tfr,  P_RgY_D,  P_pshs,   P_RegD,    (char *)2, P_RegY,      0},
    {1,   P_tfr,  P_RgU_D,  P_pshs,   P_RegD,    (char *)2, P_RegU,      0},
    {2,   P_pshs, P_RegX,   P_pshs,   P_RegD,    (char *)1, P_RegD_X,    0},
    {2,   P_pshs, P_RegU,   P_pshs,   P_RegD,    (char *)1, P_RegD_U,    0},
    {2,   P_pshs, P_RegY,   P_pshs,   P_RegD,    (char *)1, P_RegD_Y,    0},
    {2,   P_pshs, P_RegU,   P_pshs,   P_RegY,    (char *)1, P_RgY_U,     0},
    {2,   P_ldd,  0,        P_cmpd,   P_num_0,   (char *)1, (char *)1,   0},
    {2,   P_ldx,  0,        P_cmpx,   P_num_0,   (char *)1, (char *)1,   0},
    {2,   P_ldy,  0,        P_cmpy,   P_num_0,   (char *)1, (char *)1,   0},
    {2,   P_ldu,  0,        P_cmpu,   P_num_0,   (char *)1, (char *)1,   0},
    {2,   P_sty,  0,        P_cmpy,   P_num_0,   (char *)1, (char *)1,   0},
    {2,   P_stu,  0,        P_cmpu,   P_num_0,   (char *)1, (char *)1,   0},
    {2,   P_ldb,  0,        P_stb,    (char *)1, (char *)1, (char *)1,   0},
    {1,   P_sex,  0,        P_clra,   0,         (char *)2, 0,           0},
    {2,   P_leax, P_one_X,  0,        P_min1_X,  (char *)2, ",x+",       0},
    {2,   P_leay, P_one_Y,  0,        P_min1_Y,  (char *)2, ",y+",       0},
    {2,   P_leau, P_one_U,  0,        P_min1_U,  (char *)2, ",u+",       0},
    {2,   P_leax, P_min1_X, 0,        P_zero_X,  (char *)2, ",-x",       0},
    {2,   P_leay, P_min1_Y, 0,        P_zero_Y,  (char *)2, ",-y",       0},
    {2,   P_leau, P_min1_U, 0,        P_0_U,     (char *)2, ",-u",       0},
    {2,   P_leax, P_two_X,  0,        P_min2_X,  (char *)2, ",x++",      0},
    {2,   P_leay, P_two_Y,  0,        P_min2_Y,  (char *)2, ",y++",      0},
    {2,   P_leau, P_two_U,  0,        P_min2_U,  (char *)2, ",u++",      0},
    {2,   P_leax, P_min2_X, 0,        P_zero_X,  (char *)2, ",--x",      0},
    {2,   P_leay, P_min2_Y, 0,        P_zero_Y,  (char *)2, ",--y",      0},
    {2,   P_leau, P_min2_U, 0,        P_0_U,     (char *)2, ",--u",      0},
    {2,   P_sex,  0,        P_cmpd,   P_immedDP, P_cmpb,    (char *)2,   0},
    {2,   P_clra, 0,        P_cmpd,   P_immedDP, P_cmpb,    (char *)2,   0},
    {2,   P_leas, P_two_S,  P_std,    P_min2_S,  (char *)2, ",s++",      0},
    {2,   P_leas, P_two_S,  P_pshs,   P_RegD,    P_std,     P_0_S,       0},
    {2,   P_pshs, P_RegU,   P_leas,   P_min2_S,  (char *)1, P_RegD_U,    0},
    {2,   P_pshs, P_RegU,   P_leas,   "-4,s",    (char *)1, P_RgDXU,     0},
    {2,   P_pshs, P_RegU,   P_leas,   "-6,s",    (char *)1, P_RgDXYU,    0},
    {0}
}
#endif
;

/*GLOBAL char D03a8[]
#ifdef MAIN
= {0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0
}
#endif
;*/

GLOBAL int GNOTUSED
#ifdef MAIN
= 0
#endif
;

GLOBAL char _chcodes[]
#ifdef MAIN
= {

     0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,
     0x01,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x02,0x00,
     0x38,0x38,0x28,0x28,0x28,0x28,
     0x28,0x28,0x28,0x28,0x00,0x00,
     0x00,0x00,0x00,0x00,0x02,0x22,
     0x22,0x22,0x22,0x22,0x22,0x02,
     0x02,0x02,0x02,0x02,0x02,0x02,
     0x02,0x02,0x02,0x02,0x02,0x02,
     0x02,0x02,0x02,0x02,0x02,0x02,
     0x02,0x00,0x00,0x00,0x00,0x02,
     0x00,0x04,0x04,0x04,0x04,0x04,
     0x04,0x04,0x04,0x04,0x04,0x04,
     0x04,0x04,0x04,0x04,0x04,0x04,
     0x04,0x04,0x04,0x04,0x04,0x04,
     0x04,0x04,0x04,0x00,0x00,0x00,
     0x00,0x00
}
#endif
;

/* The following are library variables ? */
/*GLOBAL int D0438[]
#ifdef MAIN
= {10000, 1000, 100, 10}
#endif
;

GLOBAL int *D0440
#ifdef MAIN
= &D0438[4]
#endif
;

GLOBAL char D0442[]
#ifdef MAIN
= "lx"
#endif
;*/

/* End init non-dp data */

/*GLOBAL int D086b;
GLOBAL char D086d[10];
GLOBAL char D0877[10];
GLOBAL int D0881;
GLOBAL int D0883;
GLOBAL int D0885;*/
