/* ************************************************************************ *
 * comp_13.c - part 13 for c.comp                                           *
 *                                                                          *
 * comes from p2_02.c                                                       *
 *                                                                          *
 * $Id:: comp_13.c 73 2008-10-03 20:20:29Z dlb                            $ *
 * ************************************************************************ */

#include "ccomp.h"

int BitMsks[] = {
    0x0002, 0x0004, 0x0008, 0x0010,
    0x0020, 0x0040, 0x0080, 0x0100,
    0x0200, 0x0400, 0x0800, 0x1000,
    0x2000, 0x4000
};

static void L6eec (
#ifdef __STDC__
    CMDREF *
#endif
);

static void L717c (
#ifdef __STDC__
    int, CMDREF *
#endif
);

static void L74cb (
#ifdef __STDC__
    CMDREF *
#endif
);

static int L771d (
#ifdef __STDC__
    CMDREF *
#endif
);

static int is_ref (
#ifdef __STDC__
    CMDREF *
#endif
);

static void L77e6 (
#ifdef __STDC__
    CMDREF *
#endif
);

static void L794a (
#ifdef __STDC__
    CMDREF *, int
#endif
);

static void L7b83 (
#ifdef __STDC__
    CMDREF *
#endif
);

static void L7d19 (
#ifdef __STDC__
    CMDREF *, int
#endif
);

static void L7e63 (
#ifdef __STDC__
    CMDREF *
#endif
);

static void L7e9c (
#ifdef __STDC__
        CMDREF *cref
#endif
);

/* ************************************************************ *
 * reg2d_add () - generates code to tfr R,d, addd <something>   * 
 * ************************************************************ */

void
#ifndef __STDC__
reg2d_add (cref)
    register CMDREF *cref;
#else
reg2d_add (CMDREF *cref)
#endif
{
    L6eac (cref);

    if (cref->vartyp != C_REG_D)
    {
        /* The following generates :
         * tfr somereg,d
         * addd <whatever>
         */
        gencode (117, C_REG_D, 119, cref);
        cref->vartyp = C_REG_D;
    }
}

/* ************************************************************ *
 * leax_reg () - generates code to leax <value>,R               *
 *               where R != x                                   *
 * ************************************************************ */
void
#ifndef __STDC__
leax_reg (cref)
    register CMDREF *cref;
#else
leax_reg (CMDREF *cref)
#endif
{
    L7e9c (cref);

    if (cref->vartyp != C_REG_X)
    {
        /* The following generates:
         * leax cref->cmdval,R
         */
        gencode (117, C_REG_X, 119, cref);
        cref->vartyp = C_REG_X;
    }
}

void
#ifndef __STDC__
L6eac (cref)
    register CMDREF *cref;
#else
L6eac (CMDREF *cref)
#endif
{
    switch (cref->ft_Ty)
    {
        case FT_LONG:      /* L6eba */
            L34be (cref);
            break;
        case FT_FLOAT:     /* L6ec1 */
        case FT_DOUBLE:
            L3987 (cref);
            break;
        default:           /* L6ec8 */
            L6f83 (cref);
            L6eec (cref);
    }
}

static void
#ifndef __STDC__
L6eec (cref)
    register CMDREF *cref;
#else
L6eec ( CMDREF *cref)
#endif
{
    switch (cref->vartyp)
    {
        case C_DQUOT:      /* L6efb */
            gencode (117, C_REG_X, 119, cref);
            cref->vartyp = C_REG_X;
            cref->cmdval = 0;
            break;
        case C_AMPERSAND:    /* L6f1d */
            gencode (127, C_REG_X, 119, cref->cr_Left);
            cref->cmdval = 0;
            cref->vartyp = C_REG_X;
            break;
        case C_REG_X:
        case C_REG_D:
        case C_REG_U:
        case C_REG_Y:
            break;
        default:           /* L6f3e */
            /* gen "tfr |cref->vartyp|,d\n addd ..." */
            gencode (117, C_REG_D, 119, cref);
#ifdef COCO
vtytoRG_D:
#endif
            cref->vartyp = C_REG_D;
            break;
    }
}

CMDREF *
#ifndef __STDC__
L6f83 (cref)
    register CMDREF *cref;
