/* **************************************************************** *
 * comp_12.c - part 12 for c.comp                                   *
 *                                                                  *
 * comes from p1_08.c                                               *
 *                                                                  *
 * This file deals with handling of data read in from the input     *
 * file.  It handles initial setup of and storage into the proper   *
 * LBLDEF structures of the data read in.                           *
 *                                                                  *
 * $Id:: comp_12.c 73 2008-10-03 20:20:29Z dlb                  $   *
 * **************************************************************** */


#include <stdlib.h>
#ifndef COCO
#   include <ctype.h>
#   include <unistd.h>
int *cocodbl (
#  ifdef __STDC__
    char *, int *
#  endif
);
#endif

union data_tys {
        long l;
        double d;
        int i;
        char c[9];
};

#include "ccomp.h"

#ifdef COCO
int _atoftbl[] =
{
    0x0000, 0x0000, 0x0000, 0x0081,
    0x2000, 0x0000, 0x0000, 0x0084,
    0x4800, 0x0000, 0x0000, 0x0087,
    0x7a00, 0x0000, 0x0000, 0x008a,
    0x1c40, 0x0000, 0x0000, 0x008e,
    0x4350, 0x0000, 0x0000, 0x0091,
    0x7424, 0x0000, 0x0000, 0x0094,
    0x1896, 0x8000, 0x0000, 0x0098,
    0x3ebc, 0x2000, 0x0000, 0x009b,
    0x6e6b, 0x2800, 0x0000, 0x009e,
    0x1502, 0xf900, 0x0000, 0x00a2,
    0x2d78, 0xebc5, 0xac62, 0x00c3,
    0x49f2, 0xc9cd, 0x0467, 0x4fe4,
};
#endif

static int islblchr (
#ifdef __STDC__
    int
#endif
);

#ifdef COCO
double _dnorm();
#else
#  ifndef OSK
#    ifndef direct
#       define direct
#    endif
#  endif
#endif

static void fget_lbl (
#ifdef __STDC__
    char *
#endif
);

static int
str2num (
#ifdef __STDC__
    int, void *, int *
#endif
);

static void do_squot ();
static void do_dquot ();

static void inizFTbl (
#ifdef __STDC__
    char *, int
#endif
);

static unsigned int str_sum (
#ifdef __STDC__
    char *
#endif
);

static int dobkslsh ();

static void addstrchr (
#ifdef __STDC__
    int
#endif
);

static int is_octal (
#ifdef __STDC__
    char
#endif
);

static int ishexdigit (
#ifdef __STDC__
        char
#endif
);

/*extern direct int StrsCtrl;
extern char _chcod01[];*/

/* ************************************************************ *
 * nxt_word() - Process a group of characters                   *
 *      Sets up D003f and LblVal to appropriate codes           *
 *      If it's a valid Label, finds label or creates an entry  *
 *      If number, retrieves it and sets it up                  *
 *      If assignment or operator, sets up definition for it    *
 * Exit conditions:                                             *
 *      Builtin:                                                *
 *        D003f = C_* type                                      *
 *        LblVal = fnccode (FT_EXTERN if SIZEOF)                *
 *      User Defined Label:                                     *
 *        D003f = C_USRLBL                                      *
 *        LblVal = ptr to LBLDEF struct for this variable       *
 *      Numeric:                                                *
 *        D003f = C_* type of number                            *
 *        LblVal = value itself if size <= sizeof (int)         *
 *                else ptr to the storage of the value          *
 *      Anything else:                                          *
 *        D003f = C_* type                                      *
 *        LblVal = Info regarding any additional chars.  I.E.   *
 *                whether is is "+", "++", "+=", etc            *
 * ************************************************************ */

