/* ************************************************************************ *
 * comp_06.c - part 6 for c.comp                                            *
 *                                                                          *
 * from last part of p2_03.c                                                *
 *                                                                          *
 * $Id:: comp_06.c 64 2008-09-23 00:46:01Z dlb                            $ *
 * ************************************************************************ */

#include "ccomp.h"

void
#ifndef __STDC__
L34be (cref)
    CMDREF *cref;
#else
L34be (CMDREF *cref)
#endif
{
    L34d9 (cref);
    L3898 (cref);
}

void
#ifndef __STDC__
L34d9 (cref)
    register CMDREF *cref;
#else
L34d9 (CMDREF *cref)
#endif
{
    CMDREF *var4;
    int _varty;
    int valptr;

    switch (_varty = cref->vartyp)
    {
        case C_FLACC:          /* L3894 */
        case C_isRgX:
        case C_isRgY:
        case C_isRgU:
        case C_USRLBL:
            break;      /* Do nothing for index regs or USRLBL */
        case C_ASTERISK:   /* L34ee */
            do_asterisk (cref);
            L77ac (cref);

            switch (cref->vartyp)
            {
                case C_isRgX:          /* L3500 */
                case C_isRgY:
                case C_isRgU:
                    if (cref->cmdval)
                    {
#ifdef COCO
                        gencode (C_REGOFST, 119, cref);
#else
                        /* cast cref to int to satisfy prototype */
                        gencode (C_REGOFST, 119, (int)cref, 0);
#endif
                        cref->vartyp = C_isRgX;
                        cref->cmdval = 0;
                    }

                    /* fall through to break */
                default:           /* L3894 */
                    break;
            }

            break;

        case C_U2LNG:      /* L3530 */
            reg2d_add (cref->cr_Left);
            gencode (C_TYLONG, C_U2LNG NUL2);
            goto vty2flacc;
        case C_DBL2LNG:    /* L353f */
            L3987 (cref->cr_Left);
#ifdef COCO
            gencode (C_TYFLOAT, C_DBL2LNG, cref->cr_Left);
#else
            /* cast cref->cr_Left to satisfy prototype */
            gencode (C_TYFLOAT, C_DBL2LNG, (int)(cref->cr_Left), 0);
#endif
            goto vty2flacc;
        case C_I2LNG:      /* L355e */
            reg2d_add (cref->cr_Left);
            gencode (C_TYLONG, C_I2LNG NUL2);
vty2flacc:
            cref->vartyp = C_FLACC;
            break;
        case C_LONG:       /* L3582 */
#ifdef COCO
            gencode (C_TYLONG, C_LONG, cref->cmdval);
#else
            gencode (C_TYLONG, C_LONG, cref->cmdval, 0);
#endif
            /*L_L3203 (cref->cmdval, 4);*/   /* was in c.pass1, not c.comp */
            goto L3771;
        case C_QUESTION:   /* L3597 */
            /* The following gives a warning from gcc, but it works.
             * L7567() passes the first parameter as a parameter for
             * the function, when called
             */
            L7567 (cref, L34be);
            goto L3771;
        case C_PLUSPLUS:   /* L35a4 */
        case C_MINMINUS:
        case C_TILDE:
        case C_MINUS:
            L34be (cref->cr_Left);
#ifdef COCO
            gencode (C_TYLONG, _varty);
#else
            gencode (C_TYLONG, _varty, 0, 0);
#endif
            goto L3771;
        case C_INCREMENT:  /* L35c0 */
        case C_DECREMENT:
            gencode (127, C_REG_X, 128, 0);
#ifdef COCO
            gencode (122, C_REG_X);
#else
            gencode (122, C_REG_X, 0, 0);
#endif
            L34be (cref->cr_Left);
#ifdef COCO
            gencode (C_TYLONG, _varty);
            gencode (C_TYLONG, C_BIGMOV);
#else
            gencode (C_TYLONG, _varty, 0, 0);
            gencode (C_TYLONG, C_BIGMOV, 0, 0);
#endif

            gencode ( C_TYLONG,
                      ((_varty == C_INCREMENT) ? C_DECREMENT : C_INCREMENT)
#ifndef COCO
                    , 0, 0
#endif
                  );
            goto vty2flacc;

        case C_PARENS:     /* L3621 */
            L75e3 (cref);
            goto vty2flacc;
        case C_MULT:       /* L362b */
            if (((cref->cr_Left)->vartyp) == C_LONG)
            {
                CMDREF *_tmpref;

                /* Swap cr_Left with cr_Right */

                _tmpref = cref->cr_Left;
                cref->cr_Left = cref->cr_Right;
                cref->cr_Right = _tmpref;
            }

            if (((cref->cr_Right)->vartyp) == C_LONG)       /* L3645 */
            {
#ifdef COCO
                /* use var4 as int[2] here */
                var4 = ((CMDREF *)(cref->cr_Right)->cmdval);

                /* If long value fits into COCO int, and we can multiply
                 * by shifting, convert to int */

                    /* L3696 */
                if ( (((int *)var4)[0] == 0)              &&
                     (valptr = shftcount (var4->varsize))   )
#else
                long _lval = ((long)(cref->cr_Right)->cmdval);
                
                if ((_lval <= 0xffff) && (valptr = shftcount (var4->varsize)))
#endif
                {
                    var4 = cref->cr_Right;
                    /* for non-coco systems, cmdval is already a direct long */
#ifdef COCO
                    var4->cmdval = valptr;
#endif
                    var4->vartyp = C_INT;
                    var4->ft_Ty = FT_INT;
                    _varty = (_varty == C_MULT) ? C_LSHIFT : C_RSHIFT;
                    goto L36d0;
                }
            }

            /* fall through to next case */
        case C_SLASH:      /* L3696 */
        case C_EQEQ:
        case C_NOTEQ:
        case C_GT_EQ:
        case C_LT_EQ:
        case C_GT:
        case C_LT:
        case C_PLUS:
        case C_NEG:
        case C_PERCENT:
        case C_AND:
        case C_VBAR:
        case C_CARET:
            var4 = cref->cr_Left;

            if (var4->vartyp == C_LONG)
            {
                pushslong (var4);
            }
            else
            {
                L34be (var4);
#ifdef COCO
                gencode (C_TYLONG, C_RgStk);
#else
                gencode (C_TYLONG, C_RgStk, 0, 0);
#endif
            }

            L34be (cref->cr_Right);
#ifdef COCO
            gencode (C_TYLONG, _varty);
#else
            gencode (C_TYLONG, _varty, 0, 0);
#endif
            goto vty2flacc;
        case C_LSHIFT:     /* L36d2 */
        case C_RSHIFT:
L36d0:
            L34be (cref->cr_Left);
#ifdef COCO
            gencode (122, C_REG_X);
#else
            gencode (122, C_REG_X, 0, 0);
#endif
            reg2d_add ( cref->cr_Right);
#ifdef COCO
            gencode (C_TYLONG, _varty);
#else
            gencode (C_TYLONG, _varty, 0, 0);
#endif
            goto vty2flacc;
        case C_EQUAL:      /* L3706 */
            L34be (cref->cr_Left);
#ifdef COCO
            gencode (122, C_REG_X);
#else
            gencode (122, C_REG_X, 0, 0);
#endif
            L34be (cref->cr_Right);
            goto L375e;

            /* We'll put this here -- don't know at the moment why it was
             * put here, will investigate l8tr
             */
L372b:
            L34be (var4 = cref->cr_Left);
#ifdef COCO
            gencode (122, C_REG_X);
#else
            gencode (122, C_REG_X, 0, 0);
#endif
            cref->vartyp = _varty - 80;
            var4->vartyp = C_isRgX;
            L34d9 (cref);
L375e:
#ifdef COCO
            gencode (C_TYLONG, C_BIGMOV);
#else
            gencode (C_TYLONG, C_BIGMOV, 0, 0);
#endif
L3771:
            cref->vartyp = C_isRgX;
            cref->cmdval = 0;
            break;
        default:           /* L377f */
            if (_varty >= C_PLUSEQ)
            {
                goto L372b;
            }

            comperr (cref, "longs");
            break;
    }
}

