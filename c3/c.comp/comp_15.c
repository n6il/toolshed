/* ************************************************************************ *
 * comp_15.c part 15 for c.comp                                             *
 *                                                                          *
 * comes from first part of p2_03.c                                         *
 *                                                                          *
 * $Id:: comp_15.c 73 2008-10-03 20:20:29Z dlb                            $ *
 * ************************************************************************ */

#include "ccomp.h"

#ifndef __STDC__
static void L9577 ();
static void L97ae ();
static int swap_rel ();
static int invrs_rel ();
static int iszeroint ();
#else
static void L9577 (int, CMDREF *, int, int, int);
static void L97ae (CMDREF *);
static int swap_rel (int);
static int invrs_rel (int);
static int iszeroint (CMDREF *);
#endif

void
#ifndef __STDC__
L939d (cref, lblno, parm3, istrue)
    register CMDREF *cref;
    int lblno, parm3, istrue;
#else
L939d (CMDREF *cref, int lblno, int parm3, int istrue)
#endif
{
    int _vartyp;
    int _lblnum;

    switch (_vartyp =  cref->vartyp)
    {           /* break = L9573 */
        case C_ANDAND:     /* L93b2 */
            L939d (cref->cr_Left, (_lblnum = ++LblNum), parm3, 1);
            goto L93f0;
        case C_OROR:       /* L93d2 */
            L939d (cref->cr_Left, lblno, (_lblnum = ++LblNum), 0);
L93f0:
            loclbl_CR (_lblnum);
            L939d (cref->cr_Right, lblno, parm3, istrue);
            break;
        case C_EXCLAM:     /* L93fc */
            L939d (cref->cr_Left, parm3, lblno, (1 - istrue));
            break;
        case C_EQEQ:
        case C_NOTEQ:
        case C_LT_EQ:
        case C_LT:
        case C_GT_EQ:      /* L9411 */
        case C_GT:
        case C_U_LTEQ:
        case C_U_LT:
        case C_U_GTEQ:
        case C_U_GT:
            L9577 (_vartyp, cref, lblno, parm3, istrue);
            break;
        case C_INT:
        case C_DOUBLE:     /* L942b */
        case C_LONG:
            if (cref->cmdval && ( ! istrue))
            {
#ifdef COCO
                gencode (124, lblno, 0);
#else
                gencode (124, lblno, 0, 0);   /* output "lbra <lblno>" */
#endif
            }
            else
            {
                if (istrue && ( ! cref->cmdval))
                {
#ifdef COCO
                    gencode (124, parm3, 0);
#else
                    gencode (124, parm3, 0, 0);   /* output "lbra <parm3>" */
#endif
                }
            }
            break;
        case C_COMMA:      /* L9455 */
            L6f83 (cref->cr_Left);
            L939d (cref->cr_Right, lblno, parm3, istrue);
            break;          /* return */
        default:            /* L9476  */
            if (cref->ft_Ty == FT_LONG)
            {
                L34be (cref);

                /* output "lda 0,x\n ora 1,x\n ora 2,x\n ora 3,x"
                 * That is, test if == 0 */
#ifdef COCO
                gencode (C_TYLONG, 139);
#else
                gencode (C_TYLONG, 139, 0, 0);
#endif
                /* go to L94d7 */
            }
            else
            {
                if ((cref->ft_Ty == FT_FLOAT) || (cref->ft_Ty == FT_DOUBLE))
                {               /* else L94cc */
                    if (cref->vartyp == C_FLT2DBL)
                    {
                        cref = cref->cr_Left;
                    }

                    L3987 (cref);           /* L94b0 */

                    /* output " lda %MSByte,x\n" */
#ifdef COCO
                    gencode (C_TYFLOAT, 139, cref->ft_Ty);
#else
                    gencode (C_TYFLOAT, 139, cref->ft_Ty, 0);
#endif
                }
                else
                {
                    L97ae (cref);
                }
            }
#ifdef COCO
L94d7:
#endif
            if (istrue)
            {
#ifdef COCO
                gencode (130, C_EQEQ, parm3);
#else
                gencode (130, C_EQEQ, parm3, 0);  /* output "lbeq <parm3>" */
#endif
            }
            else
            {
#ifdef COCO
                gencode (130, C_NOTEQ, lblno);
#else
                gencode (130, C_NOTEQ, lblno, 0); /* output "lbne <lblno>" */
#endif
            }

            break;
    }
}

