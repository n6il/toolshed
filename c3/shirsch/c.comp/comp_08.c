/* **************************************************************** *
 * ccomp_08.c                                                       *
 * from p1_06.c                                                     *
 *                                                                  *
 *  $Id:: comp_08.c 73 2008-10-03 20:20:29Z dlb                   $ *
 * **************************************************************** */


#ifndef COCO
#  ifndef OSK
#   ifndef direct
#       define direct
#   endif
#  endif
#endif

/*direct int StrsCtrl,
           D005a;*/

#include "ccomp.h"

static void rsrvnulls (
#ifdef __STDC__
    int
#endif
);

static int prinitarr (
#ifdef __STDC__
        int ftType, LBLDEF *regptr, struct brktdef *p3, int readflg
#endif
);

static int prtconst (
#ifdef __STDC__
    int
#endif
);

static void prtnumarray (
#ifdef __STDC__
    void *, int
#endif
);

static void find_sep ();

/* ******************************************************************** *
 * signextend () - Returns the sign-extended value for the number       *
 *                 passed as a parameter                                *
 * Passed:  (1) Value to extend                                         *
 *          (2) COCO size of the value in bytes                         *
 *                1 for char, 2 for int                                 *
 * Returns: The value with sign extended                                *
 * ******************************************************************** */

#ifndef COCO
int
#ifdef __STDC__
signextend (int val, int varsiz)
#else
signextend (val, varsiz)
    int val, varsiz;
#endif
{
#ifndef _OSK
    int _mask[2] = {0xff, 0xffff};
#else
    /* OSK cannot preinitialize static variables ??? */
    int _mask[2];

    _mask[0] = 0xff;
    _mask[1] = 0xffff;
#endif

    if (val & (0x80 << ((varsiz - 1) * 8)))
    {
        return ( val | (-1 ^ _mask[varsiz-1]));
    }
    else
    {
        return val;
    }
}
#endif

/* **************************************************************** *
 * do_initvar () - Process a label name when there is an assignment *
 * Passed : (1) - LBLDEF * lblstruct (for the label)                *
 *          (2) - FT_type for label                                 *
 *          (3) - gntyp - the gentype for the label                 *
 * **************************************************************** */

void
#ifdef __STDC__
do_initvar (register LBLDEF *lbldf, int _ftTyp, int gntyp)
#else
do_initvar (lbldf, _ftTyp, gntyp)
    register LBLDEF *lbldf;
    int _ftTyp;
    int gntyp;
#endif
{
    void *_lbl_nam; /* Also char * */
    int strnulls;   /* Remainder of str after init data */

    /* print appropriate vsec */

    switch (_ftTyp)
    {
        case FT_TYPEDEF:        /* L3cc7 */
        case FT_EXTERN:
        case FT_DPXTRN:
            cant_init ();
            return;
        case FT_DIRECT:         /* DP data */       /* L3ccd */
        case FT_DPSTATIC:
            prt_vsect (1);
            break;
        default:                /* non-dp data */   /* L3cd2 */
            prt_vsect (0);
            break;
    }

    /* Write label name to output stream */

    if ( ! InFunction)     /* L3cfe */ /* else L3d30 */
    {
        _lbl_nam = lbldf->fnam;

        if ((_ftTyp != FT_STATIC) && (_ftTyp != FT_DPSTATIC))
        {
            prt_label (_lbl_nam, 1);    /* with colon */    /* Global label */
        }
        else            /* Static variables */
        {
            lbldf->fnccode = FT_NONDPDAT;      /* _16 */
            prt_label (_lbl_nam, 0);        /* Static (no colon) */
        }
    }
    else
    {
        prt_loclbl (lbldf->lbl_nbr);
        prntCR ();
    }

    /* FIXME : gntyp not FT_DPXTRN */
    if (gntyp == FT_DPXTRN)   /* _19 */    /* L3d3c */     /* else L3d90 */
    {
        StrsCtrl = 1;
        nxt_word ();

        if (D003f == C_DQUOT)   /* String data */   /* else L3d89 */
        {
            _lbl_nam = lbldf->arrmbrs;

            if (*((int *)_lbl_nam) == 0)        /* else L3d64 */
            {
                *((int *)_lbl_nam) = StrTotLngth;
            }
            else
            {
                if ((strnulls = *((int *)_lbl_nam) - StrTotLngth) >= 0)
                {                   /* L3d64 */
                    prnt_rzb (strnulls);
                }
                else
                {
                    reprterr ("too long");
                }
            }

            nxt_word ();        /* L3d81 */
            goto do_endsct;
        }

        StrsCtrl = 2;      /* L3d89 */
    }
    else        /* Not DPEXTERN */
    {
        StrsCtrl = 2;      /* _21 */   /* L3d90 */
        nxt_word ();
    }

    if (gntyp == FT_STRUCT)    /* else L3db5 */
    {
        prinitarr (gntyp, lbldf, (struct brktdef *)lbldf->mbrlist, 0);
                    /* go to L3de7 */
    }
    else
    {
        if (inbrkets (gntyp))      /* L3db5 */
        {
            prinitarr ( gntyp, lbldf, lbldf->arrmbrs, 0);
        }
        else
        {
            if ( ! (prinitarr (gntyp, lbldf, lbldf->arrmbrs, 1)))
            {
                find_sep ();    /* Look for comma or ";" */
            }
        }
    }

do_endsct:
    prt_endsect ();
    StrsCtrl = 0;
}

