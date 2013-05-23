/* **************************************************************** *
 * ccomp_04.c                                                       *
 *                                                                  *
 * comes from p1_03.c                                               *
 * $Id:: comp_04.c 73 2008-10-03 20:20:29Z dlb                    $ *
 * **************************************************************** */

#include "ccomp.h"

static CMDREF *calcconsts (
#ifdef __STDC__
    register CMDREF *regptr
#endif
);

static int is_const (
#ifdef __STDC__
    register CMDREF *regptr
#endif
);

static CMDREF *L1587 (
#ifdef __STDC__
    CMDREF *oldref
#endif
);

static void cktsttyp (
#ifdef __STDC__
    register CMDREF *regptr
#endif
);
static int dflt_cast (
#ifdef __STDC__
    register CMDREF *c_ref
#endif
);

static int cast_alike (
#ifdef __STDC__
    register CMDREF *cref_1, CMDREF *cref_2
#endif
);

static CMDREF *mult_const (
#ifdef __STDC__
    int p1, int p2, struct brktdef *p3, CMDREF *p4
#endif
);

static void ckif_lvalue (
#ifdef __STDC__
    register CMDREF *regptr, int p2
#endif
);

static int membrsize (
#ifdef __STDC__
    register CMDREF *regptr, int *p2, int *p3
#endif
);

/* Coco doesn't use cocodbl - this replaces COCO library routines */
#ifndef COCO
extern int *cocodbl (
#ifdef __STDC__
    char *, int *
#endif
);
#endif

CMDREF *
#ifdef __STDC__
L111d (register CMDREF *curntcmd)
#else
L111d (curntcmd)
register CMDREF *curntcmd;
#endif
{
    CMDREF *_cmdLeft;
    CMDREF *_cmdRight;
#ifdef COCO
    int v6;     /* unused - can delete if not comparing code */
#endif
    int _varty;
    CMDREF *_cLeft_cRight;
#ifdef COCO
    int v0;     /* unused - can delete if not comparing code */
#endif

    if (curntcmd)    /* else L1204 (return) */
    {
        /* The following two commands  walk down the tree all the
         * way to the left and then begins processing the commands
         * as it comes back up.  As it walks up (to the right),
         * it will then call itself for all commands below, and to
         * the right of the cmdref being processed.
         * It works like a "sort" routine for all crefs under the
         * cref first called.
         */
        
        curntcmd->cr_Left = L111d (curntcmd->cr_Left);
        curntcmd->cr_Right = L111d (curntcmd->cr_Right);
        curntcmd = L1587 (curntcmd);    /* Handle casts ? */

        /* If it's a constant, return unchanged */

        if (is_const (curntcmd))
        {

            return curntcmd;            /*goto L1202;*/
        }

        /* Calculate and combine if they are constants */

        curntcmd = calcconsts (curntcmd);        /* L115b */
        _cmdLeft = curntcmd->cr_Left;
        _cmdRight = curntcmd->cr_Right;
        
        if (((_varty = curntcmd->vartyp) == C_QUESTION) &&
                      (_cmdLeft->vartyp == C_INT))   /* else L11b9 */
        {
            if (_cmdLeft->cmdval)      /* else L1192 */
            {
                _cLeft_cRight = _cmdRight->cr_Left;
                CCREFtoLeft (_cmdRight->cr_Right);
            }
            else
            {
                _cLeft_cRight = _cmdRight->cr_Right;       /* L1192 */
                CCREFtoLeft (_cmdRight->cr_Left);
            }

            /* CurntCmd becomes cr_Left for _cmdRight
             * _cmdRight becomes cr_Left for cmd_Left
             * _cmd_Left becomes cr_Left for curntcmd
             */

            mk_lftcmd (_cmdRight);
            mk_lftcmd (_cmdLeft);
            mk_lftcmd (curntcmd);
        }
        else        /* Not "? :" */
        {           /* Not a ptr or address */  /* L11b9 */
            if ((_varty != C_ASTERISK) || (_cmdLeft->vartyp != C_AMPERSAND))
            {
                if ((_varty != C_AMPERSAND) || (_cmdLeft->vartyp != C_ASTERISK))
                {
                    /* not "**", "*&", "&*" or "&&" */
                    return curntcmd;
                }
            }

            /* _cmdLeft inherit ft_Ty & varsize from curntcmd */

            _cLeft_cRight = _cmdLeft->cr_Left;      /* L11dd */

            _cLeft_cRight->ft_Ty = curntcmd->ft_Ty;
            _cLeft_cRight->varsize = curntcmd->varsize;
            
            /* CurCmdRef becomes cr_Left for curntcmd
             * curntcmd becomes cr_Left for _cmdLeft
             */

            mk_lftcmd (curntcmd);
            mk_lftcmd (_cmdLeft);
        }
        curntcmd = _cLeft_cRight;      /* L11f9 */
    }

#ifdef COCO
L1202:
#endif
    return curntcmd;
}

/* ******************************************************************** *
 * calcconsts () - Calculates the value of constant expressions if      *
 *              whole expression results in a constant.                 *
 *              Reconstructs CMDREF tree when appropriate.              *
 * Returns: Updated CMDREF of focus, with values updated if complete    *
 *          expression results in a constant                            *
 *          In Most cases, the CMDREF's left and right ref ptrs are     *
 *          nulled  out and a chain with the Left and Right ref's,      *
 *          with the former contents of CurntCREF being the left ref    *
 *          for leftref and CurntCREF pointing to the right-ref.        *
 * ******************************************************************** */

static CMDREF *
#ifdef __STDC__
calcconsts (register CMDREF *cntr_cref)
#else
calcconsts (cntr_cref)
    register CMDREF *cntr_cref;