static void
#ifndef __STDC__
L9577 (eq_rel, cref, lbl_1, lbl_2, istruflg)
    int eq_rel;
    CMDREF *cref;
    int lbl_1, lbl_2, istruflg;
#else
L9577 ( int eq_rel,
        CMDREF *cref,
        int lbl_1,
        int lbl_2,
        int istruflg)
#endif
{
    int vtypleft;
    int _dstlbl;
    CMDREF *crLeft = cref->cr_Left;

    register CMDREF *_crRight = cref->cr_Right;

    _dstlbl = (istruflg ? lbl_2 : lbl_1);

    eq_rel = (istruflg ? swap_rel (eq_rel) : eq_rel);

    if (crLeft->ft_Ty == FT_LONG)
    {
        L34d9 (cref);       /* go to L95d5 */
        goto do_br_CC;
    }
    else
    {           /* L95be */
        if ((crLeft->ft_Ty == FT_FLOAT) || (crLeft->ft_Ty == FT_DOUBLE))
        {
            L39a2 (cref);
            goto do_br_CC;

        }
    }

    if ( (iszeroint ( crLeft))                                 ||
         ((int_usrlbl (crLeft)) && ( ! int_usrlbl (_crRight))) ||
         ((isRegU_Y (_crRight->vartyp)) &&
                        ( ! isRegU_Y (crLeft->vartyp)))
       )  /* else L9626 */
    {
        /* Original code used vtypleft for the temp storage, but non-CoCo
         * systems squawk. We could cast, but the following is clearer.
         * We could eliminate the #ifdef's, and can, but this will keep code
         * the same as original
         */
#ifdef COCO
#   define __tmpref vtypleft
#else
        CMDREF *__tmpref;
#endif

        /* Swap crLeft with _crRight */
        __tmpref = crLeft;        /* L9613 */
        crLeft = _crRight;
        _crRight = __tmpref;
        eq_rel = invrs_rel (eq_rel);
#ifdef COCO
#   undef __tmpref
#endif
    }

    vtypleft = crLeft->vartyp;        /* L9626 */

    if (isRegU_Y (vtypleft))       /* else L96a9 */
    {
        L6f83 (_crRight);

        switch (_crRight->vartyp)
        {
            case C_AMPERSAND:           /* L9642 */
                L6eac (_crRight);
                goto do_pshreg;
            case C_CHR2INT:    /* L964d */
                gencode (117, C_REG_D, 119, _crRight);
                _crRight->vartyp = C_REG_D;
                /* fall through to next case */
            case C_REG_X:
            case C_REG_Y:    /* L966c */
            case C_REG_U:
            case C_REG_D:
do_pshreg:
#ifdef COCO
                gencode (122, _crRight->vartyp);
#else
                gencode (122, _crRight->vartyp, 0, 0);  /* pshs REG */
#endif
                 _crRight->vartyp = C_RgStk;    /* go to L9725 */
            default:           /* L9725 */
                 break;
        }
    }       /* end if (isRegU_Y...) */
    else
    {
        if ((iszeroint (_crRight)) && (eq_rel < C_U_LTEQ))       /* L96a9 */
        {
            L97ae (crLeft);       /* go to L973e */
            /* this should probably have been goto do_br_CC; */
#ifdef COCO
            gencode (130, eq_rel, _dstlbl);
#else
            gencode (130, eq_rel, _dstlbl, 0);
#endif
            return;
        }
        else
        {
            L6eac (crLeft);       /* L96c6 */
            vtypleft = crLeft->vartyp;
            
            if (                               (L76bf (_crRight)) ||
                 ((vtypleft == C_REG_D) && int_usrlbl (_crRight))
               )
            {
                L6f83 (_crRight);   /* go to L9725 */
            }
            else
            {
#ifdef COCO
                gencode (122, vtypleft);
#else
                gencode (122, vtypleft, 0, 0);  /* gen 'pshs REG' */
#endif
                crLeft->vartyp = C_RgStk;
                L6eac (_crRight);
                vtypleft = _crRight->vartyp;
                _crRight = crLeft;
                eq_rel = invrs_rel (eq_rel);
            }
        }
    }       /* end else => if ! isRegU_Y... */

    /* gen code 'cmpREG _crRight-something */
    gencode (129, vtypleft, 119, _crRight); /* L9725... */
do_br_CC:
    /* gen code 'b_CC _dstlbl' */
#ifdef COCO
    gencode (130, eq_rel, _dstlbl);
#else
    gencode (130, eq_rel, _dstlbl, 0);
#endif
}