/* ******************************************************************** *
 * prinitarr () - Prints init values for arrays or structures           *
 * Passed:  (1) - base FT_Type                                          *
 *          (2) _ ptr to the LBLDEF                                     *
 *          (3) - ptr to member list (either brktdef or memberdef       *
 *          (4) - Flag - nonzero if we are inside braces                *
 * ******************************************************************** */

static int
#ifdef __STDC__
prinitarr ( int ftType,
             register LBLDEF *lbldf,
             struct brktdef *p3,   /* or memberdef for structs */
             int inbracz)
#else
prinitarr (ftType, lbldf, p3, inbracz)
    int ftType;
    register LBLDEF *lbldf;
    struct brktdef *p3;
    int inbracz;
#endif
{
    int _inbrkts;
    int _curftyp;
    unsigned int n_entries;
    struct brktdef *curntbrace;
    unsigned int maxent;
    struct memberdef *_mbrpt;

    if ( ! inbracz)
    {
        lookfor (C_LBRACE);
    }
    else
    {
        if (D003f == C_LBRACE)
        {
            _inbrkts = 1;
            nxt_word ();
        }
        else
        {
            _inbrkts = 0;
        }
    }

    if (inbrkets (ftType))  /* L3e25 */     /* else L3f13 */
    {
        /* Array initialization */

        if ( ! (maxent = p3->elcount))
        {
            maxent = -1;
        }

        if ((_curftyp = MSBrshft2 (ftType)) == FT_STRUCT)     /* L3e3f */
        {
            curntbrace = (struct brktdef *)lbldf->mbrlist;
        }
        else
        {
            curntbrace = p3->brPrev;     /* L3e55 */
        }

        n_entries = 0;

        /* Process until a right bracket is found, max elements defined
         * or non-comma is encoutered
         */

        while (D003f != C_RBRACE)     /* L3e98 */
        {
                                /*L3e62*/
            if (prinitarr (_curftyp, lbldf, curntbrace, (inbracz + 1)))
            {                                               /*else L3fca*/
                if ((++n_entries >= maxent) || (D003f != C_COMMA))
                {
                    break;
                }

                nxt_word ();
                continue;   /* useless, but trying to match their code */
            }
            else
            {
                return 0;
            }
        }

        if (maxent == -1)
        {
            p3->elcount = n_entries;
        }
        else
        {
            /* If less than full array has been initialized, fill
             * remainder with nulls
             */

            if (n_entries < maxent)        /* L3eaf */
            {
                rsrvnulls (get_varsize (_curftyp, lbldf->vsize, p3->brPrev) *
                (maxent - n_entries));
            }
        }

L3edc:
        if ((inbracz) && ( ! _inbrkts))
        {
            return 1;
        }

        if (D003f == C_COMMA)
        {
            nxt_word ();
        }

        if (D003f == C_RBRACE)
        {
            nxt_word ();
            return 1;
        }

        reprterr ("too many elements");
        find_sep ();
        return 1;
    }

    /* Here, we need to change defs for p3, and some vars,
     * since we are now dealing with structs rather than braces */

    /* L3f13 */
    if (ftType == FT_STRUCT)        /* else L3f9e */
    {
        if ( ! (_mbrpt = (struct memberdef *)p3))
        {                           /* else L3f6c */
            reprterr ("unions not allowed");
            find_sep ();
            return 0;
        }

        while (D003f != C_RBRACE)
        {
            lbldf = _mbrpt->mmbrlbl;    /* L3f2c */

            if ( ! prinitarr (lbldf->gentyp, lbldf,
                            (lbldf->gentyp == FT_STRUCT ?
                                          (struct brktdef *)(lbldf->mbrlist) :
                                          (lbldf->arrmbrs)),
                            (inbracz + 1)))
            {
                return 0;
            }

            if ((_mbrpt = _mbrpt->mbrPrev) && (D003f == C_COMMA))
            {       /* else L3f95 */
                nxt_word ();
                continue;       /* not necessary, really */
            }
            else
            {
                break;
            }
        }

        while (_mbrpt)      /* L3f95 */
        {
            lbldf = _mbrpt->mmbrlbl;
            rsrvnulls ( get_varsize ( lbldf->gentyp,
                                      lbldf->vsize,
                                      lbldf->arrmbrs)
                      );
            _mbrpt = _mbrpt->mbrPrev;
        }

        goto L3edc;
    }

    if (prtconst (ftType))     /* L3f9e */     /* else L3fbc */
    {
        if (_inbrkts)        /* else L3fb7 */
        {
            lookfor (C_RBRACE);
        }

        return 1;
    }
    else
    {
        reprterr ("constant expression required");
        find_sep ();
        return 0;
    }
}

