/* ******************************************************************** *     
 * cocodbl.c - converts a host-native double to a coco double array     *
 * ******************************************************************** */

#define uchar unsigned char

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct _tdat {
    int ret;
    char U0000[8];
    char U0008[8];
    char Parm_2[8];
    char U0018;
    char U0019;
    char U001a;
    char U001b;
    char U001c;
    char U001d;
};

#ifdef __STDC__
static int L049a (char *parm1, char *parm2, struct _tdat *tmpdat);
static int L0178 (char *lpt, char curch);
static void coc_dnorm (char *parm_1);
static void L0282 (char *realdbl, struct _tdat *tmpdat);
static void coc_dmul (char *parm1, char *parm2);
static void coc_ddiv (char *numratr,  char *divisr);
static void scale (char *dblarr, int decplaces, int pos_expon);
static int _chadd (char *loc, int addval, int oldcarry);
static int _rol (char *rolchr, int oldcarry);
#else
static int L049a ();
static int L0178 ();
static void coc_dnorm ();
static void L0282 ();
static void coc_dmul ();
static void coc_ddiv ();
static void scale ();
static int _chadd ();
static int _rol ();
#endif

/* Defining atoftbl as const will produce a segfault
 * if we attempt to alter it
 * However, neither COCO nor MW's standard C support "const"
 */

#ifdef __STDC__
const
#endif
char atoftbl[][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80},   /*          0.5 */
    {0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84},   /*           10 */
    {0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87},   /*          100 */
    {0x7a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8a},   /*         1000 */
    {0x1c, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8e},   /*        10000 */
    {0x43, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00, 0x91},   /*       100000 */
    {0x74, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x94},   /*      1000000 */
    {0x18, 0x96, 0x80, 0x00, 0x00, 0x00, 0x00, 0x98},   /*     10000000 */
    {0x3e, 0xbc, 0x20, 0x00, 0x00, 0x00, 0x00, 0x9b},   /*    100000000 */
    {0x6e, 0x6b, 0x28, 0x00, 0x00, 0x00, 0x00, 0x9e},   /*   1000000000 */
    {0x15, 0x02, 0xf9, 0x00, 0x00, 0x00, 0x00, 0xa2},   /*  10000000000 */
    {0x2d, 0x78, 0xeb, 0xc5, 0xac, 0x62, 0x00, 0xc3},   /*      1.0e+20 */
    {0x49, 0xf2, 0xc9, 0xcd, 0x04, 0x67, 0x4f, 0xe4}    /*      1.0e+30 */
};

int *
#ifdef __STDC__
cocodbl (char *str, int *cocdbl)
#else
cocodbl (str, cocdbl)
    char *str;
    int *cocdbl;
#endif
{
    int ndx;
    /*char dblstr[100];*/
    char _mydbl[8];

    int _exponent;
    int _isneg;
    int _negexpon;
    int _decplaces;
    char _curchr;

    while ((*str == ' ') || (*str == '\t'))
    {
        ++str;
    }

    if (*str == '-')
    {
        _isneg = 1;
    }
    else
    {
        _isneg = 0;
    }

    if ((*str == '-') || (*str == '+'))
    {
        ++str;
    }

    memset (_mydbl, 0, sizeof (_mydbl));

    while (isdigit (*str))
    {
        L0178 (_mydbl, *str);    /* _13 */
        ++str;
    }

    _decplaces = 0;

    if (*str == '.')        /* else _17 */
    {
        ++str;
        
        while (isdigit (*str))
        {
            L0178 (_mydbl, *str);    /* _19 */
            ++str;
            ++_decplaces;
        }
    }

    _mydbl[7] = 184;
    
    coc_dnorm (_mydbl);
    
    if (((_curchr = *str) == 'e') || (_curchr == 'E'))    /* else _21 */
    {
        _negexpon = 1;
        ++str;

        if (*str == '+')
        {
            ++str;
        }
        else
        {
            if (*str == '-')
            {
                ++str;
                _negexpon = 0;
            }
        }

        _exponent = 0;     /* _25 */

        while (isdigit (_curchr = *(str++)))
        {
            _exponent = (((_exponent * 10) + _curchr) - '0');
        }

        _decplaces += (_negexpon ? -_exponent : _exponent);
    }

    if (_decplaces < 0)     /* _21 */
    {
        _decplaces = -_decplaces;
        _negexpon = 1;
    }
    else
    {
        _negexpon = 0;
    }

    /* Coco code is: */
    /*_mydbl.dbl = scale (&_mydbl.dbl, _decplaces, _negexpon);*/
    scale (_mydbl, _decplaces, _negexpon);

    if (_isneg)
    {
        _mydbl[0] ^= 0x80;
    }

    /* Now move data to coco storage */

/*movit:*/
    for (ndx = 0; ndx < 8; ndx += 2)
    {
        cocdbl[ndx/2] = (_mydbl[ndx] << 8) | (unsigned char)(_mydbl[ndx + 1]);
    }

    return cocdbl;
}