#else
L6f83 (CMDREF *cref)
#endif
{
    int _vartype;

    /* if it's a series of entries create a tree */

    if ((_vartype = cref->vartyp) == C_COMMA)       /* else L6fcc */
    {
        CMDREF *_vrRight = cref->cr_Right;

        CCREFtoLeft (L6f83 (cref->cr_Left));    /* recurse into self */
        CmdrefCpy (_vrRight, cref);             /* copy _vrRight -> cref */
        mk_lftcmd (_vrRight);
        L6f83 (cref);                   /* go to L7176 */
    }
    else
    {
        if (cref->ft_Ty == FT_LONG)
        {
            L34d9 (cref);       /* go to L7176 */
        }
        else
        {
            if ((cref->ft_Ty == FT_FLOAT) || (cref->ft_Ty == FT_DOUBLE))
            {
                L39a2 (cref);       /* go to L7176 */
            }
            else
            {
                if (numeric_op (_vartype))
                {
                    L717c (_vartype, cref); /* go to L7176 */
                }
                else
                {               /* L7011 */
                    switch (_vartype)
                    {
                        case C_DQUOT:      /* L7176 */
                        case C_USRLBL:
                        case C_REG_Y:
                        case C_REG_U:
                        case C_AMPERSAND:
                        case C_INT:
                            break;
                        case C_EQUAL:      /* L7016 */
                            L77e6 (cref);
                            break;
                        case C_CHR2INT:    /* L701e */
                            L6f83 (cref->cr_Left);
                            break;
                        case C_LNG2INT:    /* L7028 */
                            L34d9 (cref->cr_Left);
                            /* Cast  parm 4 to satify prototype */
#ifdef COCO
                            gencode (C_LNG2INT, 119, (int)(cref->cr_Left));
#else
                            gencode (C_LNG2INT, 119, (int)(cref->cr_Left), 0);
#endif
                            cref->vartyp = C_REG_D;
                            break;
                        case C_DBL2INT:    /* L7046 */
                            L3987 (cref->cr_Left);
#ifdef COCO
                            gencode (C_TYFLOAT, C_DBL2INT);
#else
                            gencode (C_TYFLOAT, C_DBL2INT, 0, 0);
#endif
                            cref->vartyp = C_REG_D;
                            break;
                        case C_ASTERISK:   /* L705e */
                            do_asterisk (cref);
                            break;
                        case C_EXCLAM:     /* L7065 */
                        case C_ANDAND:
                        case C_OROR:
                            L74cb (cref);
                            break;
                        case C_TILDE:      /* L706c */
                        case C_MINUS:
                            reg2d_add (cref->cr_Left);
#ifdef COCO
                            gencode (_vartype);
#else
                            gencode (_vartype, 0, 0,0);
#endif
                            cref->vartyp = C_REG_D;
                            break;
                        case C_QUESTION:    /* L7080 */
                            /* warning from gcc - but this will work.
                             * L7567 will provide cref as a parameter
                             * for reg2d_add when it calls it
                             */
                            L7567 (cref, reg2d_add);
                            cref->vartyp = C_REG_D;
                            break;
                        case C_PLUSPLUS:    /* L7095 */
                        case C_INCREMENT:
                        case C_MINMINUS:
                        case C_DECREMENT:
                            L7d19 (cref, C_REG_D);
                            break;
                        case C_PARENS:     /* L70a1 */
                            L75e3 (cref);
                            break;
                        default:           /* L70ab */
                            if (_vartype >= C_PLUSEQ)   /* 160 */
                            {
                                L794a ( cref, _vartype);
                            }
                            else
                            {
                                comperr (cref, "translation");
                                printf ("%x\n", _vartype);
                            }

                            break;
                    }
                }
            }
        }
    }

    return cref;        /* L7176 */
}

static void
#ifndef __STDC__
L717c (vtyp, cref)
    int vtyp;
    CMDREF *cref;