static void
#ifdef __STDC__
rsrvnulls (int nulcount)
#else
rsrvnulls (nulcount)
    int nulcount;
#endif
{
    prt_bgnfld ("rzb ");
    prnt_integer (nulcount);
    prntCR();
}

/* ************************************************************************ *
 * prtconst () - Output constant value                                      *
 * Passed : FT_TY                                                           *
 * Returns: Non-Zero if value is a constant, NULL if not a constant         *
 * ************************************************************************ */

static int
#ifdef __STDC__
prtconst (int fty)
#else
prtconst (fty)
    int fty;
#endif
{
    CMDREF *v6;
    int nval;
    int _isconst;
    int t_ftTy;
    register CMDREF *_cref;

    if ( ! (_cref = L111d (get_operand (FT_CHAR))))   /* must be char or int */
    {
        return 0;
    }

    nval = 0;
    _isconst = 1;
    t_ftTy = _cref->ft_Ty;

    if (ispointer (fty))        /* else L4054 */
    {
        switch (t_ftTy)
        {
            case FT_LONG:      /* L4031 */
                do_cast (_cref, FT_INT);   /* do this at L407d */
                break;      /* jump to L4086 */
            default:     /* L4036 */
                if ( ! ispointer (t_ftTy))
                {
                    goto not_const;
                }
            case FT_INT:      /* L4086 */
            case FT_UNSIGNED:
                break;
        }
    }
    else    /* L4054 */
    {
        if (ispointer (t_ftTy))    /* else L406e */
        {
            if ( ! isintegral (fty))      /* else L4086 */
            {
                goto not_const;
            }
        }
        else        /* Not a pointer */
        {           /* L406e */
            do_cast (_cref, ((fty == FT_FLOAT) ? FT_DOUBLE : fty));
        }
    }

    /* L4086 here */
    
    if ((_cref->vartyp == C_PLUS) || (_cref->vartyp == C_NEG))
    {                       /* else L40df */
        
        if ((v6 = _cref->cr_Right)->vartyp != C_INT)    /* else L40b1 */
        {
not_const:
            _isconst = 0;
            goto const_ret;
        }

        if (_cref->vartyp == C_NEG)
        {
            nval = -(v6->cmdval);
        }
        else
        {
            nval = v6->cmdval;       /* L40c7 */
        }

        CCREFtoLeft (v6);
        v6 = _cref;
        _cref = _cref->cr_Left;
        mk_lftcmd (v6);
    }

    /* L40df */

    if (_cref->vartyp == C_AMPERSAND)   /* else L4143 */
    {
        switch (((CMDREF *)((v6 = _cref->cr_Left)->cmdval))->cmdval)
        {
            default:     /* L40f5 */
                _isconst = 0;
                break;      /* go to L4192 */
            case FT_STATIC:
            case FT_EXTERN:        /* L40fc */
            case FT_NONDPDAT:
            case FT_DIRECT:
            case FT_DPXTRN:
                prnt_fdb ();
                D005a = 1;  /* Allow preceding operand with ">" or "<" */
                /* v6 is really a CMDREF * here, but to satisfy prototype,
                 * cast to int
                 */
                prt_rgofst (119, (int)v6, nval);
                D005a = 0;
                prntCR ();
                break;      /* go to L4192 */
        }
    }
    else
    {       /* L4143 */
        switch (_cref->vartyp)
        {
            default:     /* L40f5 */
                _isconst = 0;
                break;
            case C_DOUBLE:     /* L4147 */
                if (fty == FT_FLOAT)    /* else L415b */
                {
                    *(float *)(_cref->cmdval) = *((double *)(_cref->cmdval));
                }

            case C_INT:     /* L415b */  /* '6' */
            case C_LONG:
                    prtnumarray ((void *)(_cref->cmdval), fty);

                    break;
                case C_DQUOT:     /* L4169 */  /* '7' */
                    prnt_fdb ();
                    loclbl_CR (_cref->cmdval);
                    break;
            }
        }

const_ret:
        CCREFtoLeft (_cref);
        return _isconst;
}