/* ************************************************************ *
 * L0178                                                        *
 * Passed: (1) address of double storage                        *
 *         (2) current character                                *
 * ************************************************************ */

#ifdef __STDC__

static int
L0178 (char *realdat, char curch)
#else

static int 
L0178 (realdat, curch)
    char *realdat;
    char curch;
#endif
{
    char locdat[8];
    int carry,
        offset,
        count;

    carry = 0;

    for (offset = 6; offset >= 0; offset--)
    {
        carry = _rol (&realdat[offset], carry);
        locdat[offset] = realdat[offset];
    }

    count = 2;

    while (count--)
    {
        carry = 0;

        for (offset = 6; offset >= 0; offset--)
        {
            carry = _rol (&(realdat[offset]), carry);
        }

        if (carry)
        {
            return 1;
        }
    }

    carry = 0;

    for (offset = 6; offset >= 0; offset--)
    {
        carry = _chadd (&realdat[offset], locdat[offset], carry);
    }

    if (carry)
    {
        return 1;
    }

    carry = 0;
    carry = _chadd (&(realdat[6]), curch & 0x0f, carry);

    for (offset = 5; offset >= 0; offset--)
    {
        carry = _chadd (&realdat[offset], 0, carry);
    }

    if (carry)
    {
        return 1;
    }

    return 0;
}


/*Parm_2 equ $0010 
oldstack equ $001e 
Parm_1 equ $0022*/ 

/*_dnorm: ldx   2,s 
 lbsr  _dstack 
 bsr   L0008 
 rts
 */

#ifdef __STDC__

static void
coc_dnorm (char *parm_1)
#else

static void 
coc_dnorm (parm_1)
    char *parm_1;
#endif
{
    char mycpy[8];
    struct _tdat tmpdat;

    /* It might be just as well to work with the original parm_1,
     * not sure
     */
    
    memcpy (mycpy, parm_1, sizeof (mycpy));    /* do we needed this ?? */
    tmpdat.U001d = tmpdat.U0019 = 0;
/* lbsr  L0282 */
    tmpdat.U0000[0] = 0;
    L0282 (mycpy, &tmpdat);

    /* move realdbl to ??? after L0008 is done */
    memcpy (parm_1, mycpy, sizeof (mycpy));
}

/* ************************************************************ *
 * _chsub () subtract a byte value                              *
 * Passed: (1) pointer to destination                           *
 *         (2) ointer to char to subtract from                 *
 *         (3) value to subtract                                *
 *         (4) carry code (sub from dest if set                 *
 * Returns: carry code (CC) resulting from this subtract        *
 * ************************************************************ */

#ifdef __STDC__

static int
_chsub (char *dest, char *loc, int subval, int oldcarry)
#else

static int 
_chsub (dest, loc, subval, oldcarry)
    char *dest;
    char *loc;
    int subval;
    int oldcarry;