int
#ifndef __STDC__
int_usrlbl (cref)
    CMDREF *cref;
#else
int_usrlbl (CMDREF *cref)
#endif
{
    switch (cref->vartyp)
    {
        case C_USRLBL:      /* L9762 */
        case C_INT:
            return 1;
        case C_ASTERISK:    /* L9767 */
            return crf_isint (cref);
        default:
            return 0;
    }
}

#ifdef COCO
/* This function is unused - may be safely deleted if no further
 * COCO testing is needed */

p2_03_notused (parm1, parm2)
    CMDREF *parm1;
    int parm2;
{
    if ((parm1->vartyp == C_USRLBL) &&
                    ((LBLDEF *)(parm1->cmdval)->gentyp == FT_AUTO))
    {
        return 1;
    }

    return 0;
}
#endif

static void
#ifndef __STDC__
L97ae (cref)
    register CMDREF *cref;
#else
L97ae (CMDREF *cref)
#endif
{
    CMDREF *ref_right;
    int var0 = 0;

    switch (cref->vartyp)
    {                                       /* break = L9889 */
        case C_AND:        /* L97c2 */
            ref_right = cref->cr_Right;

            if ( (ref_right->vartyp != C_INT)       ||
                 ((unsigned int)(ref_right->cmdval) > (unsigned int)255)
               )
            {
                    var0 = 1;
            }

            break;

        case C_EQUAL:      /* L97de */
            if (   (isRegU_Y ((cref->cr_Left)->vartyp)) &&
                 ! ( int_usrlbl (cref->cr_Right))          )
            {
                var0 = 1;
            }

            break;

        case C_INCREMENT:
        case C_DECREMENT:
        case C_MINMINUS:
        case C_PLUSPLUS:
        case C_PLUSEQ:
        case C_MINEQU:     /* L97fc */
        case C_ANDEQ:
        case C_OREQ:
        case C_EOREQ:
            if ( ! isRegU_Y ((cref->cr_Left)->vartyp))
            {
                break;
            }
        case C_REG_Y:
        case C_REG_U:
        case C_VBAR:
        case C_CARET:      /* L980b */
        case C_TILDE:
        case C_MINUS:
        case C_PARENS:
            var0 = 1;
            break;
    }

    D0066 = 0;
    L6f83 (cref);

    switch (cref->vartyp)
    {
        case C_REG_X:
        case C_REG_D:          /* L9898 */
        case C_REG_Y:
        case C_REG_U:
            if (var0 || D0066)
            {
                gencode (129, cref->vartyp, C_INT, 0);
            }

            break;

        case C_CHR2INT:    /* L98b2 */
            gencode (117, C_REG_D, 119, (cref = cref->cr_Left));
            break;
        default:           /* L98b4 */
            gencode (117, C_REG_D, 119, cref);
            break;
    }
}

/* ******************************************************************** *
 * swap_rel () - Returns the reverse relation for the param             *
 *              I.E. ">" becomes "<"                                    *
 * ******************************************************************** */

static int
#ifndef __STDC__
swap_rel (parm1)
    int parm1;
#else
swap_rel (int parm1)
#endif
{
    switch (parm1)
    {
        case C_EQEQ:        /* L98fd */
            return C_NOTEQ;
        case C_NOTEQ:       /* L9902 */
            return C_EQEQ;
        default:            /* L9907 */
            return (((parm1 > C_GT) ? 195 : 187) - parm1);
    }
}

/* ******************************************************************** *
 * invrs_rel () - Returns the inverse relation for parameter passed.    *
 *              I.E. ">" returns as "<=", etc.                          *
 * ******************************************************************** */

static int
#ifndef __STDC__
invrs_rel (parm1)
    int parm1;
#else
invrs_rel (int parm1)
#endif
{
    switch (parm1)
    {
        case C_EQEQ:       /* L9935 */
        case C_NOTEQ:
            return parm1;
        case C_LT_EQ:
        case C_LT:         /* L9939 */
        case C_U_LTEQ:
        case C_U_LT:
            return (parm1 + 2);
        default:           /* L9940 */
            return (parm1 - 2);
    }
}

static int
#ifndef __STDC__
iszeroint (cref)
    register CMDREF *cref;
#else
iszeroint (CMDREF *cref)
#endif
{
    return ((cref->vartyp == C_INT) && ( ! cref->cmdval));

    /*if ((cref->vartyp == C_INT) && ( ! cref->cmdval))
    {
        return 1;
    }
    else
    {
        return 0;
    }*/
}