#endif
{
    CMDREF *_leftCMD;
    CMDREF *_rightCMD;
    CMDREF *_tmpLftCref;
    int _myvartyp;
#ifdef COCO
    int v4;     /* Not used - may delete if no more comparing with orig. */
#endif
    int l_cnstint;  /* TRUE if  _leftCMD->vartyp == C_INT */
    int r_cnstint;  /* TRUE if _rightCMD->vartyp == C_INT */

    if ( ! cntr_cref)
    {
        return 0;
    }

    _leftCMD = cntr_cref->cr_Left;      /* L121f */
    _rightCMD = cntr_cref->cr_Right;
    
    _myvartyp = cntr_cref->vartyp;

    if ( (numeric_op (_myvartyp)) ||
          (_myvartyp == C_ANDAND) ||
            (_myvartyp == C_OROR)
       )
    {                                       /* else L145d */
            /* NOTE:  The following single "&" _is_ correct. *
             * It should not be "&&"
             */

        /* if both are constant ints, merge and return */

        if ( (l_cnstint = (_leftCMD->vartyp == C_INT) )  &     /* L1246 */
             (r_cnstint = (_rightCMD->vartyp == C_INT))
           )
        {
            goto mrg_n_exit;
        }

        if (l_cnstint)  /* I.E. <constant int> ... <some-other-type> */
        {                           /* L127b */     /* else L12bb */
            switch (_myvartyp)
            {
                case C_PLUS: /* L1283 */    /* _44 */
                case C_MULT:
                case C_AND:
                case C_VBAR:
                case C_CARET:

                        /* The above are transmutable.
                         * Swap left for right so we have constant last */

                    _tmpLftCref = _leftCMD;
                    cntr_cref->cr_Left = _leftCMD = _rightCMD;
                    cntr_cref->cr_Right = _rightCMD = _tmpLftCref;
                    r_cnstint = 1;
                    break;
            }
        }

        switch (_myvartyp)
        {
            case C_VBAR:    /* L12c0 (_51) */
            case C_CARET:
                
                /* constant is 0 */

                if ((r_cnstint) && ( ! _rightCMD->cmdval))
                {
                    goto swap_n_retrn;
                }

                return cntr_cref;
            case C_PLUS:    /* L12d1 (_60) */
                
                /* if both are non-const, return doing nothing */
                
                if ( ! r_cnstint)
                {
                    return cntr_cref;
                }

                /* + 0 is a no-op.. Update crefs and return */

                if ( ! (_rightCMD->cmdval))
                {
                    goto swap_n_retrn;
                }

                switch (_leftCMD->vartyp)
                {
                    case C_AMPERSAND:   /* L12e6 */
                        (_leftCMD->cr_Left)->ptrdstval += _rightCMD->cmdval;
swap_n_retrn:
                        /* Result of following:
                         *                        _rightCMD
                         *                       /
                         *                     cntr_cref
                         *                    /
                         *               CurntCREF
                         * _rightCMD => CurntCREF
                         *  cntr_cref = _leftCMD
                         */
                        
                        /* CurntCREF now points to _rightCMD,
                         * _leftCMD->ptrdstval = math result of left-right
                         * _leftCMD is returned to caller
                         */

                        _tmpLftCref = _leftCMD;
                        mk_lftcmd (cntr_cref);
                        mk_lftcmd (_rightCMD);
                        cntr_cref = _tmpLftCref;
                        return cntr_cref;
                    case C_PLUS:        /* L1317 */
                        if ((_leftCMD->cr_Right)->vartyp == C_INT)
                        {
                            ((_leftCMD->cr_Right)->cmdval) += _rightCMD->cmdval;

                            /* Reshuffle CREF's and return leftcmd as
                             * center of focus
                             */
                            goto swap_n_retrn;
                        }

                        return cntr_cref;
                    default:            /* L1522 */
                        return cntr_cref;
                }
            case C_NEG:     /* L1347 ( _71 ) */

                /* If right is constant, negate right, switch to ADD
                 * call self to add
                 */

                if (r_cnstint)
                {
                    _rightCMD->cmdval = -(_rightCMD->cmdval);
                    cntr_cref->vartyp = C_PLUS;
                    return (calcconsts (cntr_cref));
                }

                if (l_cnstint)    /* L1366 */
                {
                    if ( ! _leftCMD->cmdval)    /* 0 - <something> */
                    {
                        cntr_cref->vartyp = C_MINUS;
                        cntr_cref->cr_Left = _rightCMD;
                        cntr_cref->cr_Right = 0;
                        mk_lftcmd (_leftCMD);
                    }
                }

                return cntr_cref;
            case C_AND:     /* L1386 */
                if ((r_cnstint) && ! (_rightCMD->cmdval))
                {
                    goto retrn_0;
                    /*return 0;*/
                }
                else
                {
                    return cntr_cref;
                }

            case C_MULT:     /* _97  (L1395) */
                if ( ! r_cnstint)
                {
                    return cntr_cref;
                }

                switch (_rightCMD->cmdval)
                {
                    case 0:         /* L13a1 ( _106 ) */
                        goto retrn_0;
                    default:        /* L13a6 */
                        if ( (_leftCMD->vartyp != C_MULT)             ||
                             ((_leftCMD->cr_Right)->vartyp != C_INT)
                           )
                        {
                            return cntr_cref;
                        }

                        (_leftCMD->cr_Right)->cmdval *= _rightCMD->cmdval;
                        /* Fall through to next case */
                    case 1: /* "* 1" is a no-op */     /* L13ee ( _113 ) */
                        goto swap_n_retrn;
                }
            case C_SLASH:   /* L13e0 ( _104 ) */
                if (r_cnstint && (_rightCMD->cmdval == 1))
                {
                    goto swap_n_retrn;
                }

                /* 0 / <something> */

                if (l_cnstint && ! (_leftCMD->cmdval))   /* L13f3 */
                {
retrn_0:
                    cntr_cref->vartyp = C_INT;
                    cntr_cref->cmdval = 0;          /* L140c */
                    cntr_cref->cr_Left = cntr_cref->cr_Right = 0;

                    /* CurCMDREF becomes left for _leftCMD */
                    
                    mk_lftcmd (_leftCMD);
                    mk_lftcmd (_rightCMD);
                }

                break;
        }           /* end switch (_myvartype) */

        return cntr_cref;      /* L1522 */
    }       /* end if math function */

    /* L145d */
    switch (_myvartyp)     /* L150d ( _124 ) */
    {
        case C_EXCLAM: /* L1462 ( _125 ) */
        case C_TILDE:
        case C_MINUS:
            if (_leftCMD->vartyp == C_INT)     /* else L1492 */
            {
mrg_n_exit:
                cntr_cref->vartyp = C_INT;
                cntr_cref->__cr18 = 0;

                /* calculate the value of the cmdvals */
#ifdef COCO
                cntr_cref->cmdval = do_math ( _myvartyp,
                                             _leftCMD->cmdval,
                                             _rightCMD->cmdval
                                           );
#else
                /* I believe making parm3 0 will work.  do_math doesn't
                 * appear to use it for these three cases
                 */
                /* Didn't work... trying "if" now... */

                cntr_cref->cmdval = do_math (_myvartyp,
                                            _leftCMD->cmdval,
                                            _rightCMD ? _rightCMD->cmdval : 0);
#endif
                /* bra L140c */
                /* eliminate left & right crefs of cntr_cref
                 * left and right values combined into cntr_cref
                 */

                cntr_cref->cr_Left = cntr_cref->cr_Right = 0;

                /* result of following:
                 * CurntCREF becomes the cr_Left for _leftCMD
                 * _leftCMD => CurntCREF
                 */
                
                mk_lftcmd (_leftCMD);
                mk_lftcmd (_rightCMD);
            }
            else
            {                                       /* L1492 */
                if (_leftCMD->vartyp == C_LONG)     /* else L14e1 */
                {
                    long *lngptr;

#ifdef COCO
                    if ( ! (lngptr = _leftCMD->cmdval))    /* else L14b2 */
                    {
                        lngptr = addmem (sizeof (long));

                        /* This doesn't seem right.  It seems like lngptr
                         * should be initialized...
                         */
                    }
#else
                    /* Longs are stored directly in the cross assemblers,
                     * but we will use pointers to keep down the #ifdef's
                     */

                    lngptr = (long *)(&(_leftCMD->cmdval));
#endif

                    switch (_myvartyp)
                    {
                        case C_MINUS:                   /* L14b6 */
                            *lngptr = -(*lngptr);
                            break;
                        case C_TILDE:                   /* L14c1 */
                            *lngptr = ~(*lngptr);
                            break;
                    }

#ifdef COCO         /* already there for non-coco's */
                    _leftCMD->cmdval = (int)lngptr;   /* go to L1501 */
#endif
                    mk_lftcmd (cntr_cref);      /* L1501 */
                    cntr_cref = _leftCMD;
                }
                else
                {
                    if (_leftCMD->vartyp == C_DOUBLE)        /* L14e1 */
                    {
#ifdef COCO
                        double *dptr;

                        if (dptr = (double *)(_leftCMD->cmdval))
                        {
                            *dptr = -(*dptr);
                        }
#else
                        struct dbltree *dptr;

                        if ((dptr = (struct dbltree *)(_leftCMD->cmdval)))
                        {               /* else L1501 */
                            dptr->native = -(dptr->native);

                            /* make sure that the MS Word is sign extended
                             * I don't think that in the cross disassembler,
                             * we consiger pos/neg, but simply the COCO
                             * sign bit
                             */

                            if ((dptr->cocoarr)[0] & 0x8000)
                            {
                                (dptr->cocoarr)[0] |= (-1 << 16);
                            }
                            else
                            {
                                (dptr->cocoarr)[0] &= 0xffff;
                            }

                            /* This is the negation of a COCO double */
                            (dptr->cocoarr)[0] ^= (-1 << 15);
                        }
#endif

                        mk_lftcmd (cntr_cref);      /* L1501 */
                        cntr_cref = _leftCMD;
                    }
                }
            }
    }

    return cntr_cref;
}