#endif
{
    unsigned char sub1,     /* convenience substitutes */
                  sub2;

    sub1 = (uchar)(*loc);
    sub2 = (uchar)(subval & 0xff);

    *dest = (sub1 - sub2 - oldcarry) & 0xff;

    return (sub1 < (sub2 + oldcarry));
}

/* ************************************************************ *
 * _chadd () add a character                                    *
 * Passed: (1) pointer to char to add (and store)               *
 *         (2) value to add to (1)                              *
 *         (3) old carry - add to dest if set                   *
 * Returns: carry code for this add                             *
 * ************************************************************ */

#ifdef __STDC__

static int
_chadd (char *loc, int addval, int oldcarry)
#else

static int 
_chadd (loc, addval, oldcarry)
    char *loc;
    int addval;
    int oldcarry;
#endif
{
    int newcarry;
    int tmpsum;

    tmpsum = (unsigned char)(*loc) + (unsigned char)(addval) + oldcarry;

    if (tmpsum & 0x100)
    {
        newcarry = 1;
    }
    else
    {
        newcarry = 0;
    }

    *loc = tmpsum & 0xff;

    return newcarry;
}

/* ************************************************************ *
 * _ror () - rotates a char one bit right and returns the carry *
 * Passed: (1) pointer to char to rotate                        *
 *         (2) oldcarry - previous carry (to add to char if     *
 *             if set                                           *
 * Returns: carry code for this rotate                          *
 * ************************************************************ */

#ifdef __STDC__

static int
_ror (char *rolchr, int oldcarry)
#else

static int 
_ror (rolchr, oldcarry)
    char *rolchr;
    int oldcarry;
#endif
{
    int newcarry;

    newcarry = *rolchr & 1;

    *rolchr >>= 1;

    if (oldcarry)
    {
        *rolchr |= 0x80;
    }
    else
    {
        *rolchr &= 0x7f;
    }

    return newcarry;
}

/* ************************************************************ *
 * _rol () - rotates a char one bit left and returns the carry  *
 * Passed: (1) pointer to char to rotate                        *
 *         (2) oldcarry - previous carry (to add to char if     *
 *             if set                                           *
 * Returns: carry code for this rotate                          *
 * ************************************************************ */

#ifdef __STDC__

static int
_rol (char *rolchr, int oldcarry)
#else

static int 
_rol (rolchr, oldcarry)
    char *rolchr;
    int oldcarry;
#endif
{
    int newcarry,
        tmprol;

    tmprol = (((*rolchr & 0xff)) << 1) | oldcarry;

    if (tmprol & 0x100)
    {
        newcarry = 1;
    }
    else
    {
        newcarry = 0;
    }

    *rolchr = tmprol & 0xff;

    return newcarry;
}

/*L0282 clr   ,u */
#ifdef __STDC__

static void
L0282 (char *realdbl, struct _tdat *tmpdat)
#else

static void 
L0282 (realdbl, tmpdat)
    char *realdbl;
    struct _tdat *tmpdat;