#else
L717c (int vtyp, CMDREF *cref)
#endif
{
    CMDREF *_rfRt;  /* v4 */
#ifdef COCO
    int v2;     /* unused - may delete when debugging is complete */
#endif
    unsigned int var0;

    register CMDREF *rfLft;
    
    rfLft = cref->cr_Left;
    _rfRt = cref->cr_Right;

    /* First modify vtyp if unsigned */

    if (rfLft->ft_Ty == FT_UNSIGNED)
    {
        switch (vtyp)
        {
            case C_SLASH:      /* L719c */
                vtyp = C_UDIV;
                break;
            case C_PERCENT:    /* L71a1 */
                vtyp = C_UMOD;
                break;
            case C_RSHIFT:     /* L71a6 */
                vtyp = C_URSHFT;
                break;
            default:           /* L71f2 */
                break;
        }
    }
    else
    {
        switch (vtyp)
        {
            case C_PLUS:       /* L71c2 */
            case C_NEG:
                if ((rfLft->vartyp == C_AMPERSAND) && 
                            (_rfRt->vartyp != C_INT))
                {
                    L7e9c (cref);
                    D0066 = 0;
                    return;
                }

                break;
        }
    }

    switch (vtyp)
    {                   /* L748c = "break" */
        case C_NEG:        /* L71f7 */
            if ( ! int_usrlbl (_rfRt) )    /* else L7229 */
            {
                L6eac (_rfRt);
#ifdef COCO
                gencode (122, _rfRt->vartyp);
#else
                gencode (122, _rfRt->vartyp, 0, 0);
#endif
                _rfRt->vartyp = C_RgStk;
                reg2d_add (rfLft);
            }
            else
            {
                reg2d_add (rfLft);      /* L7229 */
                L6f83 (_rfRt);
            }

            gencode (C_NEG, C_REG_D, 119, _rfRt);
            break;
        case C_AND:
        case C_VBAR:
        case C_CARET:        /* L724d */
        case C_PLUS:
            if ( ((int_usrlbl (rfLft)) && ! (int_usrlbl (_rfRt))) ||
                    (rfLft->vartyp == C_INT))
            {
                CMDREF *__tmpref = rfLft;

                rfLft = _rfRt;
                _rfRt = __tmpref;
            }

            if (isRegU_Y (rfLft->vartyp))      /* L7275 */   /* else L72a0 */
            {
#ifdef COCO
                gencode (122, rfLft->vartyp);     /* L7282 */
#else
                gencode (122, rfLft->vartyp, 0, 0);
#endif
                reg2d_add ( _rfRt);
                _rfRt->vartyp = C_RgStk;        /* go to L72d1 */
            }
            else
            {
                reg2d_add (rfLft);      /* L72a0 */

                if ( ! int_usrlbl (_rfRt))
                {
#ifdef COCO
                    gencode (122, C_REG_D);
#else
                    gencode (122, C_REG_D, 0, 0);
#endif
                    reg2d_add ( _rfRt);
                    _rfRt->vartyp = C_RgStk;        /* go to 1052 */
                }
                else
                {
                    L6f83 (_rfRt);      /* L72b7 */

                    if (vtyp != C_PLUS)
                    {
                        L77ac ( _rfRt);
                    }
                }
            }

            gencode (vtyp, C_REG_D, 119, _rfRt);
            break;
        case C_EQEQ:       /* L72ec */
        case C_NOTEQ:
        case C_GT:
        case C_LT:
        case C_LT_EQ:
        case C_GT_EQ:
        case C_U_GT:
        case C_U_LT:
        case C_U_LTEQ:
        case C_U_GTEQ:
            L74cb (cref);
            break;
        case C_URSHFT:           /* L72f6 */
        case C_RSHIFT:
        case C_LSHIFT:
            if ((_rfRt->vartyp) == C_INT)      /* else L735a */
            {
                var0 = _rfRt->cmdval;
L730a:
                if (var0 <= FT_STRUCT)  /* 4 */     /* else L735a */
                {
                    reg2d_add ( rfLft);

                    while (var0--)
                    {
                        switch (vtyp)
                        {
                            case C_LSHIFT:      /* L7321 */
#ifdef COCO
                                gencode (152);
#else
                                gencode (152, 0, 0, 0);
#endif
                                break;
                            case C_RSHIFT:      /* L7326 */
#ifdef COCO
                                gencode (150);
#else
                                gencode (150, 0, 0, 0);
#endif
                                break;
                            case C_URSHFT:            /* L732b */
#ifdef COCO
                                gencode (151);
#else
                                gencode (151, 0, 0, 0);
#endif
                                break;
                        }
                    }       /* end while (var0--)  */ 

                    D0066 = 1;      /* D0066 _may_ be IsStruct, or the like */
                    goto L7492;
                }
                /*goto L7492;*/
            }

            goto L73a8;
        case C_MULT:       /* L735e */
            if (rfLft->vartyp == C_INT)    /* else L7372 */
            {
                CMDREF *_tmpref;

                /* Swap rfLft for rfRt */

                _tmpref = rfLft;
                rfLft = _rfRt;
                _rfRt = _tmpref;
            }       /* fall through to next case */
        case C_UDIV:           /* L7372 */
            if ((_rfRt->vartyp == C_INT) &&
                        (var0 = shftcount ( _rfRt->cmdval)))    /* else L73aa */
            {
                _rfRt->cmdval = var0;

                vtyp = (vtyp == C_MULT) ? C_LSHIFT : C_URSHFT;    /* L73a1 */
                goto L730a;
            }
        case C_SLASH:      /* L73aa */
        case C_UMOD:
        case C_PERCENT:
L73a8:
            L6eac (rfLft);
#ifdef COCO
            gencode (122, rfLft->vartyp);
#else
            gencode (122, rfLft->vartyp, 0, 0);
#endif
            reg2d_add ( _rfRt);
#ifdef COCO
            gencode (vtyp);
#else
            gencode (vtyp, 0, 0, 0);
#endif
            break;
        default:           /* L73d4 */
            comperr (cref, "binary op.");
            break;
    }   /* end switch vtyp */

    /* Following jump-label is not needed - vestige from the old source */
#ifdef COCO
L748c:
#endif
    D0066 = 0;
L7492:
    cref->vartyp = C_REG_D;
}

/* **************************************************************** *
 * shftcount () - Parses the table BitMsks for a match between      *
 *      parameter valu and an value in the table.                   *
 * Passed:  The value to compare                                    *
 * Returns: If matched, the offset into the table, which represents *
 *              the bit position of that value.                     *
 *          0 if no match found.  I.E. operation cannot be handled  *
 *              with a shift                                        *
 * **************************************************************** */

int
#ifndef __STDC__
shftcount (valu)
    int valu;
#else
shftcount (int valu)
#endif
{
    register int _count = 0;

    /* NOTE: Probably BitMsks should be defined in this file and
     * 14 should be changed to sizeof (BitMsks)/sizeof (BitMsks[0])
     */
    while (_count < 14)
    {
        if ( BitMsks[_count++] == valu)
        {
            return _count;
        }
    }

    return 0;
}

static void
#ifndef __STDC__
L74cb (cref)
    register CMDREF *cref;