static int
#ifdef __STDC__
is_const (register CMDREF *regptr)
#else
is_const (regptr)
    register CMDREF *regptr;
#endif
{
    if (regptr)
    {
        switch (regptr->vartyp)
        {
            case C_REG_Y:  /* L153a */
            case C_REG_U:
            case C_INT:
            case C_LONG:
            case C_DOUBLE:
            case C_USRLBL:
            case C_DQUOT:
                return 1;
        }
    }

    return 0;
}

void
#ifdef __STDC__
divby_0 (void)
#else
divby_0 ()
#endif
{
    reprterr ("divide by zero");
}

/* ************************************************************ *
 * L1587 () - Insures that two vars are of compatible type      *
 *          and does casts when legal                           *
 * ************************************************************ */

static CMDREF *
#ifdef __STDC__
L1587 (CMDREF *myref)
#else
L1587 (myref)
    CMDREF *myref;
#endif
{
    CMDREF *right_ref;
#ifdef COCO
    LBLDEF *v16;
#else
    void *v16;
#endif
    int realvrtyp;
    int _tftyp;
    int _siz;
    int _t_cr_18;
    struct brktdef *_t_arrmbrs;
    int _typ_left;
    int fty_right;
    int is_ref;

    register CMDREF *left_ref = myref->cr_Left;

    right_ref = myref->cr_Right;
    _t_cr_18 = _siz = INTSIZ;   /* default is int */
    _t_arrmbrs = 0;
    _tftyp = FT_INT;          /* default is int */
    
    switch (realvrtyp = myref->vartyp)
    {
        case C_USRLBL:         /* L15bb */
            v16 = (void *)(myref->cmdval);
            
            if (((LBLDEF *)v16)->fnccode == FT_TYPEDEF)    /* else L15eb */
            {
                err_lin (myref, "typedef - not a variable");
                resetcmdref (myref);
                break;
            }

            _tftyp = ((LBLDEF *)v16)->gentyp;      /* L15eb */
            _t_arrmbrs = ((LBLDEF *)v16)->arrmbrs;

            /* if it's a structure (?), move basic gentype from LBLDEF (?)
             * stored at v16->vsize to v16->gentype, preserving all
             * ptr/brace/bracket depts.  Set size of v16 to that of
             * the stored LBLDEF.
             * NOTE: We did encounter some problems with this routine
             * (see below)
             */

            /* Following may not be G_STRCNAM (10) */
            if ((_tftyp & 0x0f) == G_STRCNAM)     /* else L162c */
            {
                LBLDEF *loc;

                /* FIXME : We need to investigate this. For structures, is
                 * vsize used to store ptr to memberdef?
                 */

                loc = ((LBLDEF *)v16)->vsize;
#ifdef COCO
                _tftyp = v16->gentyp =
                                (_tftyp & 0xfff0) + loc->gentyp;
                setlblsize (v16, _t_arrmbrs, loc->vsize);
#else
                /* The above corresponds to the the original code, but often
                 * accessing loc produces segfaults under Linux.  The
                 * original code _could_ be wrong.
                 * In tests on the COCO, often loc carries a value of 2
                 * or the like - a segfault for any system that protects
                 * against such.  COCO "seems" to handle this correctly,
                 * although I don't see how, as it always uses
                 * *(loc + 2 bytes) irregardless.  I did some testing
                 * with the original COCO program, insering some printf
                 * calls at this location to see if it, too, came up with
                 * these non-pointer values, and yes, indeed, they
                 * corresponded with the Linux progs, so it is not an
                 * error in the cross compiler.  Some of the vsize's
                 * come here not being pointers as expected.
                 * IIRC, the only place this showed up for me was in compiling
                 * the source for cc 2.2.1.  And it would crash with some
                 * of the structs from <module.h>  mod_exec, for one, of
                 * not the only one.
                 */
                
                /* FIXME */
                /* The following may be a kludge... hopefully it will work The
                 * problem is that sometimes, we get a segfault because loc is
                 * not a pointer, and is outside valid storage area.  It did
                 * not bother operation on the COCO, but causes a crash on
                 * other systems.
                 * 0x7fff was arbitrarily chosen as a flag to signify a valid
                 * storage area. This could fail at some time.
                 */

                /* One other possibility I've thought of is to test
                 * for loc < 0x7fff (or possibly even as small as INTSIZ)
                 * and simply skip this.  loc comes from v16->vsize, and
                 * the main change below is v16->vsize (as well as
                 * v16->gentyp.  I didn't try it
                 */

                _tftyp = ((LBLDEF *)v16)->gentyp = (_tftyp & 0xfff0)    +
                                            ((loc > (LBLDEF *)0x7fff) ?
                /* Just guessing at a default... Trying FT_INT */
                                                    loc->gentyp : FT_INT);

                setlblsize ( (LBLDEF *)v16,
                             _t_arrmbrs,
                             ((loc > (LBLDEF *)0x7fff) ? loc->vsize :
                                                         (int)loc)
                           );
#endif
            }

            _siz = ((LBLDEF *)v16)->vsize;

            /* Handle arrays and parenthesized expressions */

            if ((inbrkets (_tftyp)) || (inparentheses (_tftyp)))
            {                   /*else L16ae*/
                left_ref = myref;
                left_ref->varsize = _siz;
                
                myref = add_cmdref ( C_AMPERSAND, left_ref,
                                     0, 0,  /* No left or right */
                                     myref->_cline, myref->_lpos);

                /* For arrays, bump _t_arrmbrs to prev member,
                 * change this level's bracket-def into ptrdepth
                 * for left-ref and _tftyp.
                 */

                /* For parenthesized expr's, simply increase ptrdepth
                 */

                if (inbrkets (_tftyp))       /* else L1699 */
                {
                    _t_arrmbrs = prevbrkt (_t_arrmbrs);
                    _tftyp =
                          incptrdpth ( left_ref->ft_Ty = MSBrshft2 (_tftyp) );
                }
                else
                {
                    _tftyp = incptrdpth (left_ref->ft_Ty = _tftyp);
                }

                left_ref->__cr18 = 1;
            }
            else        /* Not array or parentheses */
            {
                switch (_typ_left = ((LBLDEF *)v16)->fnccode)      /* L16ae */
                {
                    case C_REG_Y:    /* _179 */     /* L16b9 */
                    case C_REG_U:
                        myref->vartyp = _typ_left;
                        myref->cmdval = 0;
                        break;
                }

            }

            _t_cr_18 = 1;        /* L16d5 */
            break;
        case C_INT:    /* L16da */
            _t_cr_18 = 0;
            break;
        case C_LONG:        /* _182 */      /* L16e1 */
            _tftyp = FT_LONG;
            _t_cr_18 = 1;
            _siz = LONGSIZ;
            break;
        case C_DOUBLE:      /* L16f1 */
            _tftyp = FT_DOUBLE;
            _t_cr_18 = 1;
            _siz = DBLSIZ;
            break;
        case C_DQUOT:       /* L1701 */
            _tftyp = FT_RETURN;
            _siz = CHARSIZ;
            break;
        case 32:    /* Cast */         /* L170c */
            ck_declared (left_ref);

            /* Result of a cast cannot be array or function */

            if ((inparentheses ( _tftyp = myref->ft_Ty)) ||
                                 (inbrkets (_tftyp))        )
            {
                err_lin (myref, "cannot cast");
                _tftyp = incptrdpth (_tftyp);
            }

            if (ispointer (_tftyp))
            {
                do_cast (left_ref, FT_INT);
            }
            else
            {
                _tftyp = do_cast (left_ref, _tftyp);
            }

            _t_arrmbrs = myref->arrdefs;        /* L176d */
            _siz = myref->varsize;
            mk_lftcmd (myref);
            myref = left_ref;
            break;
        case C_AMPERSAND:   /* L178b */
            ckif_lvalue (left_ref, 1);
            
            if ((left_ref->vartyp == C_REG_Y) || (left_ref->vartyp == C_REG_U))
            {
                err_lin (myref, "can't take address");
                resetcmdref (left_ref);
            }

            _t_cr_18 = left_ref->__cr18;
            _siz = left_ref->varsize;
            _tftyp = incptrdpth (left_ref->ft_Ty);
            _t_arrmbrs = left_ref->arrdefs;
            break;
        case C_ASTERISK:    /* L17d9 */
            _t_arrmbrs = left_ref->arrdefs;
            
            if (ispointer (_tftyp = left_ref->ft_Ty)) /* else L181d */
            {
                /* Decrease pointer depth by one */

                _tftyp = MSBrshft2 (_tftyp);

                /* If array, replace bracket state with pointer state
                 * make left_ref the current ref
                 */

                if (inbrkets (_tftyp))  /* else L183f */
                {
                    _tftyp = incptrdpth (MSBrshft2 (_tftyp));
                    mk_lftcmd (myref);
                    myref = left_ref;
                }
            }
            else
            {
                err_lin (left_ref, "pointer required");
                resetcmdref (left_ref);
                left_ref->ft_Ty = 17;
                _tftyp = FT_INT;
                _t_arrmbrs = 0;
            }

            /* Replace current vars with those of left_ref */

            _t_cr_18 = left_ref->__cr18;     /* L183f */
            _siz = left_ref->varsize;
            break;
        case C_PERIOD:      /* L184b */
            _siz = membrsize (right_ref, &_tftyp, &is_ref);
            _t_arrmbrs = right_ref->arrdefs;
            ckif_lvalue (left_ref, 1);

            if ( !  (right_ref->cmdval) && ! (is_ref))
            {
                mk_lftcmd (right_ref);
                mk_lftcmd (myref);
                myref = left_ref;
                _t_cr_18 = myref->__cr18;     /* go to L1902 */
            }
            else
            {
                _t_cr_18 = left_ref->__cr18;     /* L189f */

                if (left_ref->vartyp == C_ASTERISK)   /* else L18c4 */
                {
                    v16 = left_ref->cr_Left;
                    mk_lftcmd (left_ref);
                    myref->cr_Left = (left_ref = (CMDREF *)v16);
                }
                else
                {
                    _typ_left = left_ref->ft_Ty;        /* L18c4 */
                    left_ref = myref->cr_Left =
                                add_cmdref ( C_AMPERSAND,
                                             left_ref, 0,   /* left, right */
                                             0,
                                             left_ref->_cline,
                                             left_ref->_lpos);
                    left_ref->ft_Ty = incptrdpth (_typ_left);
                }

                left_ref->__cr18 = _t_cr_18;
                goto L1960;
            }

            break;
        case C_PTRREF:      /* L1905 */
            _siz = membrsize (right_ref, &_tftyp, &is_ref);
            _t_arrmbrs = right_ref->arrdefs;
            
            if (((_typ_left = left_ref->ft_Ty) != FT_INT))   /* else L1959 */
            {
                if ( ! (ispointer (_typ_left)))   /* Trying to match code */
                {
                    err_lin (left_ref, "pointer or integer required");
                    left_ref->vartyp = C_INT;
                    left_ref->cmdval = 0;
                    left_ref->__cr18 = 0;
                }
            }

            _t_cr_18 = left_ref->__cr18;
L1960:
            myref->vartyp = C_PLUS;
            myref = calcconsts (myref);

            if ( ! is_ref)        /* else L19c0 */
            {
                myref->ft_Ty = incptrdpth (_tftyp);
                myref->__cr18 = _t_cr_18;
                myref->varsize = _siz; 
                myref = add_cmdref (C_ASTERISK, myref, 0, 0,
                                    myref-> _cline, myref->_lpos);
            }

            break;
        case C_PARENS:      /* function */         /* L19c3 */
            _tftyp = left_ref->ft_Ty;

            if ((left_ref->vartyp == C_AMPERSAND) &&
                    (inparentheses (MSBrshft2 (_tftyp))))
            {       /* else L19ff */
                v16 = myref->cr_Left = left_ref->cr_Left;
                mk_lftcmd (left_ref);
                _tftyp = MSBrshft2 (((CMDREF *)v16)->ft_Ty);
                /* go to L1a63 */
            }
            else
            {
                if (inparentheses (_tftyp))
                {
                    _tftyp = MSBrshft2 (_tftyp);      /* go to L1a63 */
                }
                else
                {
                    if ( ! _tftyp)       /* else L1a51 */
                    {
                        v16 = (void *)(left_ref->cmdval);
                        ((LBLDEF *)v16)->gentyp = 0x30 + FT_INT;
                        ((LBLDEF *)v16)->vsize = INTSIZ;
                        ((LBLDEF *)v16)->arrmbrs = 0;
                        ((LBLDEF *)v16)->fnccode = FT_EXTERN;
                        ((LBLDEF *)v16)->isfunction = 0;

                        /* copy first 3 members from v16 to left_ref */
#ifdef COCO
                        mem_cp (v16, left_ref, sizeof (left_ref->ft_Ty)   +
                                               sizeof (left_ref->varsize) +
                                               sizeof (left_ref->arrdefs));
#else
                        memcpy (left_ref, v16, sizeof (left_ref->ft_Ty)   +
                                               sizeof (left_ref->varsize) +
                                               sizeof (left_ref->arrdefs));
#endif
                        _tftyp = FT_INT;
                    }
                    else
                    {
                        err_lin (left_ref, "not a function");
                        _tftyp &= 0x0f;
                    }
                }
            }

            _t_arrmbrs = left_ref->arrdefs;        /* L1a63 */
            _siz = left_ref->varsize;       /* Fall through to next case */
        case FT_PARAM:         /* L1a6d */
            ck_declared (left_ref);
            get_fttyp (left_ref);

            switch (left_ref->ft_Ty)
            {
                case FT_CHAR:     /* L1a7f */
                     do_cast (left_ref, FT_INT);
                     break;
                case FT_FLOAT:     /* L1a84 */
                     do_cast (left_ref, FT_DOUBLE);
                     break;
            }

            break;
        case C_COMMA:       /* L1a9f */
            _siz = right_ref->varsize;
            _tftyp = right_ref->ft_Ty;
            _t_arrmbrs = right_ref->arrdefs;
            break;
        case C_EXCLAM:      /* L1ab5 */
            if ( ! (ispointer (left_ref->ft_Ty)))
            {
                dflt_cast (left_ref);
            }

            _tftyp = FT_INT;
            break;
        case C_MINUS:       /* L1acd */
            _tftyp = dflt_cast (left_ref);
            break;
        case C_TILDE:       /* L1ad7 */
            _tftyp = dflt_cast (left_ref);

            if (_tftyp == FT_DOUBLE)    /* else L1bc8 */
            {
                notintegral (left_ref);
                resetcmdref (left_ref);
            }

            break;
        case C_PLUSPLUS:    /* _269  (L1af4) */
        case C_INCREMENT:
        case C_MINMINUS:
        case C_DECREMENT:
            ckif_lvalue (left_ref, 0);
            _siz = left_ref->varsize;
            
                    /* If ptr, braces, or brackets */

            if ((_tftyp = left_ref->ft_Ty) & 0x30)      /* else L1b25 */
            {
                myref->cmdval = get_varsize ( MSBrshft2 (_tftyp),
                                              left_ref->varsize,
                                              left_ref->arrdefs);
            }
            else
            {
                myref->cmdval = 1;       /* L1b25 */
            }

            _t_cr_18 = left_ref->__cr18;

            if (left_ref->vartyp == C_ASTERISK)   /* else L1cac */
            {
                ++_t_cr_18;
            }

            break;
        case C_ANDAND:      /* _278  (L1b44) */
        case C_OROR:        /* _278 */
            if ( ! ispointer (left_ref->ft_Ty))
            {
                dflt_cast (left_ref);
            }

            if ( ! (ispointer (right_ref->ft_Ty)))      /* L1b56 */
            {
                dflt_cast (right_ref);
            }

            break;
        case C_SLASH:       /* L2006 */
        case C_MULT:
            _tftyp = cast_alike (left_ref, right_ref);
            break;
        case C_AND:         /* _292  (L1b6e) */
        case C_VBAR:
        case C_CARET:
        case C_PERCENT:
            if ((_tftyp = cast_alike (left_ref, right_ref)) == FT_DOUBLE)
            {
                goto needints;  /* error - both must be integral */
            }

            break;
        case C_LSHIFT:      /* L1b8a */
        case C_RSHIFT:
            if (((_tftyp = dflt_cast (left_ref)) == FT_DOUBLE) ||
                    (dflt_cast (right_ref) == FT_DOUBLE))   /* else L1bcb */
            {
needints:
                err_lin (myref, "both must be integral");
                resetcmdref (myref);
                break;
            }

            do_cast (right_ref, FT_INT);
            break;
        case C_LT:          /* L1bdb */
        case C_GT:
        case C_LT_EQ:
        case C_GT_EQ:
            if ( ((ispointer (left_ref->ft_Ty))     &&
                        (ispointer (right_ref->ft_Ty))
                 )  ||
                 (cast_alike (left_ref, right_ref) == FT_UNSIGNED))
            {
                myref->vartyp = realvrtyp + 4;   /* L1c04 */
            }
                

            break;
        case C_EQEQ:        /* L1c11 */
        case C_NOTEQ:
            if (ispointer (left_ref->ft_Ty))    /* else L1c4a */
            {
                if ( ! ((ispointer (right_ref->ft_Ty))))      /* else L1c56 */
                {
                    dflt_cast (right_ref);        /* L1c2f + 1 line */
                    do_cast (right_ref, FT_INT);     /* go to L1c54 */
                }
            }
            else
            {
                cast_alike (left_ref, right_ref);    /* L1c4a */
            }
            
            break;      /* L1c56 */
        case C_PLUS:        /* L1c59 */
            if (ispointer (right_ref->ft_Ty))     /* else L1c78 */
            {
                /* Swap right_ref and left_ref */
                {
                    CMDREF *__tmp = left_ref;

                    left_ref = right_ref;
                    right_ref = __tmp;
                }

                myref->cr_Left = left_ref;
            }

            _siz = left_ref->varsize;        /* L1c78 */
            _tftyp = left_ref->ft_Ty;

            if (ispointer (_tftyp))
            {
                _t_arrmbrs = left_ref->arrdefs;
                goto L1d92;
            }

            _tftyp = cast_alike (left_ref, right_ref);         /* L1c93 */
            _t_cr_18 = left_ref->__cr18 + right_ref->__cr18;
            break;
        case C_NEG:         /* L1caf */
            _t_cr_18 = left_ref->__cr18 + right_ref->__cr18;
            
            if (ispointer ((_tftyp = left_ref->ft_Ty)))   /* else L2006 */
            {
                _siz = left_ref->varsize;
                _t_arrmbrs = left_ref->arrdefs;

                if (ispointer (right_ref->ft_Ty))    /* else L1d95 */
                {
                    myref->__cr18 = _t_cr_18;

                    if ( (_tftyp != right_ref->ft_Ty) ||
                         (_siz != right_ref->varsize)      )
                    {
                        err_lin (myref, "pointer mismatch");
                        /* go to L1d83 */
                    }
                    else
                    {
                        _tftyp = MSBrshft2 (_tftyp);

                        _siz = get_varsize (_tftyp, _siz, _t_arrmbrs);

                        if (_siz != 1)  /* Not a char array */ /* else L1d83 */
                        {
                            _t_cr_18 = 2;

                            v16 = add_cmdref (C_INT, 0, 0, _siz, 0, 0);

                            myref = add_cmdref ( C_SLASH,
                                                 myref, v16,  /* lft, rght*/
                                                 0,            /* cmdval */
                                                 myref->_cline,
                                                 myref->_lpos);
                        }
                    }

                    _siz = INTSIZ;
                    _t_arrmbrs = 0;
                    _tftyp = FT_INT;
                    break;
                }
L1d92:
                dflt_cast (right_ref);

                if ( ! isintegral (right_ref->ft_Ty))        /* else L1dbf */
                {
                    notintegral (right_ref);
                    resetcmdref (right_ref);
                }

                do_cast (right_ref, FT_INT);

                myref->cr_Right = mult_const (  _siz,
                                                _tftyp,
                                                _t_arrmbrs,
                                                right_ref
                                              );
                _t_cr_18 = left_ref->__cr18 + right_ref->__cr18;
            }
            else
            {
                _tftyp = cast_alike (left_ref, right_ref);  /* copy of L2006 */
                /*goto L2170;*/
            }
            
            break;
        case C_EQUAL:       /* L1df9 */
            ckif_lvalue (left_ref, 0);
            ck_declared (right_ref);
            fty_right = get_fttyp (right_ref);

            _tftyp = left_ref->ft_Ty;

            if (  (ispointer (_tftyp))    &&
                 !(ispointer (fty_right)) &&
                 !(isintegral (fty_right))  )
            {
                goto type_err;
            }

            if ( !(ispointer (fty_right))  ||                  /* L1e3f */
                  (ispointer (_tftyp))     ||
                  (isintegral (_tftyp))      )                 /* else L1ebf */
            {
                do_cast (right_ref, _tftyp);                    /* L1e62 */
                goto L1f54;
            }
            else
            {
                goto type_err;
            }

L1e76:
            ckif_lvalue (left_ref, 0);
            dflt_cast (right_ref);
            fty_right = get_fttyp (right_ref);
            
            if (ispointer ((_tftyp = left_ref->ft_Ty)))
            {
                switch (realvrtyp)
                {
                    case C_MINEQU:       /* L1eab */
                    case C_PLUSEQ:
                        do_cast (right_ref, FT_INT);
                        goto L1ef5;
                    default:     /* L1ebf */
                        goto type_err;
                }
            }

            switch (realvrtyp)
            {
                default:     /* L1ed8 */
                    if (_tftyp == FT_FLOAT)
                    {
                        do_cast (right_ref, FT_DOUBLE);
                    }
                    else
                    {
                        do_cast (right_ref, _tftyp);
                    }
                case C_LSHEQ:    /* L1ef8 */
                case C_RSH_EQ:
L1ef5:
                    myref->vartyp = realvrtyp - 80;  /* make simple *SH */
                    myref = L1587 (myref);        /* call self */

                    if (_tftyp == FT_CHAR)    /* else L1f2f */
                    {
                        myref->cr_Left = left_ref->cr_Left;
                        mk_lftcmd (left_ref);
                        left_ref = myref->cr_Left;
                        _tftyp = FT_INT;
                    }

                    if ((realvrtyp - 80) == myref->vartyp)   /* else L1f57 */
                    {
                        myref->vartyp = realvrtyp; /* restore to *EQ if valid */
                    }
            }

L1f54:
            _siz = left_ref->varsize;
            _t_cr_18 = left_ref->__cr18 + right_ref->__cr18;
            _t_arrmbrs = left_ref->arrdefs;
            break;
            
            /*goto L2170;*/     /* L1ebf */
type_err:
            err_lin (myref, "type mismatch");  /* go to L2049 */
            break;
        case C_COLON:       /* L1f7f */
            if (ispointer ((_tftyp = left_ref->ft_Ty)))    /* else L1fde */
            {
                _siz = left_ref->varsize;
                _t_cr_18 = left_ref->__cr18;
                _t_arrmbrs = left_ref->varsize;

                if (ispointer (right_ref->ft_Ty))       /* else L1fd1 */
                {
                    if (  (_tftyp != right_ref->ft_Ty)           ||
                          (left_ref->varsize != right_ref->varsize)   )
                    {
                        err_lin (myref, "pointer mismatch");
                        /*break;*/
                    }
                    else
                    {
                        _t_arrmbrs = 0;
                    }

                    break;
                }
                
                cktsttyp (right_ref);   /* Insure that "? :" pair match */
                break;
            }

            if (ispointer ((_tftyp = right_ref->ft_Ty))) /*L1fde  else L2006 */
            {
                _siz = right_ref->varsize;
                _t_cr_18 = right_ref->__cr18;
                _t_arrmbrs = right_ref->arrdefs;
                cktsttyp (left_ref);
                break;
            }

            _tftyp = cast_alike (left_ref, right_ref);     /* L2006 */
            break;
        case C_QUESTION:    /* L2017 */
            _tftyp = right_ref->ft_Ty;
            _siz = right_ref->varsize;
            _t_arrmbrs = right_ref->arrdefs;
            break;
        default:            /* L202d */
            if (realvrtyp >= C_PLUSEQ)
            {
                goto L1e76;
            }

            comperr ( myref, "type check");
            break;
    }

    myref->ft_Ty = _tftyp;       /* L2170 */
    myref->varsize = _siz;
    myref->__cr18 = _t_cr_18;
    myref->arrdefs = _t_arrmbrs;
    return myref;
}