void 
nxt_word ()
{
    int _ftcode;
    union data_tys _buf;
    double *_valptr;
    double my_dbl;
#ifndef COCO
    int coc_dbl[4];
#endif

    skipblank();

    if (CurChr == -1)
    {
        D003f = -1;
        return;
    }

    InpLinStr = CurLinPtr - 1;
    InpLinNum = FileLine;

    while ((D003f = _chcod01[(int)CurChr]) == '\0')
    {
        reprterr ("bad character");
        getnxtch ();
        InpLinStr = CurLinPtr;
    }

    LblVal = _chcod_2[(int)CurChr];

    switch (D003f)     /* at L626e */
    {
        register void *_treept;

        case 0x6a:         /*alpha - must be label */   /* L5f83 */

            fget_lbl (_buf.c);     /* Read in label name from file */
            _treept = FindLbl (_buf.c);

            /* gentyp = 51 ==> "builtin" ?? */

            if ((D003f = ((LBLDEF *)_treept)->gentyp) == C_BUILTIN)
            {
                /* All builtins except sizeof return unchanged */

                if ((LblVal = ((LBLDEF *)_treept)->fnccode) == FT_SIZEOF)
                {
                   D003f = C_SIZEOF;
                   LblVal = FT_EXTERN;        /* 14 */
                }
            }
            else
            {
               D003f = C_USRLBL;
               LblVal = (int)_treept;
            }

            break;         /* return */

        case 0x6b:                     /*digit */ /* L5fc4 */
#ifdef COCO
            _ftcode = str2num (1, &(my_dbl));
#else
            _ftcode = str2num (1, &(my_dbl), coc_dbl);
#endif
L5fd7:
            _valptr = &(my_dbl);
            
            switch (_ftcode)
            {
                case FT_INT:    /* 1 */     /* L5fe3 */
                    LblVal = *(int *)_valptr;
                    D003f = C_INT;    /* '6' */
                    break;
                case FT_LONG:  /* 8 */    /* L5fe8 */

                    /* I believe that any system we will cross compile
                     * on will have sizeof (int) = sizeof (long)
                     */
#ifdef COCO
                    _treept = addmem (sizeof (long));
                    *(long *)_treept = *(long *)_valptr;
                    LblVal = (int)_treept;
#else
                    LblVal = *(int *)_valptr;
#endif
                    D003f = C_LONG;    /* 'J' */
                    break;
                case FT_DOUBLE:    /* L6004 */
                    /* dblmod */
#ifdef COCO
                    _treept = addmem (sizeof (double));
                    *(double *)_treept = *(double *)_valptr;
#else
                    _treept = addmem (sizeof (struct dbltree));
                    ((struct dbltree *)_treept)->native = *(double *)_valptr;
                    memcpy (((struct dbltree *)_treept)->cocoarr, coc_dbl,
                            sizeof (coc_dbl));
#endif
                    LblVal = (int)_treept;
                    D003f = C_DOUBLE;    /* 'K' */
                    break;
                default:   /* L6020 */
                    reprterr ("constant overflow");
                    LblVal = FT_INT;
                    D003f = C_INT;        /* '6' */
                    break;
            }

            break;

        case  0x68: /* ' (single quote)*/  /* L6049 */
            do_squot ();
            D003f = C_INT;
            break;
        case  0x69: /* " (double quote) */ /* L6051 */
            do_dquot ();
            D003f = C_DQUOT;
            break;
        default:   /* L605c */
            getnxtch ();

            switch (D003f)
            {
                case C_PERIOD: /* '.' */       /* L6064 */
                    if (_chcod01[(int)CurChr] == 0x6b)  /* Digit */
                    {
#ifdef COCO
                        _ftcode = str2num (6, &my_dbl);
#else
                        _ftcode = str2num (6, &my_dbl, coc_dbl);
#endif
                        goto L5fd7;
                    }

                    break;

                case C_AMPERSAND:          /* L608c */
                    switch (CurChr)
                    {
                        case '&':          /* L6093 */
                            D003f = C_ANDAND;
                            getnxtch ();
                            LblVal = 5;
                            break;
                        case '=':        /* L60a0 */
                            D003f = C_ANDEQ;
                            LblVal = 2;
                            getnxtch ();   /* 2008/04/15 fix &= error??? */
                            break;
                    }

                    break;
                case C_EQUAL:              /* L60b8 */
                    if (CurChr == '=')
                    {
                        D003f = C_EQEQ;
                        LblVal = 9;
                        getnxtch ();
                    }

                    break;
                case C_VBAR:               /* L60cb */
                    switch (CurChr)
                    {
                        case '|':          /* L60d2 */
                            D003f = C_OROR;
                            getnxtch ();
                            LblVal = 4;
                            break;
                        case '=':          /* L60df */
                            D003f = C_OREQ;    /* +80 */
                            getnxtch ();
                            LblVal = 2;
                            break;
                    }

                    break;
                case C_EXCLAM:              /* L60fc */
                    if (CurChr == '=')
                    {
                        D003f = C_NOTEQ;
                        getnxtch ();
                        LblVal = 9;
                    }
                    
                    break;
                case C_ASTERISK:             /* L6111 */
                    if (CurChr == '=')
                    {
                        D003f = C_X_EQ;
                        getnxtch ();
                        LblVal = 2;
                    }
                    
                    break;
                case C_SLASH:
                case C_PERCENT:
                case C_CARET:              /* L611e */
                    if (CurChr == '=')
                    {
                        D003f += 80;
                        getnxtch ();
                        LblVal = 2;
                    }
                    
                    break;
                case C_LT:                 /* L612e */
                    switch (CurChr)
                    {
                        case '<':          /* L6135 */
                            D003f = C_LSHIFT;
                            LblVal = 11;
                            getnxtch ();

                            if (CurChr == '=')
                            {
                                D003f = C_LSHEQ;   /* +80 */
                                LblVal = 2;
                                getnxtch ();
                            }
                            
                            break;
                        case '=':          /* L6155 */
                            D003f = C_LT_EQ;
                            LblVal = 10;
                            getnxtch ();
                            break;
                    }

                    break;
                case C_GT:                  /* L616d */
                    switch (CurChr)
                    {
                        case '>':          /* L6174 */
                            D003f = C_RSHIFT;
                            LblVal = 11;
                            getnxtch ();

                            if (CurChr == '=')
                            {
                                D003f = C_RSH_EQ;  /*C_RSHFT + 80*/
                                LblVal = 2;
                                getnxtch ();
                            }

                            break;
                        case '=':              /* L6194 */
                            D003f = C_GT_EQ;
                            LblVal = 10;
                            getnxtch ();
                            break;
                    }

                    break;
                case C_PLUS:                   /* L61ac */
                    switch (CurChr)
                    {
                        case '+':              /* L61b3 */
                            D003f = C_PLUSPLUS;
                            LblVal = 14;
                            getnxtch ();
                            break;
                        case '=':              /* L61bd */
                            D003f = C_PLUSEQ;
                            LblVal = 2;
                            getnxtch ();
                            break;
                    }

                    break;
                case C_MINUS:                  /* L61d4 */
                    switch (CurChr)
                    {
                        case '-':              /* L61db */
                            D003f = C_MINMINUS;
                            LblVal = 14;
                            getnxtch ();
                            break;

                        case '=':              /* L61e5 */
                            D003f = C_MINEQU;  /* C_MINUS + 80 */
                            LblVal = 2;
                            getnxtch ();
                            break;
                        case '>':              /* L61ef */
                            D003f = C_PTRREF;  /* "->" */
                            LblVal = 15;
                            getnxtch ();
                            break;
                    }

                    break;
            }      /* end switch (D003f) for default: */

            break;
    }       /* end outer switch (D003f) */
}

