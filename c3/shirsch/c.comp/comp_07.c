/* ************************************************************************ *
 * comp_07.c - part 7 for c.com                                             *
 *                                                                          *
 * comes from p2_04.c                                                       *
 * $Id:: comp_07.c 64 2008-09-23 00:46:01Z dlb                            $ *
 * ************************************************************************ */

#include "ccomp.h"

void
#ifndef __STDC__
L3987 (cref)
    CMDREF *cref;
#else
L3987 (CMDREF *cref)
#endif
{
    L39a2 (cref);
    L3898 (cref);
}

void
#ifndef __STDC__
L39a2 (cref)
    register CMDREF *cref;
#else
L39a2 (CMDREF *cref)
#endif
{
    int _vrtyp;
    CMDREF *var4;
    int _ftyp;
#ifdef COCO
    int var0;   /* Not used. keep in COCO for matching code */
#endif

    _ftyp = cref->ft_Ty;

    switch (_vrtyp = cref->vartyp)
    {
        case C_FLACC:          /* L3cac */
        case C_isRgX:
        case C_isRgY:
        case C_isRgU:
        case C_USRLBL:
            break;
        case C_ASTERISK:   /* L39bb */
            L34d9 (cref);
            break;
        case C_U2DBL:      /* L39c5 */
        case C_I2DBL:
            reg2d_add (cref->cr_Left);
L39d0:
#ifdef COCO
            gencode (C_TYFLOAT, _vrtyp);
#else
            gencode (C_TYFLOAT, _vrtyp,0 ,0);
#endif
vty2flacc:
            cref->vartyp = C_FLACC;
            break;
        case C_L2DBL:      /* L39ec */
            L34be (cref->cr_Left);
            goto L39d0;
        case C_TOFLOAT:    /* L39f5 */
        case C_FLT2DBL:
            L3987 (cref->cr_Left);
            goto L39d0;
        case C_DOUBLE:     /* L3a02 */
#ifdef COCO
            gencode (C_TYFLOAT, C_DOUBLE, cref->cmdval);
#else
            gencode (C_TYFLOAT, C_DOUBLE, cref->cmdval, 0);
#endif
            cref->vartyp = C_isRgX;
            /*L_L3203 (cref->cmdval, DBLSIZ);    Not used in c.comp */
            cref->cmdval = 0;
            break;
        case C_QUESTION:   /* L3a18 */
            /* The following gives a warning, but it's correct for the
             * application.  L7567 () passes the first parameter to
             * the function as its parameter, also
             */
            L7567 (cref, L3987);
            goto L3bac;
        case C_PLUSPLUS:            /* L3a27 */
        case C_MINMINUS:
        case C_MINUS:
            L3987 (cref->cr_Left);
#ifdef COCO
            gencode (C_TYFLOAT, _vrtyp, _ftyp);
#else
            gencode (C_TYFLOAT, _vrtyp, _ftyp, 0);
#endif
            goto L3bac;
        case C_INCREMENT:
        case C_DECREMENT:           /* L3a47 */
#ifdef COCO
            gencode (127, C_REG_X, C_FLACC);
            gencode (122, C_REG_X);
#else
            gencode (127, C_REG_X, C_FLACC, 0);
            gencode (122, C_REG_X, 0, 0);
#endif
            L3987 (cref->cr_Left);
#ifdef COCO
            gencode (C_TYFLOAT, _vrtyp, _ftyp);
            gencode (C_TYFLOAT, C_BIGMOV, _ftyp);
            gencode ( C_TYFLOAT,
                      (_vrtyp == C_INCREMENT ? C_DECREMENT : C_INCREMENT),
                      _ftyp
                    );
#else
            gencode (C_TYFLOAT, _vrtyp, _ftyp, 0);
            gencode (C_TYFLOAT, C_BIGMOV, _ftyp, 0);
            gencode ( C_TYFLOAT,
                      (_vrtyp == C_INCREMENT ? C_DECREMENT : C_INCREMENT),
                      _ftyp, 0
                    );
#endif
             goto vty2flacc;
        case C_PARENS:     /* L3aba */
            L75e3 (cref);
            goto vty2flacc;
        case C_EQEQ:
        case C_NOTEQ:
        case C_GT_EQ:
        case C_LT_EQ:
        case C_GT:         /* L3ac3 */
        case C_LT:
        case C_PLUS:
        case C_NEG:
        case C_MULT:
        case C_SLASH:
            L3987 (cref->cr_Left);
#ifdef COCO
            gencode (C_TYFLOAT, C_RgStk);
#else
            gencode (C_TYFLOAT, C_RgStk, 0, 0);
#endif
            L3987 (cref->cr_Right);
#ifdef COCO
            gencode (C_TYFLOAT, _vrtyp);
#else
            gencode (C_TYFLOAT, _vrtyp, 0, 0);
#endif
            goto vty2flacc;
        case C_EQUAL:      /* L3af7 */
            L3987 (cref->cr_Left);
#ifdef COCO
            gencode (122, C_REG_X);
#else
            gencode (122, C_REG_X, 0, 0);
#endif
            L3987 (cref->cr_Right);
            goto L3b95;

L3b1d:
            var4 = cref->cr_Left;
             
            if (_ftyp == FT_FLOAT)  /* else L3b4f */
            {
                L3987 (var4->cr_Left);
#ifdef COCO
                gencode (122, C_REG_X);
                gencode (C_TYFLOAT, C_FLT2DBL);
#else
                gencode (122, C_REG_X, 0, 0);
                gencode (C_TYFLOAT, C_FLT2DBL, 0, 0);
#endif
            }
            else
            {
                L3987 (var4);
#ifdef COCO
                gencode (122, C_REG_X);
#else
                gencode (122, C_REG_X, 0, 0);
#endif
            }

            cref->vartyp = _vrtyp - 80;
            var4->vartyp = C_isRgX;
            L39a2 (cref);

            if (_ftyp == FT_FLOAT)
            {
#ifdef COCO
                gencode (C_TYFLOAT, C_TOFLOAT);
#else
                gencode (C_TYFLOAT, C_TOFLOAT, 0, 0);
#endif
            }
L3b95:
#ifdef COCO
            gencode (C_TYFLOAT, C_BIGMOV, _ftyp);
#else
            gencode (C_TYFLOAT, C_BIGMOV, _ftyp, 0);
#endif
L3bac:
            cref->vartyp = C_isRgX;
            cref->cmdval = 0;
            break;

        default:           /* L3bba */
            if (_vrtyp >= C_PLUSEQ)
            {
                goto L3b1d;
            }

            comperr (cref, "floats");
    }
}