/* *********************************************************** *
 * cktsttyp () - Checks that 2nd & 3rd expressions of a        *
 *      "? ... :" term is of correct type.                     *
 * *********************************************************** */

static void
#ifdef __STDC__
cktsttyp (register CMDREF *regptr)
#else
cktsttyp (regptr)
register CMDREF *regptr;
#endif
{
    if ((regptr->vartyp != C_INT) || (regptr->cmdval))
    {
        err_lin (regptr, "should be NULL");
        regptr->vartyp = C_INT;        /* Store correct value */
        regptr->cmdval = 0;
    }
}

/* ******************************************************** *
 * dflt_cast () - Checks that the type is some numeric type *
 *         Promotes "char" to "int" and "float" to double   *
 * Returns: ft_Ty (modified if applicable)                  *
 * ******************************************************** */

static int
#ifdef __STDC__
dflt_cast (register CMDREF *c_ref)
#else
dflt_cast (c_ref)
    register CMDREF *c_ref;
#endif
{
    int _ttype;

    ck_declared (c_ref);
    
    switch (_ttype = c_ref->ft_Ty)
    {
        case FT_CHAR:      /* L21dc */
            _ttype = FT_INT;
            do_cast (c_ref, _ttype);
            break;
        case FT_FLOAT:      /* L21e1 */
            _ttype = FT_DOUBLE;
            do_cast (c_ref, _ttype);
            break;
        case FT_DOUBLE:      /* L2224 */
        case FT_LONG:
        case FT_INT:
        case FT_UNSIGNED:
            break;
        default:     /* L21f0 */
            err_lin (c_ref, "type error");
            _ttype = FT_INT;
            break;
    }

    return (c_ref->ft_Ty = _ttype);
}