/* ******************************************************************** *
 * tblsetup () - Installs built-in names into the tables                *
 * ******************************************************************** */

void 
#ifdef __STDC__
tblsetup (void)
#else
tblsetup ()
#endif
{
    register LBLDEF *regptr;

    inizFTbl (  "double",   FT_DOUBLE);
    inizFTbl (   "float",   FT_FLOAT);
    inizFTbl ( "typedef",   FT_TYPEDEF);
    inizFTbl (  "static",   FT_STATIC);
    inizFTbl (  "sizeof",   FT_SIZEOF);
    inizFTbl (     "int",   FT_INT);
    
    Struct_Union = 1;       /* Iniz int and float into Struct/Union tables */

    inizFTbl (     "int",   FT_INT);
    inizFTbl (   "float",   FT_FLOAT);

    Struct_Union = 0;       /* Now back to non-struct tables */
    
    inizFTbl (    "char",   FT_CHAR);
    inizFTbl (   "short",   FT_SHORT);
    inizFTbl (    "auto",   FT_AUTO);
    inizFTbl (  "extern",   FT_EXTERN);
    inizFTbl (  "direct",   FT_DIRECT);
    inizFTbl ("register",   FT_REGISTER);
    inizFTbl (    "goto",   FT_GOTO);
    inizFTbl (  "return",   FT_RETURN);
    inizFTbl (      "if",   FT_IF);
    inizFTbl (   "while",   FT_WHILE);
    inizFTbl (    "else",   FT_ELSE);
    inizFTbl (  "switch",   FT_SWITCH);
    inizFTbl (    "case",   FT_CASE);
    inizFTbl (   "break",   FT_BREAK);
    inizFTbl ("continue",   FT_CONTINUE);
    inizFTbl (      "do",   FT_DO);
    inizFTbl ( "default",   FT_DEFAULT);
    inizFTbl (     "for",   FT_FOR);
    inizFTbl (  "struct",   FT_STRUCT);
    inizFTbl (   "union",   FT_UNION);
    inizFTbl ("unsigned",   FT_UNSIGNED);
    inizFTbl (    "long",   FT_LONG);
    
    regptr = FindLbl ("errno");
    regptr->gentyp = 1;
    regptr->fnccode = FT_EXTERN;
    regptr->vsize = INTSIZ;

    regptr = FindLbl("lseek");
    regptr->gentyp = 56;
    regptr->fnccode = FT_NONDPDAT;
    regptr->vsize = LONGSIZ;
}