#else
L74cb (CMDREF *cref)
#endif
{
    int _lbnum_2;
    int _lbnum_1;

    _lbnum_1 = ++LblNum;
    L939d (cref, (_lbnum_2 = ++LblNum), _lbnum_1, 1);
    loclbl_CR (_lbnum_2);
    gencode (117, C_REG_D, C_INT, (CMDREF *)1);  /* fake it for parm 4 */

#ifdef COCO
    gencode (124, (_lbnum_2 = ++LblNum), 1);
#else
    gencode (124, (_lbnum_2 = ++LblNum), 1, 0);
#endif
    loclbl_CR (_lbnum_1);

    gencode (117, C_REG_D, C_INT, 0);
    loclbl_CR (_lbnum_2);
    cref->vartyp = C_REG_D;
}

void
#ifndef __STDC__
L7567 (cref, fnc)
    register CMDREF *cref;
    void (*fnc)();
#else
L7567 (CMDREF *cref, void (*fnc)())
#endif
{
    int secnd_lbl;
    int thrd_lbl;
    int frst_lbl;

    /* This gives code in a bit different order than original, as
     * the following line was included in parameter push of the
     * call to L939d(), but gcc gave warning.  Some compilers
     * might not do the "right thing", so we'll do it this way
     */
    frst_lbl = ++LblNum;
    L939d (cref->cr_Left, (secnd_lbl = ++LblNum), frst_lbl, 1);
    cref = cref->cr_Right;
    loclbl_CR (secnd_lbl);
    (*fnc) (cref->cr_Left);
#ifdef COCO
    gencode (124, (thrd_lbl = ++LblNum), 0);
#else
    gencode (124, (thrd_lbl = ++LblNum), 0, 0);
#endif
    loclbl_CR (frst_lbl);
    (*fnc) (cref->cr_Right);
    loclbl_CR (thrd_lbl);
}

void
#ifndef __STDC__
L75e3 (cref)
    CMDREF *cref;
#else
L75e3 (CMDREF *cref)
#endif
{
    CMDREF *left_cref;
    int loclstk;

    loclstk = StkUsed;
    left_cref = cref;

    while ((left_cref = left_cref->cr_Right))
    {
        register CMDREF *right_cref = left_cref->cr_Left;    /* L75f8 */;

        if (right_cref->ft_Ty == FT_LONG)     /* else L7627 */
        {
            if (right_cref->vartyp == C_LONG)
            {
                pushslong (right_cref);  /* go to L7680 */
                continue;
            }
            else
            {
                L34be (right_cref);
#ifdef COCO
                gencode (C_TYLONG, C_RgStk);
#else
                gencode (C_TYLONG, C_RgStk, 0, 0);
#endif
                continue;
            }
        }
        else
        {       /* L7627 */
            if ( (right_cref->ft_Ty == FT_FLOAT)  ||
                 (right_cref->ft_Ty == FT_DOUBLE)   )
            {
                L3987 (right_cref);
#ifdef COCO
                gencode (C_TYFLOAT, C_RgStk);
#else
                gencode (C_TYFLOAT, C_RgStk, 0, 0);
#endif
            }
            else
            {
                L6f83 (right_cref);

                switch (right_cref->vartyp)
                {
                    case C_REG_D:
                    case C_REG_X:
                    case C_REG_U:
                    case C_REG_Y:
                        break;
                    default:
                        L6eec (right_cref);
                        break;
                }

                gencode (122, right_cref->vartyp NUL2);
            }

        }
    }       /* end while (left_cref = cr->right)  ( L7680 ) */

    L6f83 (cref->cr_Left);
#ifdef COCO
    gencode (C_PARENS, 119, cref->cr_Left);
#else
    /* cast parm3 to satify prototype */
    gencode (C_PARENS, 119, (int)(cref->cr_Left), 0);
#endif
    StkUsed = prt_rsrvstk (loclstk);
    cref->vartyp = C_REG_D;
}


int 
#ifndef __STDC__
L76bf (cref)
    register CMDREF *cref;
#else
L76bf (CMDREF *cref)
#endif
{
    CMDREF *vty_left;

    switch (cref->vartyp)
    {
        case C_USRLBL:     /* L76d9 */
        case C_INT:
            return 1;
        case C_ASTERISK:   /* L76cf */
            switch ((vty_left = cref->cr_Left)->vartyp)
            {
                case C_INT:   /* L76d9 */
                case C_USRLBL:
                case C_REG_Y:
                case C_REG_U:
                    return 1;
                default:           /* L76de */
                    return L771d (vty_left);
            }
    }

    return 0;
}

static int 
#ifndef __STDC__
L771d (cref)
    register CMDREF *cref;
#else
L771d (CMDREF *cref)
#endif
{
    switch (cref->vartyp)
    {
        case C_PLUS:       /* L772b */
        case C_NEG:
            if ( (isRegU_Y ((cref->cr_Left)->vartyp))  &&
                 ((cref->cr_Right)->vartyp == C_INT))
            {
                return 1;
            }

    }

    return 0;
}

/* **************************************************************** *
 * crf_isint () - Returns TRUE if cref->__cr18 is an int or ?       *
 * **************************************************************** */

int 
#ifndef __STDC__
crf_isint (cref)
    CMDREF *cref;