/* ******************************************************** *
 * do_cast () - Performs casts                              *
 * Passed: (1) CMDREF * to variable to be cast              *
 *         (2) to_type - the type to convert to             *
 * ******************************************************** */

int
#ifdef __STDC__
do_cast (register CMDREF *ptr, int to_typ)
#else
do_cast (ptr, to_typ)
    register CMDREF *ptr;
    int to_typ;
#endif
{
    /* 4 bytes stack */
    void *_valptr;
    int _cast_type;

    _cast_type = 0;

    switch (ptr->ft_Ty)
    {
        case FT_CHAR:      /* L2241 */
            switch (to_typ)
            {
                case FT_INT:      /* _434 (L2245) */
                case FT_UNSIGNED:
                    _cast_type = C_CHR2INT;           /* 0x85 */
                    break;
                case FT_LONG:      /* L224a */
                    do_cast (ptr, FT_INT);
                    _cast_type = C_I2LNG;
                    break;
                case FT_DOUBLE:      /* _437  (L225d) */
                case FT_FLOAT:
                    do_cast (ptr, FT_INT);    /* First convert char to int */
                    do_cast (ptr, to_typ);
                    break;
                default:
                    break;
            }

            break;
        default:
            if ( ! (ispointer (ptr->ft_Ty)))
            {
                break;
            }
            
            /* else fall through to case FT_INT */
        
        case FT_INT:      /* _442 (L22a4) */
            switch (to_typ)
            {
                case FT_LONG:      /* L22a9 */
                    if ((ptr->vartyp == C_INT))     /* else L22e4 */
                    {
#ifdef COCO
                        /* NOTE:  We need to change this..  _valptr is a long
                         * here, so offsets are offsets into the long.
                         */

                        _valptr = addmem (sizeof (long));
                        
                        /* sign extend into MSB */

                        if ((((int *)_valptr)[1] = ptr->cmdval) < 0)
                        {
                            ((int *)_valptr)[0] = -1;
                        }
                        else
                        {
                            ((int *)_valptr)[0]= 0;
                        }

                        ptr->cmdval = _valptr;
#endif
flag_long:
                        ptr->vartyp = C_LONG;
                        ptr->varsize = LONGSIZ;
                    }
                    else
                    {
                        _cast_type = C_I2LNG;
                    }

                    break;
                case FT_CHAR:      /* L22e9 */
                    to_typ = FT_INT;
                    break;
                case FT_FLOAT:      /* L22f1 */
                    do_cast (ptr, FT_DOUBLE);
                    _cast_type = C_TOFLOAT;
                    break;
                case FT_DOUBLE:      /* L2302 */
                    if (ptr->vartyp == C_INT)      /* else L2334 */
                    {
#ifdef COCO
                        _valptr = addmem (sizeof (double));
                        *(double *)_valptr = (int)(ptr->cmdval);
#else
                        char _tint[20];

                        _valptr = addmem (sizeof (struct dbltree));
                        ((struct dbltree *)_valptr)->native =
                                                    (int)(ptr->cmdval);
                        sprintf (_tint, "%d", (int)(ptr->cmdval));
                        cocodbl (_tint, ((struct dbltree *)_valptr)->cocoarr);
#endif

flag_dbl:
                        ptr->cmdval = (int)_valptr;
                        ptr->vartyp = C_DOUBLE;
                        ptr->varsize = DBLSIZ;
                        break;
                    }

                    _cast_type = C_I2DBL;
                    break;
            }

            break;
        case FT_UNSIGNED:      /* L2357 */
            switch (to_typ)
            {
                case FT_LONG:      /* L235b */
                    _cast_type = C_U2LNG;
                    break;
                case FT_FLOAT:      /* L2360 */
                    do_cast (ptr, FT_DOUBLE);
                    _cast_type = C_TOFLOAT;
                    break;
                case FT_DOUBLE:      /* L2371 */
                    _cast_type = C_U2DBL;
                    break;
                case FT_CHAR:      /* L2379 */
                    to_typ = FT_INT;
                    break;
            }

            break;
        case FT_LONG:      /* L2398 */
            switch (to_typ)
            {

                default:     /* L239d */
                    if ( ! ispointer (to_typ))
                    {
                        break;
                    }
                    /* Fall through to next case */
                case FT_INT:      /* _477 (L23aa) */
                case FT_UNSIGNED:
                case FT_CHAR:
                    if (ptr->vartyp == C_LONG)
                    {
                        /* Discard MSB of long */
                        _valptr = (void *)(ptr->cmdval);
#ifdef COCO
                        ptr->cmdval = ((int *)_valptr)[1];
#endif
flag_int:
                        ptr->vartyp = C_INT;
                        ptr->varsize = INTSIZ;
                    }
                    else
                    {
                        _cast_type = C_LNG2INT;
                    }
                    
                    break;
                case FT_FLOAT:      /* L23d2 */
                    do_cast (ptr, FT_DOUBLE);
                    _cast_type = C_TOFLOAT;
                    break;
                case FT_DOUBLE:      /* L23e3 */
                    if (ptr->vartyp == C_LONG)
                    {
#ifdef COCO
                        _valptr = addmem (sizeof (double));
                        *((double *)_valptr) = *((long *)(ptr->cmdval));
#else
                        char _tval[50];

                        _valptr = addmem (sizeof (struct dbltree));
                        ((struct dbltree *)_valptr)->native =
                                                        (long)(ptr->cmdval);
                        sprintf (_tval, "%d", (int)(ptr->cmdval));
                        cocodbl (_tval, ((struct dbltree *)_valptr)->cocoarr);
#endif
                        goto flag_dbl;
                    }

                    _cast_type = C_L2DBL;       /* L2408 */
                    break;
            }

            break;
        case FT_FLOAT:      /* L2432 */
            switch (to_typ)
            {
                case FT_LONG:      /* _496 (L2436) */
                case FT_UNSIGNED:
                case FT_CHAR:
                case FT_INT:
                    do_cast (ptr, FT_DOUBLE);
                    do_cast (ptr, to_typ);
                    break;
                case FT_DOUBLE:      /* L2450 */
                    _cast_type = C_FLT2DBL;
                    break;
            }

            break;
        case FT_DOUBLE:      /* L247a */
            switch (to_typ)
            {
                case FT_CHAR:      /* _504 (L247e) */
                case FT_UNSIGNED:
                case FT_INT:
                    if (ptr->vartyp == C_DOUBLE)        /* else L2492 */
                    {
#ifdef COCO
                        ptr->cmdval = (int)(*((double *)(ptr->cmdval)));
#else
                        ptr->cmdval = (int)(((struct dbltree *)
                                                    (ptr->cmdval))->native);
#endif
                        goto flag_int;
                    }
                    else
                    {
                        _cast_type = C_DBL2INT;
                    }

                    break;
                case FT_LONG:      /* L2497 */
                    if (ptr->vartyp == C_DOUBLE)        /* else L24b0 */
                    {
#ifdef COCO
                        ptr->cmdval = (long *)(*((double *)(ptr->cmdval)));
#else
                        ptr->cmdval = (((struct dbltree *)
                                                    (ptr->cmdval))->native);
#endif
                        goto flag_long;
                    }
                    else
                    {
                        _cast_type = C_DBL2LNG;
                    }

                    break;
                    
                case FT_FLOAT:      /* L24b5 */
                    _cast_type = C_TOFLOAT;

                    break;
            }
    }

    /* _429 (L2508) */
    if (_cast_type)     /* else L2540 */
    {
        CmdrefCpy (ptr, (_valptr = add_cmdref (0, 0, 0, 0, 0, 0)));
        ptr->vartyp = _cast_type;
        ptr->cr_Left = _valptr;
        ptr->cr_Right = 0;
    }

    return (ptr->ft_Ty = to_typ);
}

