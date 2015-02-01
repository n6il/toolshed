
/*
 *------------------------------------------------------------------
 *
 * $Source$
 * $RCSfile$
 * $Revision$
 * $Date$
 * $State$
 * $Author$
 * $Locker$
 *
 *------------------------------------------------------------------
 *
 * Carl Kreider (71076.76@compuserve.com, crkreider@delphi.com)
 * Syscon International Inc
 * 1108 S. High Street
 * South Bend, IN  46601-3796
 * (219) 232-3900
 *
 *------------------------------------------------------------------
 * $Log$
 * Revision 1.3  2008/10/30 15:52:24  boisy
 * Clenaed up warnings in ar2
 *
 * Revision 1.2  1996/07/20 22:30:23  cc
 * Merged in pwz's unixification (Sunos).
 *
 * Revision 1.1  96/07/20  17:10:42  cc
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

/*
 * header file for LZ compression routines
 */

#ifndef STATIC
# ifdef SYSV
#   define	STATIC	static
#  else
#   ifdef OSK
#    define	STATIC	static
#   else
#    define	STATIC	static direct
#  endif
# endif
#endif

#ifndef WORD
# ifdef SYSV
#   define WORD short
#   define UWORD unsigned short
#   define VOID void
# else
#  ifdef  OSK
#   define WORD short
#   define UWORD unsigned short
#   define VOID void
#  else
#   define WORD int
#   define UWORD unsigned int
#   define VOID int
#  endif
# endif
#endif

#define BITS		14				/* maximum number of bits/code		*/
#define OLD_BITS	11				/* old maximum number of bits/code	*/
#define DEF_BITS	12				/* default number of bits/code		*/
#define INIT_BITS	9				/* initial number of bits/code		*/
#define COMP		0				/* we are compressing				*/
#define DECOMP		1				/* we are decompressing				*/

/*
 * One code can represent 1 << BITS characters, but to get a code of
 * length N requires an input string of at least N * (N - 1) / 2
 * characters. To overflow the decompress stack, an input file would
 * have to have at least MAXSTACK * (MAXSTACK - 1) / 2 consecutive
 * occurrences of a particular character, which is unlikely for the
 * value used here. (Do keep the initial advice in mind, though.)
 */
#define MAXSTACK	2000			/* size of output stack				*/

#define TAG			2995			/* suggested by M. Meyer			*/

/*
 * The following should be changed to fit your machine and the type
 * you choose for the elements of the array buf.
 * (If you avoid insert_bit() and fetch(), never mind.)
 */
#define LOG2WSIZE	4				/* log2(size of base type of buf)	*/
#define WSIZE		16				/* size of base type of buf			*/

#define BytesToBits(b)	((b) << 3)
#define LowOrder(n)		(~(~0 << (n)))	/* thanks to K & R				*/
#define HighOrder(n)	(~0 << (n))

typedef struct {
	UWORD	next,					/* chain of entries with same prefix*/
			chain,					/* chain prefixed with this entry	*/
			suffix;					/* last char in this entry			*/
	} COMPTBL;

typedef struct {
	UWORD	prefix,					/* prefix code for this entry		*/
			lastch;					/* last char in this entry			*/
	} DCOMPTBL;

extern WORD		maxbits,			/* user settable max # bits/code	*/
				n_bits,				/* initial number of bits/code		*/
				maxmaxcode,			/* max permissible maxcode value	*/
									/* (i.e. 2 ** BITS - 1)				*/
				maxcode,			/* 2 ** n_bits - 1					*/
				free_ent,			/* first unused entry				*/
				offset;				/* cursor into buf (units of bits)	*/

extern long		lz_bytes;
extern UWORD	buf[BITS];
extern COMPTBL	*CompTbl;
extern DCOMPTBL	*CrakTbl;
extern int	debug;

int lz1_config(int bits);
WORD de_LZ_1(FILE *infile, FILE *outfile, long bytes);
int LZ_1(FILE *infile, FILE *outfile, long *bytes);
int readbuf(int cnt, FILE *fp);
int fetch(void);
int writeshort(FILE *fp, short s);
int readshort(FILE *fp, short *sp);
int readushort(FILE *fp, unsigned short *sp);