#else
crf_isint (CMDREF *cref)
#endif
{
    return (cref->__cr18 < 2);
}

static int 
#ifndef __STDC__
is_ref (cref)
    register CMDREF *cref;
#else
is_ref (CMDREF *cref)
#endif
{
    switch (cref->vartyp)
    {
        case C_PLUS:       /* L777f */
        case C_NEG:
            return ((cref->cr_Left)->vartyp == C_AMPERSAND);
        case C_DQUOT:      /* L7789 */
        case C_AMPERSAND:
            return 1;
    }

    return 0;
}

/* ******************************************************************** *
 * L77ac () - NOTE: runs only if cref->vartyp & 0x8000                  *
 *      Adds value stored in cref->cmdval to value contained in REG X   *
 * If run:                                                              *
 * Sets     cref->vartyp = C_isRgX                                      *
 *          cref->cmdval = 0                                            *
 * ******************************************************************** */

void 
#ifndef __STDC__
L77ac (cref)
    register CMDREF *cref;
#else
L77ac (CMDREF *cref)
#endif
{
    if (cref->vartyp & 0x8000)
    {
        cref->vartyp &= 0x7fff;
        /* following generates "leax <cref->cmdval>,x"
         *      if cref->vartyp != C_CHR2INT
         * otherwise:
         *  sex
         *  tfr x,d
         *  addd #<cref->cmdval>
         */
        gencode (117, C_REG_X, 119, cref);
        cref->vartyp = C_isRgX;
        cref->cmdval = 0;
    }
}

static void 
#ifndef __STDC__
L77e6 (cref)
    CMDREF *cref;
#else
L77e6 (CMDREF *cref)
#endif
{
#ifdef COCO
    int var2;   /* unused - may delete when debugging is complete */
#endif
    CMDREF *rightrf;

    register CMDREF *leftrf;

    rightrf = cref->cr_Right;
    leftrf = cref->cr_Left;

    if (isRegU_Y (leftrf->vartyp))        /* else L787c */
    {
        if (L771d (rightrf))
        {
            L7e9c (rightrf);
        }
        else
        {
            L6f83 (rightrf);
        }

        switch (rightrf->vartyp)
        {
            case C_AMPERSAND:  /* L782a */
                gencode (127, leftrf->vartyp, 119, rightrf->cr_Left);
                break;
            case C_CHR2INT:    /* L783e */
                gencode (117, C_REG_D, 119, rightrf);
                /* fall through to default */
            default:           /* L7856 */
                gencode (117, leftrf->vartyp, 119, rightrf);
                break;
        }

        cref->vartyp = leftrf->vartyp;    /* L7a13 */
        cref->cmdval = 0;
        return;
    }

    /* L787c */
    if ( ((L76bf (leftrf)) && (leftrf->ft_Ty != FT_CHAR)) &&
         ((L771d (rightrf)) || ( is_ref (rightrf)))          )
    {                       /* else L78b4 */
        L6f83 (leftrf);     /* L78a3 */
        L7e63 (rightrf);    /* go to L792a */
    }
    else
    {
        /* L78b4 */
        if (isRegU_Y (rightrf->vartyp))   /* else L78db */
        {
            L6f83 (leftrf);

            if (leftrf->ft_Ty == FT_CHAR)       /* else L792c */
            {
                reg2d_add (rightrf);       /* go to L792c (L792a) */
            }
        }
        else
        {
            if (crf_isint (leftrf))     /* L78db */
            {
                reg2d_add (rightrf);
                L6f83 (leftrf);     /* go to L792a */
            }
            else
            {
                L6f83 (leftrf);     /* L78f4 */

                if ( ! L76bf (rightrf))    /* else L7923 */
                {
                    switch (leftrf->vartyp & 0x7fff)
                    {
                        case C_isRgU:       /* L7923 */
                        case C_isRgY:
                            break;
                        default:        /* L790e */
                            L7b83 (leftrf); /* RegX + leftrf->cmdval, pshs x */
                            break;
                    }
                }

                L6eac (rightrf);       /* L7923 - I didn't have this in before */
            }
        }

    }

    gencode (121, rightrf->vartyp, 119, leftrf);     /* L792c */
    cref->vartyp = rightrf->vartyp;                /* L7a15 */
    cref->cmdval = 0;
    return;         /* L7d15 */
}
/* FIXME - the loop above may not be right.. some loops may end with L792c
 *          I think it's fixed */

/* ******************************************************************** *
 * L794a () - Handles operations where a variable is modified by some   *
 *            math operation, such as "+=", "%=" or any of the other    *
 *            ops.
 * ******************************************************************** */

static void
#ifndef __STDC__
L794a (cref, vartype)
    CMDREF *cref;
    int vartype;