/* **************************************************************** *
 * fget_lbl () - read a label name from input stream int _dest.     *
 *      ( up to LBLLEN chars), and set file position to end of      *
 *      the string                                                  *
 * **************************************************************** */

static void
#ifdef __STDC__
fget_lbl (register char *_dst)
#else
fget_lbl (_dst)
    register char *_dst;
#endif
{
    int _strln;

    _strln = 1;

    while (islblchr (CurChr) && (_strln <= LBLLEN))
    {
        *(_dst++) = CurChr;
        getnxtch ();
        ++_strln;
    }

    if (_strln == 2)
    {
        *(_dst++) = '_';
    }
    
    *_dst = '\0';

    /* Now drop off any extra characters into the bit bucket */

    while (islblchr (CurChr))
    {
        getnxtch ();
    }
}

/* A _chcodes type function */

static int
#ifdef __STDC__
islblchr (int pos)
#else
islblchr (pos)
int pos;
#endif
{
    return ((_chcod01[pos] == '\x6a') || (_chcod01[pos] == '\x6b'));
}

/* *********************************************** *
 * FindLbl () - Search for a label entry.          *
 *      return entry if found,  create one if none *
 *      found                                      *
 * *********************************************** */

LBLDEF *
#ifdef __STDC__
FindLbl ( char *fnc)
#else
FindLbl ( fnc)
char *fnc;
#endif
{
    LBLDEF **_fnclst;
    LBLDEF **_fncbase;
#ifdef COCO
    int v0;
#endif

    register LBLDEF *_myfnc;

    /* The tree is divided up into two 128-byte trees
     * StrctLbls for structure/union labels, and NStrLbls for all the rest
     */

    _fncbase = (Struct_Union ? StrctLbl : NStrLbls);

    _fnclst = &(_fncbase[str_sum (fnc)]);   /* "lsl d" type thing */
    _myfnc = *(_fnclst);

    /* Search the list for a match, return entry if found */

    while (_myfnc)
    {
        if ((_myfnc->fnam)[0] == fnc[0])        /* first chr of name match? */
        {
            if ( ! strncmp (fnc, _myfnc->fnam, LBLLEN))    /* Check complete name */
            {
                return _myfnc;                      /* Found it... return */
            }
        }

        _myfnc = _myfnc->fnext;
    }

    if ((_myfnc = D0062) != 0)
    {
        D0062 = _myfnc->fnext;
    }
    else
    {
        _myfnc = addmem (sizeof (LBLDEF));
    }

    strncpy (_myfnc->fnam, fnc, LBLLEN);
    _myfnc->gentyp = 0;
    _myfnc->fnccode = 0;

    /* This entry will be inserted as the "first" entry in the list.
     * I. E. the base will point to this entry, this entry's ->fnext
     * will point to the previous "first"
     */

    _myfnc->fnext = *(_fnclst);
    *(_fnclst) = _myfnc;
    _myfnc->ftop = _fnclst;     /* quick trip to the top? */

    return _myfnc;
}

static void
#ifdef __STDC__
inizFTbl (char *name, int fcod)
#else
inizFTbl (name, fcod)
    char *name;
    int fcod;
#endif
{
    char _lblnam[LBLLEN + 1];
    register LBLDEF *_ftbl;

    strncpy (_lblnam, name, LBLLEN);
    _lblnam[LBLLEN] = '\0';
    _ftbl = FindLbl (_lblnam);
    _ftbl->gentyp= C_BUILTIN;
    _ftbl->fnccode = fcod;
}

/* ******************************************************************** *
 * str_sum() - returns the sum of the values  of characters in a string *
 *             The purpose of this is to divide the LBLDEF's into 128   *
 *             lists - to speed up lookups.  For the faster cross       *
 *             compiler machines, I wonder if a sorted tree would work? *
 * Passed : String (name) to calculate.                                 *
 * Returns: The sum or'ed with 127 - the offset into the table          *
 * ******************************************************************** */

static unsigned int
#ifdef __STDC__
str_sum (register char *nam)
#else
str_sum (nam)
    register char *nam;
#endif
{
    unsigned int _sum;
    int _curch;

    _sum = 0;

    while ((_curch = *(nam++)) != '\0')
    {
        _sum += _curch;
    }

    return (_sum & 0x7f);
}