/* ******************************************************************** *
 * cast_alike () - casts first variable to FT_Ty of second variable     *
 *      If both are of same type, of course, there is no change         *
 * Returns: resulting FT_Ty of two vars                                 *
 * ******************************************************************** */

static int
#ifdef __STDC__
cast_alike (register CMDREF *cref_1, CMDREF *cref_2)
#else
cast_alike (cref_1, cref_2)
    register CMDREF *cref_1;
CMDREF *cref_2;
#endif
{
    int ref1_ty;
    int ref2_ty;

    ref1_ty = dflt_cast (cref_1);
    ref2_ty = dflt_cast (cref_2);

    if (ref1_ty == FT_DOUBLE)
    {
        return (do_cast (cref_2, FT_DOUBLE));
    }
    else
    {
        if (ref2_ty == FT_DOUBLE)
        {
            return (do_cast (cref_1, FT_DOUBLE));
        }
        else
        {
            if (ref1_ty == FT_LONG)
            {
                return (do_cast (cref_2, FT_LONG));
            }
            else
            {
                if (ref2_ty == FT_LONG)
                {
                    return (do_cast (cref_1, FT_LONG));
                }
                else
                {
                    if ((ref1_ty == FT_UNSIGNED) || (ref2_ty == FT_UNSIGNED))
                    {
                        return (cref_1->ft_Ty = cref_2->ft_Ty = FT_UNSIGNED);
                    }
                    else
                    {
                        return FT_INT;
                    }
                }
            }
        }
    }

    /*return 1;*/
}