#endif
{
    /* FIXME made curch unsigned */
    unsigned char curch;
    int frstwrd;

/*L0284 lda   Parm_1,u 
 bmi   L02c5  ora 1+Parm_1,u  ora 2+Parm_1,u  ora 3+Parm_1,u  ora 4+Parm_1,u 
 ora   5+Parm_1,u  ora 6+Parm_1,u  beq L02d9 */

    if ( ! ((curch = realdbl[0]) & 0x80))       /* else L02c5 */
    {
        int offset;

        /* Tests that realdbl != 0 */

        for (offset = 1; offset <= 6; offset++)
        {
            curch |= realdbl[offset] & 0xff;
        }

        if ( ! curch)        /* then L02d9 */
        {
/*L02d9 sta   7+Parm_1,u
 rts*/    
            realdbl[7] = 0;
            return;
        }

/* ldd   Parm_1,u */
        /*FIXME changed "+" to "|" */
        frstwrd = (realdbl[0] << 8) | (unsigned char)realdbl[1];
/*L02a0 dec   7+Parm_1,u 
 bne   L02a8 
 dec   U001d,u */

        do
        {
            int carry,
                frstcarry;

            if ( ! --(realdbl[7]))
            {
                --(tmpdat->U001d);
            }

/*L02a8 asl   ,u*/ 

            carry = _rol (&((tmpdat->U0000)[0]), 0);

/* rol   6+Parm_1,u 
 rol   5+Parm_1,u 
 rol   4+Parm_1,u 
 rol   3+Parm_1,u 
 rol   2+Parm_1,u */
            for (offset = 6; offset >= 2; offset--)
            {
                carry = _rol (&(realdbl[offset]), carry);
            }

/* rolb   
   rola   
   bpl   L02a0 */
            /* This would only apply for first iteration, if frstwrd
             * happened to be set on entry.  In any subsequent iteration,
             * frstcarry will always be 0, because loop exits when
             * frstwrd is negative
             */

            frstcarry = ((frstwrd & 0x8000) != 0);
            
            /* shift left frstwrd with carry from prev */

            frstwrd <<= 1;
            frstwrd += carry;
            carry = frstcarry;  /* Carry from frstwrd lshift */
        } while ( ! (frstwrd & 0x8000));

/* stb   1+Parm_1,u 
 ldb   7+Parm_1,u 
 beq   L02dd */
        realdbl[1] = frstwrd & 0xff;

        curch = frstwrd >> 8;

        if (realdbl[7] == 0)
        {
            goto L02dd;
        }
    }

/*L02c5 ldb   U0019,u 
L02c8 anda  #$7f 
 andb  #$80 
 pshs  b 
 adda  ,s+ 
 sta   Parm_1,u */
    curch &= 0x7f;
    curch += (tmpdat->U0019 & 0x80);
    realdbl[0] =  curch;
/* tst   U001d,u 
 bne   L02dd
L02d8 rts*/

    /*if (tmpdat[29] == 0)*/
    if (tmpdat->U001d == 0)
    {
        return;
    }
L02dd:
/*L02dd lda   U001d,u 
 ldb   7+Parm_1,u 
 subd  #0 
 beq   L02f0 
 bmi   L02f0 */
        /*FIXME changed "+" to "|" */
    frstwrd = (tmpdat->U001d << 8) | (uchar)(realdbl[7]);

    if ((frstwrd != 0) && ! (frstwrd & 0x8000))
    {
        fprintf (stderr, "Error @ L02dd\n");
        /* error - go report it */
/*L02ea ldd   #$0028 
 lbra  _rpterr */
    }

/*L02f0 lbsr  L031b 
 bra   L02ea */
    /* make it inline - looks like if L031b returns, we have an error */
/*L031b clra   
 sta   7+Parm_1,u 
 bra   L0381*/ 

    memset (realdbl, 0, sizeof (realdbl));

    fprintf (stderr, "Missing Variable Reference\n");
    exit (1);
}

/* **************************************************************** *
 * coc_dmul () - Multiplies two coco doubles                        *
 * Passed: (1) - First double to multiply (coco double)             *
 *         (2) - Second double to multiply (coco double)            *
 * Returns: Product stored in parameter (1) (coco double)           *
 * **************************************************************** */

#ifdef __STDC__

static void
coc_dmul (char *parm1, char *parm2)
#else

static void 
coc_dmul (parm1, parm2)
    char *parm1;
    char *parm2;