/* ******************************************************************** *
 * addmem () - Return a block of memory of the requested size.          *
 *                                                                      *
 * Note:  We first tried malloc () for non-COCO systems.  This _seemed_ *
 *        to work under Linux, but we encountered a bug using this      *
 *        under OSK.  When a parameter name for a function was the same *
 *        as that of a global function, we would get a Bus Error.  It   *
 *        seems that OSK allocates memory in _descending_ order with    *
 *        malloc () - or at least it did with this application.         *
 *        Note that we have LblPtrLow and LblPtrEnd which keep track    *
 *        of the Lower and Upper bounds respectively, of the allocated  *
 *        data.                                                         *
 *        We first tried updating LblPtrLow each time more memory was   *
 *        requested, (see previous commit) but thought perhaps the      *
 *        allocated structure areas need to be contiguous.  Now we will *
 *        try using  sbrk () with all non-OSK systems, and ibrk ()      *
 *        with OSK.                                                     *
 *                                                                      *
 * ******************************************************************** */

void *
#ifdef __STDC__
addmem (int siz)
#else
addmem (siz)
    int siz;
#endif
{
#ifdef COCO
    int
#else
    void *
#endif
    memptr;

        /* OSK doesn't provide enough contiguous memory for sbrk () to work.
         * use ibrk () which gives memory from _inside_ the data area and
         * provide enough stack space to accomodate its use when compiling
         */
#ifndef _OSK
#   ifdef __WIN32
    if ( ! (memptr = malloc (siz)))
#   else
#   ifdef COCO
    if ((memptr = sbrk (siz)) == -1)
#   else
    if ((memptr = sbrk (siz)) == (void *)-1)
#   endif
#   endif
#else
    if ((memptr = ibrk (siz)) == -1)
    {
        err_quit ("out of memory");
    }
#endif

#ifndef COCO
    /* Hopefully, this will never happen */

    if (memptr < (void *)LblPtrLow)
    {
        fprintf (stderr, "           Warning: memptr < LblPtrLow\n");
    }
#endif

    if ( ! LblPtrLow)
    {
        LblPtrLow = memptr;
    }

    LblPtrEnd = (LBLDEF *)((int)memptr + siz);

    return memptr;
}


/* ******************************************************** *
 * str2num() - reads in a string from the file and converts *
 *             it to the appropriate number value           *
 * Returns: the FT_* type on success, 0 on failure          *
 * ******************************************************** */

#ifndef COCO
static int
#   ifdef __STDC__
str2num (int p1, void *dest_val, int *cocdbl)
#   else
str2num (p1, dest_val, cocdbl)
    int p1;
    void *dest_val;
    int *cocdbl;
#   endif
{
    /* Copy source string to local storage.  This will keep
     * all global pointers updated correctly
     */

    char _tmpstr[50];
    char *_tmpptr,
         *_endptr;
    int cnt;
    /*double _dbl;
    int _ival,*/
    int _ishex;

    cnt = sizeof (_tmpstr);
    _tmpptr = _tmpstr;
    *_tmpptr = '\0';
    _ishex = 0;

    if ((CurChr == '+') || (CurChr == '-'))
    {
        *(_tmpptr++) = CurChr;
        getnxtch ();
        --cnt;
    }

    if ((*_tmpptr = CurChr) == '0')
    {
        *(_tmpptr++) = CurChr;
        getnxtch ();
        --cnt;

        if (toupper (CurChr) == 'X')
        {
            _ishex = 1;
            *(_tmpptr++) = CurChr;
            getnxtch ();
            --cnt;
        }
    }

    while ( (ishexdigit (CurChr)) ||
            (CurChr == '.')       ||
            (CurChr == 'E')       ||
            (CurChr == 'e')         )
    {
        char __prevchr;

        __prevchr = *(_tmpptr++) = CurChr;

        if ((--cnt) <= 1)
        {
            break;
        }

        getnxtch ();

        if ( ((__prevchr == 'E') || (__prevchr == 'e')) &&
             ((CurChr == '+') || (CurChr == '-'))          )
        {
            *(_tmpptr++) = CurChr;
            getnxtch ();

            if (--cnt <= 1)
            {
                break;
            }
        }
    }

    *_tmpptr = '\0';

    if ( ! (_ishex)                 && 
#ifndef _WIN32
           (index (_tmpstr, '.') ||
            index (_tmpstr, 'e') ||
            index (_tmpstr, 'E')   )
#else
           (strchr (_tmpstr, '.') ||
            strchr (_tmpstr, 'e') ||
            strchr (_tmpstr, 'E')   )
#endif
       )
    {
        *((double *)dest_val) = strtod (_tmpstr, &_endptr);
        cocodbl (_tmpstr, cocdbl);      /* dblmod */
        return FT_DOUBLE;
    }

    /**(long *)dest_val = (int)strtol (_tmpstr, &_endptr, 0);*/

    if ((toupper (CurChr)) == 'L')
    {
        *((long *)dest_val) = strtol (_tmpstr, &_endptr, 0);
        getnxtch ();
        return FT_LONG;
    }
    else
    {
        *((int *)dest_val) = (int)strtol (_tmpstr, &_endptr, 0);
        return FT_INT;
    }
}