#else
L794a (CMDREF *cref, int vartype)
#endif
{
    CMDREF *rfRight;
    int var0;
    register CMDREF *rfLeft;

    rfRight = cref->cr_Right;
    rfLeft = cref->cr_Left;
    L6f83 (rfLeft);
    vartype -= 80;      /* rest to simple math function */

    if (rfLeft->ft_Ty == FT_UNSIGNED)   /* else L7998 */
    {
        switch (vartype)
        {
            case C_SLASH:      /* L7978 */
                vartype = C_UDIV;
                break;
            case C_RSHIFT:     /* L797d */
                vartype = C_URSHFT;
                break;
            case C_PERCENT:    /* L7982 */
                vartype = C_UMOD;
                break;
        }
    }

    if (isRegU_Y (var0 = (rfLeft->vartyp & 0x7fff)))       /* else L7a6e */
    {
        switch (vartype)
        {
            case C_PLUS:       /* L79ae */
            case C_NEG:
                if (rfRight->vartyp == C_INT)     /* else L79e5 */
                {
                            /* cast parm 4 to satify prototype */
                    gencode (C_LEA_RG, var0, C_INT,
                            (CMDREF *)((vartype == C_PLUS) ? rfRight->cmdval :
                                                            -(rfRight->cmdval)));
                }
                else
                {
                    reg2d_add (rfRight);

                    if (vartype == C_NEG)
                    {
#ifdef COCO
                        gencode (C_MINUS);
#else
                        gencode (C_MINUS, 0, 0, 0);
#endif
                    }

#ifdef COCO
                    gencode (C_LEA_RG, var0, C_REG_D);
#else
                    gencode (C_LEA_RG, var0, C_REG_D, 0);
#endif
                }

                cref->vartyp = rfLeft->vartyp;  /* L7a13 */
                cref->cmdval = 0;
                return;
            case C_AND:        /* L7a22 */
            case C_VBAR:
            case C_CARET:
                goto L7a6c;
            default:           /* L7a26 */
                gencode (122, rfLeft->vartyp NUL2);
                reg2d_add (rfRight);
                gencode (vartype NUL3);
                break;  /* go to L7b63 */
        }
    }
    else
    {
L7a6c:
        gencode (117, C_REG_D, 119, rfLeft);
        
        if (rfLeft->ft_Ty == FT_CHAR)
        {
            gencode (C_CHR2INT NUL3);
        }

        switch (vartype)
        {
            case C_AND:        /* L7a9b */
            case C_VBAR:
            case C_CARET:
            case C_PLUS:
            case C_NEG:
                if (L76bf (rfRight))   /* else L7aec (default) */
                {
                    L6f83 (rfRight);

                    switch (vartype)
                    {
                        case C_AND:        /* L7ab3 */
                        case C_VBAR:
                        case C_CARET:
                            L77ac (rfRight);
                            break;
                    }

                    gencode (vartype, C_REG_D, 119, rfRight);
                    break;
                }
                /* Fall through to default */
            default:           /* L7aec */
                if ((var0 != C_isRgY) && (var0 != C_isRgU))
                {
                    L7b83 (rfLeft);
                }

                gencode (122, C_REG_D NUL2);
                reg2d_add (rfRight);

                if (vartype == C_NEG)
                {
                    vartype = 79;
                }

                gencode (vartype, C_REG_D, C_RgStk NUL1);
                break;
        }           /* end switch */
    }           /* end else = (rfLeft->vartyp & 0x7fff) == 0  */

    /* L7b63 */
    gencode (121, C_REG_D, 119, rfLeft);
    cref->vartyp = C_REG_D;
}

/* **************************************************************** *
 * L7b83 () - Adds value stored in cref->cmdval to value in RegX    *
 *      then pushes RegX onto stack                                 *
 *      for variables NOT C_USRLBL                                  *
 * **************************************************************** */

static void
#ifndef __STDC__
L7b83 (cref)
    register CMDREF *cref;
#else
L7b83 (CMDREF *cref)
#endif
{
    if ((cref->vartyp & 0x7fff) != C_USRLBL)
    {
        L77ac (cref);   /* RegX + cref->cmdval if cref->vartyp & 0x8000 */

        if (cref->cmdval)   /* If L77ac () ran */
        {
            /* following gencode generates
             *  leax <cref->cmdval>,x
             */
            /* cast parm 4 to satisfy prototype */
            gencode (C_LEA_RG, C_REG_X, C_INT, (CMDREF *)(cref->cmdval));
        }

        gencode (122, C_REG_X NUL2);  /* "pshs x" */
        cref->vartyp =  C_RgStk | 0x8000; /* -32658*/
    }
}

/* ******************************************************************** *
 * do_asterisk () Handles a pointer reference                           *
 * Passed: The CMDREF to be processed                                   *
 * ******************************************************************** */

void
#ifndef __STDC__
do_asterisk (cref)
    register CMDREF *cref;