#endif
{
    struct _tdat tmpdat;
    char exponp1;
    int offset,
        tmpint,
        carry;

/* lbsr  L02f5 */

    if ((exponp1 = L049a (parm1, parm2, &tmpdat)) && parm2[7])  /* else L031b */
    {
/*
L02f5 beq   L031b 
 lda   7+Parm_2,u 
 beq   L031b 
 lbsr  L0397 */
        
        /* L0397 routine is here */
        for (offset = 0; offset <= 6; offset++)        /* L0369 */
        {
            (tmpdat.U0000)[offset] = parm1[offset];
            parm1[offset] = 0;      /* L0381 */
        }

        (tmpdat.U0008)[0] = 0x38;

        do
        {           /* L039e */
            carry = 0;  /* preset carry for following test */

            if ((tmpdat.U0000)[6] & 1)       /* else L03cd */
            {
                /* FIXME 2008-09-12 changed carry from 1 to 0
                 * see L039e in cfloats.a
                 */
                carry = 0;  /* The first iteration does not add carry */

                for (offset = 6; offset >= 0; offset--)
                {
                    carry = _chadd (&(parm1[offset]), parm2[offset], carry);
                }
            }

            /* I _believe_ we pass the carry from the previous loop
             * on to this one
             */

            for (offset = 0; offset <= 6; offset++)      /* L03cd */
            {
                carry = _ror (&(parm1[offset]), carry);
            }

            /* and pass carry on to this one */

            for (offset = 0; offset <= 6; offset++)      /* L03cd */
            {
                carry = _ror (&((tmpdat.U0000)[offset]), carry);
            }

        } while (--((tmpdat.U0008)[0]));  /* end L0397 */
        
        tmpint = (unsigned char)(parm1[7]) + (unsigned char)(parm2[7]) - 0x80;
        parm1[7] = tmpint & 0xff;
        tmpdat.U001d = (tmpint >> 8) & 0xff;

        L0282 (parm1, &tmpdat);

        if ((tmpdat.U0000)[0] < 0)      /* else L05cb */
        {
            char tmpchr,
                 _ttchr;
                        /* L05cb */

            if ((++(parm1[6]) == 0) && (++(parm1[5]) == 0) &&
                    (++(parm1[4]) == 0) && (++(parm1[3]) == 0) &&
                    (++(parm1[2]) == 0) && (++(parm1[1]) == 0)) /* else L0601 */
            {
                _ttchr = tmpchr = parm1[0];
                _ttchr &= 0x7f;

                ++_ttchr;
                if (_ttchr & 0x80)
                {
                    ++parm1[7];
                    _ttchr &= 0x7f;
                }

                parm1[0] =  _ttchr + (tmpchr & 0x80);
            }
        }
    }
    else
    {
        memset (parm1, 0, sizeof (parm1));   /* L031b */
    }           /* end L02f5 */

    /*memcpy (dest, &parm1, sizeof (parm1));*/
}

/* **************************************************************** *
 * coc_ddiv () - Divides two coco doubles                           *
 * Passed: (1) - Numerator to divide        (coco double)           *
 *         (2) - Denominator for the divide (coco double)           *
 * Returns: Quotient stored in parameter (1) (coco double)          *
 * **************************************************************** */

#ifdef __STDC__

static void
coc_ddiv (char *numratr, char *divisr)
#else

static void 
coc_ddiv (numratr, divisr)
    char *numratr;
    char *divisr;
