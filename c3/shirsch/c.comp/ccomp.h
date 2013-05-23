/* **************************************************************** *
 * ccomp.h - general header file for cocomp                         *
 *                                                                  *
 * $Id:: ccomp.h 73 2008-10-03 20:20:29Z dlb                      $ *
 * **************************************************************** */

#ifdef MAIN
#   define GLOBAL
#else
#   define GLOBAL extern
#endif

#ifdef COCO
#   define void int
#define NUL1
#define NUL2
#define NUL3
#else
#   define direct
#   define NUL1 , 0
#   define NUL2 , 0, 0
#   define NUL3 , 0, 0, 0

#endif

/* Maximum Label length this needs to be done before "defines.h" is called  */
/* Note: If LBLLEN is changed, "grep '\.8' *.c" ( or one of the values
 * below to find printf formats                                             */

#ifdef COCO
#   define LBLLEN 8
#else
#   define LBLLEN 12
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "proto.h"

GLOBAL direct FILE   *InPath;
GLOBAL direct FILE   *OutPath;
GLOBAL direct int    FileLine;
GLOBAL direct int    ErrCount;
GLOBAL direct int    LblNum;
GLOBAL direct CMDREF *CurntCREF;
GLOBAL direct int    StkUsed;
GLOBAL direct int    StkTot;
GLOBAL direct int    D0013;
GLOBAL direct int    NoStkck;
GLOBAL direct int    DoProfil;
GLOBAL direct int    ModName;
GLOBAL direct int    BlkStkReq;
GLOBAL direct int    D001d;
GLOBAL direct int    InpLinNum;
GLOBAL direct int    Struct_Union; /* D0021 */
GLOBAL direct LBLDEF *G18Current;
GLOBAL direct struct case_ref *D0025;
GLOBAL direct LBLDEF *D0027,
                     *LocLblLast,
                     *LastLbl;      /* The last label in a block */
GLOBAL direct int    FuncGentyp;
GLOBAL direct int    ProgFlow;
GLOBAL direct int    InFunction;
GLOBAL direct int    D0033;
GLOBAL direct int    D0035;
GLOBAL direct int    D0037;
GLOBAL direct struct case_ref *CaseList; /* D0039 */
GLOBAL direct struct case_ref *CaseNow;  /* D003b */
GLOBAL direct int    StrngLngth;
GLOBAL direct int    D003f;
GLOBAL direct int    LblVal;   /* D0041 */
GLOBAL direct char   *InpLinStr;
GLOBAL direct char   CurChr;
GLOBAL direct LBLDEF *LblPtrLow;
GLOBAL direct LBLDEF *LblPtrEnd;
GLOBAL direct char   *CurLinPtr;
GLOBAL direct FILE   *stmpFP;
GLOBAL direct long   D004e;   /* not used ? */
GLOBAL direct int    D0052;   /* 4 bytes - maybe something else */
GLOBAL direct int    D0054;
GLOBAL direct int    StrsCtrl;
GLOBAL direct int    D0058;
GLOBAL direct int    D005a;
GLOBAL direct int    StkLblNum;
GLOBAL direct int    D005e;
GLOBAL direct int    StrTotLngth;
GLOBAL direct LBLDEF *D0062;
GLOBAL direct FILE   *strsFP;
GLOBAL direct int    D0066;
/*GLOBAL direct int D0070;  These final vars are from printf
GLOBAL direct int LJust;
GLOBAL direct int FillChar;
GLOBAL direct int DestFlg;*/

/* The following is a "default" CREF.  The original code had only
 * 20 elements - a CREF has 22.  If we define it as a CREF, the compiler
 * allocates 22 bytes.  In an attempt to match code, we'll define it as
 * an int array.  Coco doesn't mind. but for X-compiler, the extra int
 * doesn't matter, and it keeps gcc quiet :)
 */