/* ************************************************************************ *
 * mult_const() - Multiplies a constant by value of myref                   *
 * This may be strictly for pointers                                        *
 * ************************************************************************ */

static CMDREF *
#ifdef __STDC__
mult_const ( int myconst,
             int gntyp,
             struct brktdef *bracdf,
             CMDREF *myref)
#else
mult_const (myconst, gntyp, bracdf, myref)
    int myconst;
    int gntyp;
    struct brktdef *bracdf;                 /*arrmbrs*/
    CMDREF *myref;
#endif
{
    register CMDREF *tmpref;

    /* If multiplying by 1, return myref unchanged */

    if ((myconst = get_varsize (MSBrshft2 (gntyp), myconst, bracdf)) == 1)
    {
        return myref;
    }

    /* L25f3 */
    /* create new cmdref for INT  This will be the cr_Right */

    tmpref = add_cmdref ( C_INT,
                          0,        /* no cr_Left  */
                          0,        /* no cr_Right */
                          myconst,    /* cmdval */
                          myref->_cline, myref->_lpos );
    tmpref->ft_Ty = FT_INT;
    
    /* create new cmdref for MULT Then multiply the original
     * cref with the above const
     */
    
    tmpref = calcconsts ( add_cmdref ( C_MULT,
                                       myref,       /* cr_Left  */
                                       tmpref,      /* cr_Right */
                                       0,           /* cmdval   */
                                       myref->_cline,
                                       myref->_lpos)
                        );

    if (tmpref->vartyp == C_INT)
    {
        tmpref->__cr18 = 0;
    }
    else
    {
        tmpref->__cr18 = 2;
    }

    tmpref->ft_Ty = FT_INT;     /* L2657 */
    tmpref->varsize = INTSIZ;

    return tmpref;
}