#else
do_asterisk (CMDREF *cref)
#endif
{
    CMDREF *leftCREF;
    int newvartyp;

    L7e9c (leftCREF = cref->cr_Left);

    if ((newvartyp = leftCREF->vartyp) & 0x8000)     /* else L7c56 */
    {
        switch (newvartyp &= 0x7fff)
        {
            case C_USRLBL:     /* L7c02 */
            case C_isRgX:
            case C_isRgY:
            case C_isRgU:
                /* generate "ldx ... */
                gencode (117, C_REG_X, newvartyp, /* cast parm 4 to satisfy */
                       (CMDREF *)(leftCREF->cmdval)); /* prototype */
                break;
            default:           /* L7c1d */
                comperr (leftCREF, "indirection");
                break;
        }

        newvartyp = C_isRgX | 0x8000; /* -32621 */       /* L7c4a */
        cref->cmdval = 0;
        /* go to L7d11 */
    }
    else    /* else ! newvartyp & 0x8000 */
    {                /* L7c56 */
        switch (newvartyp)
        {
            case C_REG_U:      /* L7c5b */
                newvartyp = C_isRgU;
                goto cp_cmdval;
            case C_REG_Y:    /* L7c60 */
                newvartyp = C_isRgY;
                goto cp_cmdval;
            case C_REG_X:          /* L7c65 */
                newvartyp = C_isRgX;
                goto cp_cmdval;
            case C_isRgX:          /* L7c6a */
            case C_isRgY:
            case C_isRgU:
                newvartyp |= 0x8000;
                goto cp_cmdval;
            case C_USRLBL:     /* L7c74 */
                newvartyp = C_USRLBL | 0x8000; /* -32716 */
                cref->ptrdstval = leftCREF->ptrdstval;
cp_cmdval:
                cref->cmdval = leftCREF->cmdval;
                break;
            case C_REG_D:          /* L7c8b */
                gencode (117, C_REG_X, 119, leftCREF);
                newvartyp = C_isRgX;
                cref->cmdval = 0;
                break;
            case C_INT:   /* L7cae */
                cref->cmdval = 0;
                cref->ptrdstval = leftCREF->cmdval;
                newvartyp = C_USRLBL;
                break;
            default:           /* L7cbe */
                comperr (cref, "indirection");
                newvartyp = C_isRgX;
                break;
        }
    }       /* end else i.e. leftCREF->vartyp) & 0x8000 == 0 */

    cref->vartyp = newvartyp;    /* L7d11 */
}

static void
#ifndef __STDC__
L7d19 (cref, val)
    CMDREF *cref;
    int val;
#else
L7d19 (CMDREF *cref, int val)
#endif
{
    int _cmdval;
#ifdef COCO
    int var4;   /* unused - may delete when debugging is complete */
#endif
    int cntr_vty;
    int _lft_vtyp;
    register CMDREF *_leftref;

    _leftref = cref->cr_Left;

    L6f83 (_leftref);
    cntr_vty = cref->vartyp;
    
    if (isRegU_Y (_lft_vtyp = _leftref->vartyp))        /* else L7d79 */
    {
        switch (cntr_vty)
        {
            case C_INCREMENT:
            case C_DECREMENT:
                if (val == C_REG_D)     /* else L7d65 */
                {
                    gencode (117, val, 119, _leftref);
                    break;
                }
            default:
                val = _lft_vtyp;
                break;      /* go to L7daa */
        }
    }
    else
    {
        if ((val == C_REG_X) && ((_lft_vtyp & 0x7fff) != C_USRLBL))     /* L7d79 */
        {
            val = C_REG_D;
        }
        
        gencode (117, val, 119, _leftref);
        _lft_vtyp = val;
    }

    _cmdval = cref->cmdval;        /* L7daa */

    switch (cntr_vty)
    {
        case C_MINMINUS:        /* L7db4 */
        case C_DECREMENT:
            _cmdval = -_cmdval;

        default:                /* L7dbc */
            if (_lft_vtyp == C_REG_D)
            {
                    /* cast parm 4 to satisfy prototype */
                gencode ( C_PLUS, _lft_vtyp, C_INT, (CMDREF *)_cmdval);
                break;
            }

            gencode (C_LEA_RG, _lft_vtyp, C_INT, (CMDREF *)_cmdval);
            break;
    }

    gencode (121, _lft_vtyp, 119, _leftref);

    switch (cntr_vty)
    {
        default:                /* L7e16 */
            cref->cmdval = 0;
            break;
        case C_INCREMENT:       /* L7e1a */
        case C_DECREMENT:
            if (_lft_vtyp == C_REG_D)
            {
                    /* cast parm 4 to satify prototype */
                gencode (C_NEG, C_REG_D, C_INT, (CMDREF *)_cmdval);
            }
            else
            {
                cref->cmdval = -_cmdval;
            }
            break;
    }

    cref->vartyp = val;
    D0066 = 0;
}

static void
#ifndef __STDC__
L7e63 (cref)
    register CMDREF *cref;
#else
L7e63 (CMDREF *cref)
#endif
{
    L7e9c (cref);

    if ((cref->vartyp != C_REG_X) || (cref->cmdval != 0))    /* else L7e95 */
    {
        gencode (117, C_REG_X, 119, cref);
    }

    cref->vartyp = C_REG_X;
}

static void
#ifndef __STDC__
L7e9c (cref)
    CMDREF *cref;