#ifdef COCO
GLOBAL int dflt_cref []
#else
GLOBAL CMDREF dflt_cref
#endif
#ifdef MAIN
= {
    FT_INT, INTSIZ, 0, 0, FT_NONDPDAT,
    0, 0, 0, 0, 0
}
#endif
;

GLOBAL char D008c[8]
#ifdef MAIN
= ""
#endif
;

GLOBAL char *DummyNm
#ifdef MAIN
= D008c
#endif
;

#ifdef COCO
GLOBAL char CstrTmp[]
#   ifdef MAIN
= "cstr.XXXXX"
#   endif
;
#endif

/* Duplicate for OSK */
#ifdef OSK
GLOBAL char CstrTmp[]
#   ifdef MAIN
= "cstr.XXXXXX"
#   endif
;
#endif

GLOBAL char xexpcted[]
#ifdef MAIN
= "x expected"
#endif
;

/* These uninit vars are inserted here so that later refs to some
 * of them will work
 */

GLOBAL char    CurFilName[30];
GLOBAL LBLDEF  *NStrLbls[128];
GLOBAL LBLDEF  *StrctLbl[128];
GLOBAL char    InpBuf[256];
GLOBAL char    PrevLine[256];
GLOBAL int     D0784;
GLOBAL int     D0786;
GLOBAL int     D0788;
GLOBAL char    D078a[256];

GLOBAL char *D00ac
#ifdef MAIN
= &InpBuf[255]
#endif
;

/* These defs properly go into comp_10.a */
GLOBAL char *Ofst_y
#ifdef MAIN
= ",y"
#endif
;

GLOBAL char *Ofst_s
#ifdef MAIN
= ",s"
#endif
;

GLOBAL char *P_lbsr
#ifdef MAIN
= "lbsr "
#endif
;

GLOBAL char *P_lbra
#ifdef MAIN
= "lbra "
#endif
;

GLOBAL char *P_clra
#ifdef MAIN
= "clra"
#endif
;

GLOBAL char *UnknOpratr
#ifdef MAIN
= "unknown operator : "
#endif
;

GLOBAL char _chcod01[]
#ifdef MAIN
= {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x6d, 0x40, 0x69, 0x00,
        0x00, 0x54, 0x41, 0x68, 0x2d, 0x2e,
        0x42, 0x50, 0x30, 0x43, 0x45, 0x53,
        0x6b, 0x6b, 0x6b, 0x6b, 0x6b, 0x6b,
        0x6b, 0x6b, 0x6b, 0x6b, 0x2f, 0x28,
        0x5d, 0x78, 0x5f, 0x64, 0x00, 0x6a,
        0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a,
        0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a,
        0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a,
        0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a,
        0x6a, 0x2b, 0x66, 0x2c, 0x59, 0x6a,
        0x00, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a,
        0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a,
        0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a,
        0x6a, 0x6a, 0x6a, 0x6a, 0x6a, 0x6a,
        0x6a, 0x6a, 0x6a, 0x29, 0x58, 0x2a,
        0x44, 0x00
}
#endif
;

GLOBAL char _chcod_2[]
#ifdef MAIN
= {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x09, 0x00, 0x00,
    0x00, 0x0d, 0x0e, 0x00, 0x00, 0x00,
    0x0e, 0x0c, 0x01, 0x0e, 0x0f, 0x0d,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00,
    0x0a, 0x02, 0x0a, 0x03, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x07, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
    0x0e, 0x00
}
#endif
;

/* The following are library variables, I think */
/*GLOBAL int TenMults[]
#ifdef MAIN
= {10000, 1000, 100, 10}
#endif
;

#ifdef COCO
GLOBAL direct int *D0001
#   ifdef MAIN
= &(TenMults[sizeof (TenMults)/sizeof (TenMults[0])])
#   endif
;
#endif
GLOBAL char D088a[10];    printf
GLOBAL char D0894[10];      printf
GLOBAL char D089e;
GLOBAL int D089f;
GLOBAL int D08a1;
GLOBAL int D08a3;*/