#else

/* COCO version of str2num () */

extern double scale ();

static int 
str2num (p1, dest_val)
    int p1;
    register double *dest_val;
{
    double _number;
    int _expon;
    int _negflg;
    int v2;
    char *nptr;

    v2 = 0;
    _number = 0;

    nptr = (char *)(&_number);

    if (p1 == 6)
    {
        goto do_decimal;
    }

                            /* L6684 */
    if (CurChr == '0')          /* '0' octal or "0x" ? */  /* else L6791 */
    {
        long _my_nmbr;
        long *_xo_numptr;

        getnxtch ();

        if (CurChr == '.')
        {
            getnxtch ();
            goto do_decimal;
        }

        _my_nmbr = 0;       /* L66a0 */

        if ((CurChr =='x') || (CurChr == 'X'))  /* 'x' hex */  /* else L6733 */
        {
            int myint;

            getnxtch ();

            while ((myint = ishexdigit (CurChr)))       /* L66f6 */
            {
                _my_nmbr = ((_my_nmbr << 4) +
                        (myint - ((myint >= 'A') ? 'A' - 10 : '0')));
                getnxtch ();
            }
        }
        else        /* It's an octal # */        /*L6733*/
        {
            while (is_octal (CurChr))
            {
                _my_nmbr = ((_my_nmbr << 3) + (CurChr - '0'));
                getnxtch ();
            }
        }

        /* L673f */
        _xo_numptr = &_my_nmbr;

        if ((CurChr == 'L') || (CurChr == 'l')) /* Long */   /* else L6757 */
        {
            getnxtch ();
            goto retrnlong;
        }

        if ((*(int *)_xo_numptr) == 0)        /* L6757 */
        {
            *(int *)dest_val = (int)_my_nmbr;
            return FT_INT;
        }

retrnlong:
        *(long *)dest_val = _my_nmbr;
        return FT_LONG;
    }

    /* The following is a modified (?) version of atof
     * We'll try to follow the original code, but when we port,
     * we'll try to use std library routines
     */

    while (_chcod01[CurChr] == 0x6b)    /* digit */      /* L6791 */
    {
        if (L9990 (&(_number), CurChr))     /* else L67d5 ?????? */
        {
            getnxtch ();
            return 0;
        }

        getnxtch ();
    }

    if ((CurChr == '.') || (CurChr == 'e') || (CurChr == 'E')) /* else L68bb */
    {
        /* L67b4 */
        if (CurChr == '.')       /* else L67f4 */
        {
            getnxtch ();           /* go to L67e5 */

do_decimal:
            while (_chcod01[CurChr] == 0x6b)    /* digit */ /*L67e5*/
            {
                if (L9990 (&(_number), CurChr))
                {
                    getnxtch ();       /* go to L68cb */
                    return 0;
                }
                else
                {
                     getnxtch ();
                     ++v2;
                }
            }
        }

        /*L67f4*/
        nptr[7] = 184;
/*#asm
        leax 8,s
        pshs x
        leax 10,s
        pshs x
        lbsr _dnorm
        leas 2,s
        lbsr _dmove
#endasm*/
        /* I believe the following accomplishes the above "asm" */
        _number = _dnorm (&_number);

        if ((CurChr == 'E') || (CurChr == 'e'))   /* else L6887 */
        {
            _negflg = 1;
            getnxtch ();

            if (CurChr == '+')
            {
                getnxtch ();
            }
            else
            {
                if (CurChr == '-')
                {
                    getnxtch ();
                    _negflg = 0;
                }
            }
            
            _expon = 0;

            while (_chcod01[CurChr] == 0x6b)    /* digit */
            {
                _expon = ((_expon * 10) + (CurChr - '0'));
                getnxtch ();
            }

            if (_expon >= 40)
            {
                return 0;
            }

            v2 += ((_negflg != 0) ? -_expon : _expon);
        }
                    
        /* L6887 */
        if (v2 < 0)
        {
            v2 = -v2;
            _negflg = 1;
        }
        else
        {
            _negflg = 0;
        }

        for (;;) {}

        /* FIXME    see if we can do the following in "C" */
        /**(double *)dest_val = _number;*/
/*#asm
        leax  0,u
        pshs x
        ldd 6,s
        pshs d
        ldd 6,s
        pshs d
        leax 14,s
        lbsr _dstack
        lbsr scale
        leas 12,s
        lbsr _dmove
        bra rtdb
#endasm*/

        /* I believe the following accomplishes what the above "asm" does */
        *(double *)dest_val = scale (_number, v2, _negflg);
        return FT_DOUBLE;
    }

    /* L68bb */
    if (((nptr)[0] != '\0') || ((nptr)[1] != '\0') ||
                                  ((nptr)[2] != '\0'))
    {
        return 0;
    }

    if ((CurChr == 'l') || (CurChr == 'L'))       /* L68d0 */
    {
        char *e0;

        getnxtch ();
retlong2:
        e0 = &((nptr)[3]);
        *(long *)dest_val = *(long *)e0;
        return FT_LONG;
    }

    /*L68fb*/
    if (((nptr)[3]) || ((nptr)[4]))
    {
        goto retlong2;
    }
    else            /*L690c*/
    {
        /* This seems a round-about-way, but it's the first way I
         * got the code to come out like the source */
        union {
            char *cp;
            int *ip;
        } b0;

        b0.cp = (nptr)+5;
        /*dest_val->i = *(b0.ip);*/
        *(int *)dest_val = *(b0.ip);

        return FT_INT;
    }
}