#endif
{
    struct _tdat tmpdat;
    char exponp1;
    int tmpint,
        offset,
        carry;

    exponp1 = L049a (numratr, divisr, &tmpdat);

    if (divisr[7] == 0)        /* lbsr L0321 */
    {
        fprintf (stderr, "Divide by zero.. aborting\n");
        return;     /* ????? */
    }

    if ( ! exponp1)         /* L032c */
    {
        numratr[7] = 0;     /* L031b */
        memset (numratr, 0, sizeof (numratr));      /* L0381 */
        return;
    }

                /* lbsr L03f5 (from 032f */
    for (offset = 0; offset <= 6; offset++)        /* lbsr L0369 */
    {
        tmpdat.U0000[offset] = numratr[offset];
        numratr[offset] = 0;         /* L0381 */
    }

    tmpdat.U0008[0] = 0x39;    /* 57 */

    do
    {
                    /* L03fd */
        carry = 1;  /* set up for in case next condition is false and jump  */

        if ((uchar)((tmpdat.U0000)[0]) >= (uchar)(divisr[0]))   /* else L0434 */
        {
            unsigned int u0int,
                         dvsr;

            carry = 0;  /* Init carry for first subtract, which does not
                         * use carry
                         */

            for (offset = 6; offset >= 2; offset--)
            {
                carry = _chsub ( &((tmpdat.U0008)[offset]),
                                 &((tmpdat.U0000)[offset]),
                                 divisr[offset],
                                 carry);
            }

            u0int = (((tmpdat.U0000)[0] & 0xff) << 8) |
                            ((tmpdat.U0000)[1] & 0xff);
            dvsr = (((divisr)[0] & 0xff) << 8) | (divisr[1] & 0xff);
            /*FIXME: Why do we get better results with uchar commented out?
             * It looks like uchar would be more coco-like
             */

        /*FIXME changed "+" to "|" */
            /*tmpint = ((((tmpdat.U0000)[0]) << 8) |
            //                (uchar)(tmpdat.U0000[1])) -
            //         (((divisr[0]) << 8) | (uchar)(divisr[1])) -
            //          carry;*/

            tmpint =  u0int - dvsr - carry;
            /*carry = 1;*/  /* Set up for case jump to L0434 */

            if (u0int >= (dvsr + carry))
            /*if (tmpint >= 0)*/    /* else L0434 */
            {
                /* preset carry for L0434.  We don't enter this
                 * block with carry set to "1", and none of the
                 * commands in this block affect the carry
                 */

                carry = 0;
                (tmpdat.U0000)[1] = tmpint & 0xff;
                (tmpdat.U0000)[0] = (tmpint >> 8) & 0xff;

                for (offset = 2; offset <= 6; offset++)
                {
                    (tmpdat.U0000)[offset] = (tmpdat.U0008)[offset];
                }
            }
            else
            {
                carry = 1;  /* numeratr < denominator */
            }
        }

L0434:
        for (offset = 6; offset >= 0; offset--)
        {
            carry = _rol (&(numratr[offset]), carry);
        }

        /* Let "carry" pass on to the following loop */

        for (offset = 6; offset >= 0; offset--)
        {
            carry = _rol (&((tmpdat.U0000)[offset]), carry);
        }

        /* Note on the following while.  the original code
         * was rol ,u .. dec 8,u .. bhi ...   dec does not
         * affect the carry, but does the Zero flag.  BHI
         * is branch if Z + C, so apparenly we need to check
         * the carry for the last "rol"
         */

    } while ((--((tmpdat.U0008)[0]) != 0) && (carry == 0));

    if ((tmpdat.U0008)[0] != 0)
    {
        carry = 0;  /* First sub (offset 6) does not sub carry */

        for (offset = 6; offset >= 0; offset--)
        {
            carry = _chsub ( &((tmpdat.U0000)[offset]),
                             &((tmpdat.U0000)[offset]),
                             divisr[offset],
                             carry);
        }

        carry = 0;
        goto L0434;
    }

    /*carry = 0;*/  /* we come here from a "beq" */
    /*(tmpdat.U0000)[0] >>= 1;*/    /* L0482 */
    carry = (_ror (&(tmpdat.U0000)[0], carry));

    for (offset = 0; offset <= 6; offset++)
    {
        numratr[offset] = ~(numratr[offset]);
    }
               /* end L03f5 */

    /* return to L032c */
/* clra   
 ldb   7+Parm_1,u 
 subb  7+Parm_2,u  
 sbca  #0          
 addd  #$0081      
 sta   U001d,u     
 stb   7+Parm_1,u */
    tmpint = (numratr[7] & 0xff) - (unsigned char)divisr[7] + 0x81;
    numratr[7] = tmpint & 0xff;
    tmpdat.U001d = (tmpint >> 8) & 0xff;

/* lda 6,u
 coma
 asra
 ror Parm_1,u
 ror 1+Parm_1,u
 ...
 ror 6+Parm_1,u
*/
    carry =  ! (((tmpdat.U0000)[6]) & 1);

    for (offset = 0; offset <= 6; offset++)
    {
        carry = _ror (&(numratr[offset]), carry);
    }

/* ror ,u */
    carry = _ror (&tmpdat.U0000[0], carry);
/* lbsr  L0284 */       /* from L0321 */

    L0282 (numratr, &tmpdat);

    if ((tmpdat.U0000)[0] & 0x80)      /* then L05cb */
    {
        char tmpchr;
                    /* L05cb */

        if ((++(numratr[6]) == 0) && (++(numratr[5]) == 0) &&
                (++(numratr[4]) == 0) && (++(numratr[3]) == 0) &&
                (++(numratr[2]) == 0) && (++(numratr[1]) == 0)) /* else L0601 */
        {
            tmpchr = numratr[0];
            tmpchr &= 0x7f;

            if (++(tmpchr) & 0x80)
            {
                ++numratr[7];
                tmpchr &= 0x7f;
            }

            numratr[0] =  tmpchr + (numratr[0] & 0x80);
        }
    }

    /* return from sub */
   /* memcpy (dest, parm1, sizeof (parm1));*/
}