void
#ifndef __STDC__
L3898 (cref)
    CMDREF *cref;
#else
L3898 (CMDREF *cref)
#endif
{
    switch (cref->vartyp)
    {
        case C_USRLBL:      /* L38a6 */
            gencode (127, C_REG_X, 119, cref);
            break;
        case C_isRgY:           /* L38c0 */
        case C_isRgU:
#ifdef COCO
            gencode (C_REGOFST, 119, cref);
#else
            /* cast cref to int to satisfy prototype */
            gencode (C_REGOFST, 119, (int)cref, 0);
#endif
            break;
    }
}

/* **************************************************************** *
 * pushslong () - Push the 4-byte value of a long onto the stack    *
 * **************************************************************** */

/* NOTE: Due to the abundance of gencode () calls, we'll provide separate
 * function source for COCO and non-COCO
 */

#ifdef COCO
void
pushslong (cref)
    register CMDREF *cref;
{
    /* Declare a union to eliminate so much casting */

    union {
        long *l;
        int *i;
    } _lng;
    
    _lng.l = (long *)(cref->cmdval);

    if (*(_lng.l))
    {           /* else L3947 */
        gencode (117, C_REG_D, C_INT, _lng.i[1]);
        gencode (122, C_REG_D);
        gencode (117, C_REG_D, C_INT, _lng.i[0]);    /* go to L396e */
    }
    else
    {
        gencode (117, C_REG_D, C_INT, 0);
        gencode (122, C_REG_D);
    }

    gencode (122, C_REG_D);      /* L396e */
    /*L_L3203 (_lng.l, 4);   Not in c.comp
    cref->cmdval = 0;*/
}
/* *************** divide ************************* */
#else

void
#ifdef __STDC__
pushslong (CMDREF *cref)
#else
pushslong (cref)
    CMDREF *cref;
#endif
{
    long _lngval = (long)(cref->cmdval);

    if (_lngval)
    {           /* else L3947 */
        /* Cast __lngval values to satisfy prototype */
        gencode (117, C_REG_D, C_INT, (CMDREF *)(_lngval & 0xffff));
        gencode (122, C_REG_D, 0, 0);
        gencode (117, C_REG_D, C_INT, (CMDREF *)(_lngval >> 16));
                /* go to L396e */
    }
    else
    {
        gencode (117, C_REG_D, C_INT, 0);
        gencode (122, C_REG_D, 0, 0);
    }

    gencode (122, C_REG_D, 0, 0);
}
#endif