/* The following two functions _may_ be converted to C
 * We'll check later */

#asm
scale pshs  u 
 ldd   12,s 
 cmpd  #9 
 ble   L6955 
 leax  4,s 
 pshs  x 
 ldd   16,s 
 pshs  d 
 ldd   16,s 
 pshs  d 
 ldd   #10 
 lbsr  ccdiv 
 addd  #9 
 pshs  d 
 leax  10,s 
 lbsr  _dstack 
 bsr   L6970 
 leas  12,s 
 lbsr  _dmove 
L6955 ldd   14,s 
 pshs  d 
 ldd   14,s 
 pshs  d 
 ldd   #10 
 lbsr  ccmod 
 pshs  d 
 leax  8,s 
 lbsr  _dstack 
 bsr   L6970 
 leas  12,s 
 puls  u,pc 
L6970 pshs  u 
 ldd   12,s 
 beq   L69b6 
 leas  -2,s 
 leax  _atoftbl,y 
 stx   0,s 
 ldd   0,s 
 pshs  d 
 ldd   16,s 
 lslb   
 rola   
 lslb   
 rola   
 lslb   
 rola   
 addd  ,s++ 
 std   0,s 
 ldd   16,s 
 beq   L69a0 
 leax  6,s 
 lbsr  _dstack 
 ldx   8,s 
 lbsr  _dmul 
 bra   L69aa 
L69a0 leax  6,s 
 lbsr  _dstack 
 ldx   8,s 
 lbsr  _ddiv 
L69aa leau  _flacc,y 
 pshs  u 
 lbsr  _dmove 
 leas 2,s
 puls u,pc
L69b6 leax  4,s 
 leau  _flacc,y 
 pshs  u 
 lbsr  _dmove 
 puls  u,pc 
#endasm
#endif

static void 
do_squot ()
{
    getnxtch ();

    if (CurChr == '\\')
    {
        LblVal = dobkslsh ();
    }
    else
    {
        LblVal = CurChr;
        getnxtch();
    }

    if (CurChr != '\'')
    {
        reprterr ("unterminated character constant");
        return;
    }

    getnxtch ();
    return;
}

static void 
do_dquot ()
{
    switch (StrsCtrl)
    {
        case 0:         /* Open strings tmp file if not already opened */
        case 2:         /* L69f9 */
            if ( ! stmpFP)
            {
#ifdef COCO
                if ( ! (stmpFP = fopen (CstrTmp, "w+")))
#else
#  ifdef OSK
                if ( ! (stmpFP = fopen (CstrTmp, "w+")))
#  else
                if ( ! (stmpFP = tmpfile ()))
#  endif
#endif
                {
                    err_quit ("can't open strings file");
                }
            }

            strsFP = stmpFP;
            break;
        case 1:     /* L6a21 */
            strsFP = OutPath;
    }

    StrTotLngth = 0;

    if (StrsCtrl != 1) /* 1 => to output file */
    {
        fprintf (strsFP, "%c%d", '_', LblVal = ++LblNum);
    }

    getnxtch();

    while (CurChr != '"')
    {
        /*if (CurLinPtr == inpbuf)*/
        if ( ! (CurLinPtr - InpBuf))
        {
            reprterr ("unterminated string");
            break;
        }
        else
        {
            if (CurChr == '\\')
            {
                addstrchr (dobkslsh ());
            }
            else
            {
                addstrchr (CurChr);
                getnxtch();
            }
        }
    }

    addstrchr (0);
    putc ('\n', strsFP);
    StrngLngth = 0;
    getnxtch();
}