static void
#ifdef __STDC__
prtnumarray (void *valu, int f_type)
#else
prtnumarray (valu, f_type)
#ifdef COCO
    register
#endif
    void *valu;
    int f_type;
#endif
{
    int _siz;
#ifndef COCO
    int numarray[8];
    int _ofset;
    int *_valptr = (int *)&valu;
#endif

    switch (f_type)
    {
        case FT_CHAR:      /* L41af */
            prnt_fcb ();
#ifdef COCO
            prnt_integer ((int)valu);
#else
            prnt_integer ((int)valu & 0xff);
#endif
            prntCR ();
            return;
        case FT_INT:        /* L41be */
        case FT_UNSIGNED:
        default:
            _siz = INTSIZ/2;
#ifndef COCO
            /* Don't know if this is correct, but it should be...
             * Perhaps it should error out if valu > 0xffff*/

            *_valptr = signextend (*_valptr, 2);
#endif

            break;
        case FT_LONG:      /* L41c3 */
#ifndef COCO
            _siz = LONGSIZ/2;
            numarray[0] = (int)valu >> 16;
            numarray[1] = (int)valu & 0xffff;

                /* Sign extend each element */

            for (_ofset = 0; _ofset < 2; _ofset++)
            {
                numarray[_ofset] = signextend (numarray[_ofset], 2);
            }

            prnt_words (numarray, _siz);
            return;
#endif
        case FT_FLOAT:
            _siz = FLOATSIZ/2;
            break;
        case FT_DOUBLE:      /* L41c8 */
            _siz = DBLSIZ/2;
            break;
    }

    prnt_words (valu, _siz);
}

void
#ifdef __STDC__
cant_init (void)
#else
cant_init ()
#endif
{
    reprterr ("cannot initialize");
    find_sep ();
}

/* ************************************************************ *
 * find_sep() - reads in input stream until ",", ";", or error  *
 * ************************************************************ */

static void
#ifdef __STDC__
find_sep (void)
#else
find_sep ()
#endif
{
    for (;;)
    {
        switch (D003f)
        {
            case C_COMMA:
            case C_SEMICOLON:
            case -1:    /* err */
                return;
            default:
                nxt_word ();
        }
    }
}
