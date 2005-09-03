#ifndef COCO
#define direct
#endif

#ifndef MAIN
#define GLOBAL extern
#else
#define GLOBAL
#endif

/* symbol table types */
/* symbol definition/reference type/location */
/* type flags */
#define CODENT      0x04	/* data/code flag */
/* data type flags */
#define DIRENT      0x02	/* global/direct flag */
#define INIENT      0x01	/* clear/init. data flag */
/* location flags */
#define CODLOC      0x20	/* data/code flag */
#define DIRLOC      0x10	/* global/direct flag */
#define LOC1BYT     0x08	/* two/one byte size flag */
#define LOCMASK     (CODLOC|DIRLOC)
#define NEGMASK     0x40	/* negate on resolution */
#define RELATIVE    0x80	/* relative reference */

#include "cocotype.h"
#include <stdio.h>
#include <string.h>
#include "rstruct.h"

GLOBAL direct char *incdirs[32];
GLOBAL direct int  inccount
#ifdef MAIN
 = 0
#endif
		;

GLOBAL direct char e_flg
#ifdef MAIN
= ON
#endif
,
                x_flg
#ifdef MAIN
= ON
#endif
               ;
GLOBAL direct int PgDepth
#ifdef MAIN
= 66
#endif
,
                PgWidth
#ifdef MAIN
= 80
#endif
               ;
GLOBAL direct FILE *SrcFile,	/* D0015 */
               *OutFile,	/* D0017 */
               *PrtPth;
GLOBAL direct int NumBytes,
                d001d,
                jsrOfst,
                d0021,
                had_err;
GLOBAL direct unsigned d0025,
                d0027,
                _If_Dpth,
                _IfIsTru;
GLOBAL direct unsigned ListPag,
                ListLin;	/* d002d, d002f */
GLOBAL direct long nmbr_int;
GLOBAL direct unsigned char nmbr_str[8];	/* Little-Endian ordered
						 * string */
GLOBAL direct short d0033;
GLOBAL direct int HadBracket,
                HadArrow;
GLOBAL direct int d0039[2];	/* ???   */
GLOBAL direct int doList,
                IsExtrn,
                Glbls,
                dGlbls,
                edatas,
                ddatas;
GLOBAL direct unsigned int d0049,
               *S_Addrs;
GLOBAL direct int CodeSize;
GLOBAL direct unsigned int *Adrs_Ptr;
GLOBAL direct unsigned int d0051,
               *d0053;
GLOBAL direct short GlobCnt,
                ref_cnt,	/* D0055, D0057 */
                comn_cnt,
                locl_cnt;
GLOBAL direct char Pass2,	/* D005d */
                d005e,
                d005f,
                d0060,
                LblTyp,
                d0062,
                d0063;
GLOBAL direct char *SrcChar,	/* d0064 originally */
               *Label,
               *operatr,
               *d006a,
               *oprand,
               *coment;
GLOBAL direct JMPTBL *jmp_ptr;
GLOBAL direct CODFORM *Cod_Ent;
GLOBAL direct CODFORM *CodTbBgn,
               *CodTbEnd;
GLOBAL direct struct symblstr **d0078,
               *SmblDsc,	/* D007a */
               *d007c;

/*
 * direct char d007e, d007f[7], *d0086[6], d0092, d0093[7], *d009a[6], d00a6,
 * d00a7[7], *d00ae[6];
 */
/* Testing...  if it works for OSK, then the following is struct symblstr */
GLOBAL direct struct symblstr d007e,
                d0092,
                d00a6;

/*
 * delete this.. try to make whole thing a single structure.. seems that
 * these aren't inline in linux, at least
 */

/*
 * GLOBAL direct char prebyt, OpCod; GLOBAL direct union storer1 C_Oprnd,
 * *opr_ptr;
 */
GLOBAL direct struct cmdbytes MLCod;
GLOBAL direct char *opr_ptr;
GLOBAL direct char AIMFlg;
GLOBAL direct int undeflbl;
GLOBAL direct char C_Flg,
                ff_flg,
                g_flg,
                d00c4,
                ListFlg,
                listON,
                d00c7,
                OFlg,
                S_Flg,
                R_Keep;		/* D00c8, D00c9 */
GLOBAL direct int list_lin;	/* D00ca */
GLOBAL direct char *CmdCod;
GLOBAL direct int InMacro,
                MacFlag,
                d00d2,
                d00d4;
GLOBAL direct MACDSCR *now_mac;
GLOBAL direct int d00d8,
                d00da;

GLOBAL char     TmpNam[30]
#ifdef MAIN
= "rma.tmp"
#endif
               ;
GLOBAL char     AltLbl[10],
                LabelName[10];
GLOBAL char     cmntlen[10];	/* To hold " %1.xs val for comment length */
GLOBAL char     d0587[10],
                _nam[128],
                _ttl[128];
GLOBAL char     d0681[10];
GLOBAL long     reptFpos;
GLOBAL struct symblstr *d068f[128],
               *d078f;
GLOBAL struct ref_ent d0791[10],
               *OptBPtr;
GLOBAL struct symblstr **d07b1;

#include "rtables.h"

GLOBAL long     RofHdLoc,
                d07b7,
                d07bb,
                d07bf;

/* Following is from part4.c */
GLOBAL direct char Is_W;	/* Flag that we are indexing off W  */

/* Following is from part8.c */
GLOBAL FILE    *FrstFil[12];
GLOBAL char    *SrcNam[32],
                d0821[256];

GLOBAL FILE   **ThisFile
#ifdef MAIN
= FrstFil
#endif
               ;
GLOBAL char   **LastSrc
#ifdef MAIN
= SrcNam
#endif
               ;

GLOBAL char    *O_Nam
#ifdef MAIN
= 0
#endif
               ;
GLOBAL int      SColumn
#ifdef MAIN
= 0
#endif
               ;
GLOBAL int      rversion
#ifdef MAIN
= 1
#endif
               ;
GLOBAL direct FILE *macfp1,
               *macfp2;		/* d00dc, d00de */

/* This is a special version of chcodes required for "rma" */

GLOBAL unsigned char _chcodes[]
#ifdef MAIN
= {
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
	'\x00', '\x00', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00',
	'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x02', '\x00',
	'\x38', '\x38', '\x28', '\x28', '\x28', '\x28', '\x28', '\x28',
	'\x28', '\x28', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
	'\x02', '\x22', '\x22', '\x22', '\x22', '\x22', '\x22', '\x02',
	'\x02', '\x02', '\x02', '\x02', '\x02', '\x02', '\x02', '\x02',
	'\x02', '\x02', '\x02', '\x02', '\x02', '\x02', '\x02', '\x02',
	'\x02', '\x02', '\x02', '\x00', '\x00', '\x00', '\x00', '\x02',
	'\x00', '\x04', '\x04', '\x04', '\x04', '\x04', '\x04', '\x04',
	'\x04', '\x04', '\x04', '\x04', '\x04', '\x04', '\x04', '\x04',
	'\x04', '\x04', '\x04', '\x04', '\x04', '\x04', '\x04', '\x04',
	'\x04', '\x04', '\x04', '\x00', '\x00', '\x00', '\x00', '\x00'
}

#endif
               ;

#include "proto.h"