/* **************************************************************** *
 * addstrchr () - Adds a character to the string buffer             *
 *      Automatically inserts "fcc " and also does "fcb " for       *
 *      non-printables.  Creates new "fcc" if line length exceeds   *
 *      max.  Updates total string length.                          *
 * **************************************************************** */

static void
#ifdef __STDC__
addstrchr (int ch)
#else
addstrchr (ch)
    int ch;
#endif
{
    if ( ! StrngLngth)
    {
        fprintf (strsFP, " fcc \"");
    }

    if ((ch < ' ') || (ch == '"'))  /* L6ad6 */
    {
        fprintf (strsFP, "\"\n fcb $%x\n", ch);
        StrngLngth = 0;             /* Reset flag to start new "fcc" line */
    }
    else        /* L6afb */
    {
        putc (ch, strsFP);
        
        if ((StrngLngth++) >= 75)
        {
            fprintf (strsFP, "\"\n");
            StrngLngth = 0;
        }
    }

    ++StrTotLngth;
}

void
#ifdef __STDC__
prnt_rzb (int valu)
#else
prnt_rzb (valu)
    int valu;
#endif
{
    fprintf (strsFP, " rzb %d\n", valu);

}

static int 
#ifdef __STDC__
dobkslsh (void)
#else
dobkslsh ()
#endif
{
    int _lngth;
    register int _curch;

    getnxtch ();
    _curch = CurChr;
    getnxtch ();

    /* If it's a standard escape char, simply return */

    switch (_curch)     /* case @L6091 */
    {
        case 'n':       /* L6b74 */
            return 13;
        case 'l':       /* L6b5c */
            return 10;
        case 't':       /* L6b62 */
            return 9;
        case 'b':       /* L6b68 */
            return 8;
        case 'v':       /* L6b6e */
            return 11;
        case 'r':       /* L6b74 */
            return 13;
        case 'f':       /* L6b7a */
            return 12;
        case '\n':      /* Line continuation */
#ifndef COCO
#  ifndef OSK
        case '\r':
#  endif
#endif
            return ' ';
    }

    if (_curch == 'x')      /* \x = hexadecimal value */
    {          /* else L6c0c */
        int pt;

        _curch = _lngth = 0;
        
        while ((pt = ishexdigit (CurChr)) && (_lngth++ < 2))
        {
            /* L6bc2 */
            _curch = (_curch << 4) +
                        ((pt < 'A') ? (pt - '0') : pt - ('A' - 10));
            getnxtch ();
        }
    }
    else
    {
        /*L6c0c*/
        if (_curch == 'd')          /* else L6c54 */
        {
            _curch = _lngth = 0;

            while ((_chcod01[(int)CurChr] == 0x6b) && (_lngth++ < 3))
            {
                _curch = ((_curch * 10) + CurChr - '0');
                getnxtch ();
            }

        }
        else
        {
            if (is_octal (_curch))
            {
                _curch -= '0';
                _lngth = 0;

                while ((is_octal(CurChr)) && (_lngth++ < 3))
                {
                    _curch = ((_curch << 3) + CurChr - '0');
                    getnxtch ();
                }
            }
        }
    }

    return _curch;
}

static int
#ifdef __STDC__
is_octal (char ch)
#else
is_octal (ch)
    char ch;
#endif
{
    return ((ch <= '7') && (ch >= '0'));
}

/* ******************************************************************** *
 * ishexdigit () - a personified version of 'isxdigit()'                *
 * NOTE: I tried the stdlib isxdigit() and it DID NOT work.             *
 * ******************************************************************** */

static int 
#ifndef __STDC__
ishexdigit (ch)
    char ch;
#else
ishexdigit (char ch)
#endif
{
    if ((_chcod01[(int)ch] == 0x6b) ||
            (((ch &= 0x5f) >= 'A') && (ch <= 'F')))
    {
        return ch;
    }
    else
    {
        return 0;
    }
}