/* **************************************************************** *
 * ckif_lvalue () - Checks if the vartyp requires an lvalue and     *
 *          warns if needed.                                        *
 * Returns: Nothing                                                 *
 * **************************************************************** */

static void
#ifdef __STDC__
ckif_lvalue ( register CMDREF *regptr,
              int ref_ordot
            )
#else
ckif_lvalue (regptr, ref_ordot)
    register CMDREF *regptr;
    int ref_ordot;
#endif
{
    int _vtyp;

    switch (_vtyp = regptr->vartyp)
    {
        case C_REG_Y:         /* L27b6 */
        case C_REG_U:
        case C_ASTERISK:
            return;
    }

    if (_vtyp == C_USRLBL)       /* else L26c1 ("lvalue required") */
    {
        ck_declared (regptr);

        /* If it's a reference or array/struct element, it's OK */

        if (ref_ordot)
        {
            return;
        }

        /* The original code had some extra stuff.  I've checked this
         * and I'm _sure_ this is equivalent
         */

        /* Anything but an array or structure is permissible */

        if (( ! (inbrkets (regptr->ft_Ty))) && (regptr->ft_Ty != FT_STRUCT))
        {
            return;
        }
    }

    err_lin (regptr, "lvalue required");        /* L26c1 */
    resetcmdref (regptr);

}

void
#ifdef __STDC__
ck_declared (register CMDREF *regptr)
#else
ck_declared (regptr)
    register CMDREF *regptr;
#endif
{
    if ((regptr->vartyp == C_USRLBL) && ! (regptr->ft_Ty))    /* else L27b8 */
    {
        err_lin (regptr, "undeclared variable");
        resetcmdref (regptr);
    }
}

/* ******************************************************************** *
 * membrsize () - Update a struct/union member                          *
 *      If it's an address (pointer), copy Left ref to current CREF     *
 *      Else ifs it's a USRLBL, set vartyp, ft_Ty to "int", cmdval to   *
 *        label number, and __cr18 = 0                                  *
 *      In any other case, error, reset CMDREF members to default       *
 * Returns: size of struct/union member                                 *
 * ******************************************************************** */

static int
#ifdef __STDC__
membrsize (register CMDREF *mycref, int *ftyp, int *is_ref)
#else
membrsize (mycref, ftyp, is_ref)
    register CMDREF *mycref;
    int *ftyp;
    int *is_ref;
#endif
{
    CMDREF *leftref;

    *ftyp = mycref->ft_Ty;

    if ((mycref->vartyp) == C_AMPERSAND)       /* else L2746 */
    {
        *is_ref = 1;

        /* Copy mycref's Left ref to mycref, keeping mycref's
         * own arrdefs */

        leftref = mycref->cr_Left;
        leftref->arrdefs = mycref->arrdefs;
        CmdrefCpy (leftref, mycref);
        mk_lftcmd (leftref);
    }
    else
    {
        *is_ref = 0;
    }

    ck_declared (mycref);

    if (mycref->vartyp == C_USRLBL)       /* else L278b */
    {
#ifndef COCO
        LBLDEF *_mylbl;     /* To make things clearer */
#else
#   define _mylbl leftref
#endif
        _mylbl = (LBLDEF *)(mycref->cmdval);

        if (_mylbl->fnccode != FT_STRCMBR)   /* else L276c */
        {
            goto needstruct;
        }

        mycref->vartyp = C_INT;
        mycref->cmdval = _mylbl->lbl_nbr;   /* ?????? */
        mycref->__cr18 = 0;
        mycref->ft_Ty = FT_INT;
        return _mylbl->vsize;
#ifdef COCO
#   undef _mylbl
#endif
    }

needstruct:
    err_lin (mycref, "struct member required");
    resetcmdref (mycref);
    mycref->vartyp = C_INT;
    mycref->cmdval = 0;
    mycref->__cr18 = 0;
    *ftyp = FT_INT;
    return INTSIZ;
}

int
#ifdef __STDC__
get_fttyp(register CMDREF *regptr)
#else
get_fttyp(regptr)
    register CMDREF *regptr;
#endif
{
    if ((regptr->ft_Ty == FT_STRUCT) || (regptr->ft_Ty == FT_UNION))
    {
        err_lin (regptr, "structure or union inappropriate");
        resetcmdref (regptr);
    }

    return regptr->ft_Ty;
}

/* ************************************************************ *
 * resetcmdref () - Reset data in a CMDREF after an error       *
 *                                                              *
 * ************************************************************ */

void
#ifdef __STDC__
resetcmdref (register CMDREF *err_ref)
#else
resetcmdref (err_ref)
    register CMDREF *err_ref;
#endif
{
#ifdef COCO
    mem_cp (dflt_cref, err_ref,
            sizeof (err_ref->ft_Ty) + sizeof (err_ref->varsize) +
                    sizeof (err_ref->arrdefs));
#else
    memcpy (err_ref, &dflt_cref,
            sizeof (err_ref->ft_Ty) + sizeof (err_ref->varsize) +
                    sizeof (err_ref->arrdefs));
#endif
    err_ref->__cr18 = 1;
    CCREFtoLeft (err_ref->cr_Left);
    CCREFtoLeft (err_ref->cr_Right);
    err_ref->cr_Left = err_ref->cr_Right = 0;
    err_ref->vartyp = C_USRLBL;    /* generic alpha??? */
#ifdef COCO
    err_ref->cmdval = dflt_cref;
#else
    err_ref->cmdval = (int)(&dflt_cref);
#endif
}

/* ************************************************************ *
 * isintegral() - checks that a command is an integral type.    *
 * Returns: TRUE if it is integral                              *
 *          FALSE on anything else                              *
 * ************************************************************ */

int
#ifdef __STDC__
isintegral (int tstval)
#else
isintegral (tstval)
    int tstval;
#endif
{
    switch (tstval)
    {
        case FT_INT:
        case FT_CHAR:
        case FT_LONG:
        case FT_UNSIGNED:
            return 1;
    }

    return 0;
}

void
#ifdef __STDC__
notintegral (CMDREF *c_ref)
#else
notintegral (c_ref)
    CMDREF *c_ref;
#endif
{
    err_lin (c_ref, "must be integral");
}