/*L049a puls  d 
 pshs  u 
 leas  -30,s 
 tfr   s,u 
 pshs  d 
 clr   U001d,u*/ 

/* ************************************************************ *
 * L049a () - preps the data to be processed                    *
 * Returns: exponent byte of first variable                     *
 * ************************************************************ */

#ifdef __STDC__

static int
L049a (char *parm1, char *parm2, struct _tdat *tmpdat)
#else

static int 
L049a (parm1, parm2, tmpdat)
    char *parm1;
    char *parm2;
    struct _tdat *tmpdat;
#endif
{
    tmpdat->U001d = 0;

    /* we don't do this ... parm2 is passed as a parameter */
/* ldd   6,x 
 std   6+Parm_2,u 
 ldd   4,x 
 std   4+Parm_2,u 
 ldd   2,x 
 std   2+Parm_2,u 
 ldd   ,x 
 stb   1+Parm_2,u 
 tfr   a,b 
 sta   U001c,u */ 
    tmpdat->U001c = parm2[0];
/* ora   #$80 
   sta   Parm_2,u*/
/* eorb  Parm_1,u 
 stb   U0019,u 
 lda   Parm_1,u 
 sta   U0018,u 
 ora   #$80 
 sta   Parm_1,u */
    tmpdat->U0019 = parm1[0] ^ parm2[0];
    tmpdat->U0018 = parm1[0];
    parm1[0] |= 0x80;
    parm2[0] |= 0x80;
/* lda   7+Parm_1,u 
 rts */
    return parm1[7];
}


/* **************************************************************** *
 * scale.c -                                                        * 
 * **************************************************************** */

#ifdef __STDC__

static char *
L0000 (char *_dblar, int _decpl, int pos_expon)
#else

static char *
L0000 (_dblar, _decpl, pos_expon)
    char *_dblar;
    int _decpl;
    int pos_expon;
#endif
{
    if ( _decpl != 0)
    {
        /* The functions called write to the second parameter.
         * Pass a copy so that real table doesn't get trashed
         */

        char atofcpy[8];

        memcpy (atofcpy, atoftbl[_decpl], sizeof (atofcpy));

        if (pos_expon)
        {
            coc_dmul (_dblar, atofcpy);
        }
        else
        {
            coc_ddiv (_dblar, atofcpy);
        }
    }
    
    return _dblar;
}

#ifdef __STDC__

void
scale (char *dblarr, int decplaces, int pos_expon)
#else

void 
scale (dblarr, decplaces, pos_expon)
char *dblarr;
int decplaces;
int pos_expon;
#endif
    /*    +4       +12      +14 */
{
    if (decplaces > 9)          /* else L0079 */
    {
        L0000 (dblarr, (decplaces/10) + 9 , pos_expon);
    }

    L0000 (dblarr, decplaces % 10, pos_expon);
}