#else
L7e9c (CMDREF *cref)
#endif
{
    CMDREF *rightCREF;
    int val_right;
    int my_vtyp;
    int new_vtyp;

    register CMDREF *_crleft;

    _crleft = cref->cr_Left;
    rightCREF = cref->cr_Right;
    new_vtyp = C_REG_X;
    
    switch (my_vtyp = cref->vartyp)
    {
        case C_ASTERISK:   /* L7ec0 */
            do_asterisk (cref);
            return;
        case C_USRLBL:     /* L812e */
        case C_REG_Y:
        case C_REG_U:
        case C_INT:
        case C_DQUOT:
            return;
        case C_INCREMENT:  /* L7eca */
        case C_PLUSPLUS:
        case C_MINMINUS:
        case C_DECREMENT:
            L7d19 (cref, C_REG_X);
            return;
        case C_AMPERSAND:  /* L7edb */
            gencode (127, C_REG_X, 119, _crleft);
            cref->cmdval = 0;
            break;
        case C_CHR2INT:    /* L7ef6 */
            reg2d_add (cref);
            return;
        case C_NEG:        /* L7f00 */
            if ((_crleft->vartyp != C_AMPERSAND) && (rightCREF->__cr18))
            {                                   /* go to L7faf */
                goto L80c1;
            }
                /* Is this correct????  */
        case C_PLUS:       /* L7f11 */
            if (  (isRegU_Y (_crleft->vartyp))     ||
                  (_crleft->vartyp == C_AMPERSAND)   )          /* else L7f78 */
            {
                            /* L7f24 */
                if (rightCREF->vartyp == C_INT)   /* else L7f40 */
                {
                    L7e9c (_crleft);
                    new_vtyp = _crleft->vartyp;
                    val_right = rightCREF->cmdval;
                    
                    /* jump to L80a5 */

                    /*if (my_vtyp == C_PLUS)
                    {
                        _crleft->cmdval += val_right;
                    }
                    else
                    {
                        _crleft->cmdval -= val_right;
                    }

                    break;*/
                }
                else
                {
                    reg2d_add (rightCREF);       /* L7f40 */
                    L7e9c (_crleft);

                    if (my_vtyp == C_NEG)      /* else L7f62 */
                    {
#ifdef COCO
                        gencode (C_MINUS);
#else
                        gencode (C_MINUS, 0, 0, 0);
#endif
                    }

#ifdef COCO
                    gencode (C_REGOFST, C_REG_D, _crleft->vartyp);  /* L7f62 */
#else
                    gencode (C_REGOFST, C_REG_D, _crleft->vartyp, 0);
#endif
                    val_right = 0;
                }
            }
            else
            {                    /* L7f78 */
                if ( (my_vtyp == C_PLUS)                  &&
                     (_crleft->__cr18 < rightCREF->__cr18)  )
                {
                    CMDREF *_tmpref = _crleft;

                    /* Swap left for right */

                    _crleft = rightCREF;
                    rightCREF = _tmpref;
                }

                if ( ! L76bf (rightCREF))    /* L7f97 */     /* else L7fb4 */
                {
                    if ( ! isRegU_Y (rightCREF->vartyp))
                    {
                        goto L80c1;
                    }
                }

                L7e9c (_crleft);        /* L7fb4 */

                switch (_crleft->vartyp & 0x7fff)
                {
                    case C_REG_D:          /* L7fc4 */
                        if ((my_vtyp == C_PLUS) &&     /* else L7fe8 */
                                (rightCREF->vartyp != C_INT))
                        {
                            leax_reg (rightCREF);
                            _crleft->cmdval = 0;
                            goto L8089;
                        }


                    case C_USRLBL:
                    case C_DQUOT:
                    case C_isRgX:          /* L7fe8 */
                    case C_isRgY:
                    case C_isRgU:
                        gencode (117, C_REG_X, 119, _crleft);
                        _crleft->cmdval = 0;
                        break;
                    case C_REG_X:          /* L8054 */
                        break;
                    case C_REG_Y:    /* L8004 */
                    case C_REG_U:
                        new_vtyp = _crleft->vartyp;
                        break;
                    default:           /* L800a */
                        comperr ( _crleft, "x translate");
                        break;
                }

                if (rightCREF->vartyp == C_INT)     /* L8054 */
                {
                    val_right = rightCREF->cmdval;
                }
                else
                {
                    reg2d_add (rightCREF);
                    
                    if (my_vtyp == C_NEG)  /* else L808b */
                    {
#ifdef COCO
                        gencode (C_MINUS, 0, 0);
#else
                        gencode (C_MINUS, 0, 0, 0);
#endif
                    }

L8089:
#ifdef COCO
                    gencode (C_REGOFST, C_REG_D, new_vtyp);
#else
                    gencode (C_REGOFST, C_REG_D, new_vtyp, 0);
#endif
                    new_vtyp = C_REG_X;
                    val_right = 0;
                }
            }

                /* L80a5 */
            cref->cmdval = _crleft->cmdval +
                        ((my_vtyp == C_PLUS) ? val_right : -(val_right));
            
            break;
        default:           /* L80c3 */
L80c1:
            L6f83 (cref);
            return;
    }

    cref->vartyp = new_vtyp;    /* L8128 */
}

int
#ifndef __STDC__
isRegU_Y (vtyp)
    int vtyp;
#else
isRegU_Y (int vtyp)
#endif
{
    return ((vtyp == C_REG_U) || (vtyp == C_REG_Y));
}
