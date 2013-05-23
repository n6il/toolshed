/* ******************************************************** *
 * comp_05.c                                                *
 * Corresponds to p1_04.c                                   *
 * $Id:: comp_05.c 73 2008-10-03 20:20:29Z dlb            $ *
 * ******************************************************** */

#include "ccomp.h"

static LBLDEF *loclvardef ();
static CMDREF *L3319 (
#ifdef __STDC__
    int p1
#endif
);
static CMDREF *L337e ();
static CMDREF *get_condition ();
static void L33d7 (
#ifdef __STDC__
    register CMDREF *regptr, int p2, int p3, int p4
#endif
);
static void do_if ();
static void do_while ();
static void do_swtch ();
static void do_case ();
static void do_deflt ();
static void no_switch ();
static void do_do ();
static void do_for ();
static void do_retrn ();
static void do_break ();
static void do_contin ();
static void do_goto ();
static void gen_goto ();
static CMDREF *L3342 (
#ifdef __STDC__
    CMDREF *
#endif
);

/* ******************************************************************** *
 * do_command () - Process a single command - a line ended by a         *
 *          semicolon.                                                  *
 * ******************************************************************** */

void
#ifdef __STDC__
do_command (void)
#else
do_command ()
#endif
{
    if (D003f != C_SEMICOLON)
    {
        ProgFlow = 0;
    }

donxtwrd:
    switch (D003f)
    {
        case C_SEMICOLON:     /* L2b10 */  /* '(' */
            break;
        case C_BUILTIN:     /* L29f2 */  /* '3' */
            switch (LblVal)
            {
                case FT_IF:     /* L29f6 */
                    do_if ();
                    return;
                case FT_WHILE:     /* L29fb */
                    do_while ();
                    return;
                case FT_RETURN:     /* L2a00 */
                    do_retrn ();
                    break;  /* go to L2b10 */
                case FT_CASE:     /* L2a06 */
                    do_case ();
                    return;
                case FT_SWITCH:     /* L2a0b */
                    do_swtch ();
                    return;
                case FT_BREAK:     /* L2a10 */
                    do_break ();
                    break;      /* go to L2b10 */
                case FT_CONTINUE:     /* L2a16 */
                    do_contin ();
                    break;
                case FT_DEFAULT:     /* L2a1c */
                    do_deflt ();
                    return;
                case FT_FOR:     /* L2a21 */
                    do_for ();
                    return;
                case FT_DO:     /* L2a26 */
                    do_do ();
                    break;
                case FT_GOTO:     /* L2a2c */
                    do_goto ();
                    break;      /* go to L2b10 */
                case FT_ELSE:     /* L2a32 */
                    reprterr ("no 'if' for 'else'");
                    nxt_word ();      /* go to L2a9c */
                    goto donxtwrd;
                default:     /* L2a43 */
                    goto L2aa5;
            }

            break;
        case C_LBRACE:      /* L2a86 */
            do_block ();
            nxt_word ();
            return;
        case -1:            /* L2a8e */
            return;
        case C_USRLBL:      /* L2a90 */
            skipblank ();
            
            if (CurChr == ':')
            {
                gen_goto ();
                goto donxtwrd;
            }
            else
            {
                goto L2acd;
            }
        default:            /* L2aa7 */
L2aa5:
            if (is_sc_specifier () || is_vardef ())
            {
                reprterr ("illegal declaration");
                
                do
                {
                    nxt_word ();
                } while (D003f != C_SEMICOLON);

                break;
            }
            else
            {
L2acd:
                if ( ! (L3319 (0)))
                {
                    reprterr ("syntax error");
                    cmma_rbrkt ();
                    return;
                }
            }

            break;
    }

    lookfor (C_SEMICOLON);     /* L2b10 */
}

static void
#ifdef __STDC__
do_if (void)
#else
do_if ()
#endif
{
    int _lbnumHi;
    int v2;
    CMDREF *v0;
    register int _lbnumLo;

    nxt_word ();
    v0 = L337e ();
    _lbnumLo = ++LblNum;
    _lbnumHi = ++LblNum;

    if (D003f == C_SEMICOLON)    /* else L2b6b */
    {
        nxt_word ();
        L33d7 (v0, _lbnumLo, _lbnumHi, 0);
        loclbl_CR (_lbnumHi);
        v2 = _lbnumLo;
    }
    else
    {
        L33d7 (v0, _lbnumLo, _lbnumHi, 1);
        loclbl_CR (_lbnumLo);
        do_command ();
        v2 = _lbnumHi;
    }

    /* L2b8d */
    if ((D003f == C_BUILTIN) && (LblVal == 21))    /* else L2bd2 */
    {
        nxt_word ();

        if (D003f != C_SEMICOLON)        /* else L2bd2 */
        {
            if (_lbnumLo != v2)   /* else L2bcf */
            {
                gencode (124, (v2 = ++LblNum), 0 NUL1);
                loclbl_CR (_lbnumHi);
            }

            do_command ();
        }
    }

    loclbl_CR (v2);   /* L2bd2 */
}

static void
#ifdef __STDC__
do_while (void)
#else
do_while ()
#endif
{
    int v8;
    int v6;
    int v4;
    int v2;
    CMDREF *v0;
    register int regptr;

    v8 = D0033;
    v6 = D0035;
    v4 = D0786;
    v2 = D0788;
    nxt_word ();
    D0033 = ++LblNum;
    D0786 = D0788 = StkUsed;
    D0035 = ++LblNum;
    v0 = L337e ();

    if (D003f == C_SEMICOLON)
    {
        regptr = D0035;
    }
    else
    {
        gencode (124, D0035, 0 NUL1);
        regptr = ++LblNum;
        loclbl_CR (regptr);
        do_command ();
    }

    loclbl_CR (D0035);       /* L2c52 */
    L33d7 (v0, regptr, D0033, 0);
    loclbl_CR (D0033);
    D0033 = v8;
    D0035 = v6;
    D0786 = v4;
    D0788 = v2;
}

static void
#ifdef __STDC__
do_swtch (void)
#else
do_swtch ()
#endif
{
    int old_D0033;
    struct case_ref *old_caslist;
    int oldD0784;
    int sw_lbnum;
    struct case_ref *nxtcase;
    int oldD0786;
    struct case_ref *_refsav;
    register CMDREF *regptr;
#ifndef COCO
    struct case_ref *__mycase;
#endif

    nxt_word ();
    ++D0037;
    _refsav = CaseNow;
    old_caslist = CaseList;
    CaseList = 0;
    old_D0033 = D0033;
    oldD0784 = D0784;
    oldD0786 = D0786;
    D0033 = ++LblNum;
    D0786 = StkUsed;
    D0784 = 0;

    /* get the case value */

    lookfor (C_LPAREN);
    
    if ((regptr = L111d (get_operand (0))))       /* else L2d43 */
    {
        ck_declared (regptr);
        
        switch (regptr->ft_Ty)
        {
            case FT_CHAR:      /* L2cfd */
            case FT_LONG:
                do_cast (regptr, FT_INT);
                break;
            case FT_INT:      /* L2d33 */
            case FT_UNSIGNED:
                break;
            default:     /* L2d0b */
                notintegral (regptr);
                resetcmdref (regptr);
                break;
        }

        leax_reg (regptr);
        CCREFtoLeft (regptr);
    }
    else
    {
        exprmsng ();    /* L2d43 */
    }

    lookfor (C_RPAREN);
    gencode (124, (sw_lbnum = ++LblNum), 0 NUL1);
    do_command ();

    if ( ! ProgFlow)
    {
        gencode (124, D0033, 0 NUL1);
    }

    loclbl_CR (sw_lbnum);       /* L2d82 */
        /* CoCo will use register regptr for double duty, as CMDREF *
         * and as struct case_ref *, for speed, but for X-compiler,
         * we'll use a separate variable
         */

#ifdef COCO
#   define __mycase regptr
#endif
    __mycase = CaseList;

    while (__mycase)     /* L2dad */
    {
        nxtcase = __mycase->case_nxt;
        gencode (125, __mycase->cas04, __mycase->cas02 NUL1);
        __mycase->case_nxt = D0025;
        D0025 = __mycase;
        __mycase = nxtcase;
    }

    /* done with __mycase, release it */
#ifdef COCO
#   undef __mycase
#endif

    if (D0784)
    {
        gencode (124, D0784, 0 NUL1);
    }

    loclbl_CR (D0033);        /* L2dcb */
    CaseList = old_caslist;
    D0784 = oldD0784;
    --D0037;
    CaseNow = _refsav;
    D0033 = old_D0033;
    D0786 = oldD0786;
}

static void
#ifdef __STDC__
do_case (void)
#else
do_case ()
#endif
{
    int v0;
    register struct case_ref *my_case;

    nxt_word ();
    v0 = getconst (0);
    lookfor (C_COLON);

    if (D0037)          /* else L2e5a */
    {
        if ((my_case = D0025))
        {
            D0025 = my_case->case_nxt;
        }
        else
        {
            my_case = addmem (sizeof (struct case_ref));     /* L2e26 */
        }

        if (CaseList)      /* L2e32 */
        {
            CaseNow->case_nxt = my_case;
        }
        else
        {
            CaseList = my_case;
        }

        CaseNow = my_case;     /* L2e3e */

        /* Fix this!  my_case is a 6-byte structure */
        my_case->case_nxt = 0;
        my_case->cas02 = v0;
        loclbl_CR ((my_case->cas04 = ++LblNum));
    }
    else
    {
        no_switch ();
    }
}

static void
#ifdef __STDC__
do_deflt (void)
#else
do_deflt ()
#endif
{
    nxt_word ();

    if ( ! D0037)
    {
        no_switch ();
    }

    if (D0784)
    {
        reprterr ("multiple defaults");
    }
    else
    {
        loclbl_CR ((D0784 = ++LblNum));
    }

    lookfor (C_COLON);
}

static void
#ifdef __STDC__
no_switch (void)
#else
no_switch ()
#endif
{
    reprterr ("no switch statement");
}

static void
#ifdef __STDC__
do_do (void)
#else
do_do ()
#endif
{
    int v8;
    int v6;
    int v4;
    int v2;
    int v0;

    v8 = D0033;
    v6 = D0035;
    v0 = D0788;
    v2 = D0786;
    D0786 = D0788 = StkUsed;
    D0035 = ++LblNum;
    D0033 = ++LblNum;
    nxt_word ();
    loclbl_CR ((v4 = ++LblNum));
    do_command ();

    if ((D003f != C_BUILTIN) || (LblVal != FT_WHILE))
    {
        reprterr ("while expected");
    }

    nxt_word ();
    loclbl_CR (D0035);
    L33d7 (L337e (), v4, D0033, 0);
    loclbl_CR (D0033);
    D0033 = v8;
    D0035 = v6;
    D0786 = v2;
    D0788 = v0;
}

static void
#ifdef __STDC__
do_for (void)
#else
do_for ()
#endif
{
    int v12;
    int v10;
    int v8;
    int v6;
    int v4;
    CMDREF *v2;
    CMDREF *v0;
    register int regptr;

    v2 = 0;
    v0 = 0;
    regptr = D0033;
    v12 = D0035;
    v6 = D0786;
    v4 = D0788;
    D0788 = D0786 = StkUsed;
    v10 = ++LblNum;
    D0033 = ++LblNum;
    nxt_word ();
    lookfor (C_LPAREN);
    L3319 (0);
    lookfor (C_SEMICOLON);

    if (D003f != C_SEMICOLON)
    {
        v2 = get_condition ();
        gencode (124, (v8 = ++LblNum), 0 NUL1);
    }

    lookfor (C_SEMICOLON);     /* L2fe5 */
    
    if ((v0 = L111d (get_operand (0))))
    {
        ck_declared (v0);
        D0035 = ++LblNum;
    }
    else
    {
        D0035 = v10;
    }

    lookfor (C_RPAREN);
    loclbl_CR (v10);
    do_command ();

    if (v0)
    {
        loclbl_CR (D0035);
        L3342 (v0);
    }

    if (v2)     /* L3043 */
    {
        loclbl_CR (v8);
        L33d7 (v2, v10, D0033, 0);
    }
    else
    {
        gencode (124, v10, 0 NUL1);
    }

    loclbl_CR (D0033);
    D0033 = regptr;
    D0035 = v12;
    D0786 = v6;
    D0788 = v4;
}

static void
#ifdef __STDC__
do_retrn (void)
#else
do_retrn ()
#endif
{
    register CMDREF *regptr;

    nxt_word ();

    if ((D003f != C_SEMICOLON) && (regptr = get_operand (0)))      /* else L3197 */
    {
        regptr = L111d (regptr);
        ck_declared (regptr);
        get_fttyp (regptr);

        do_cast (regptr, ((ispointer (FuncGentyp)) ? 1 : FuncGentyp));

        switch (FuncGentyp)
        {
            case FT_LONG:       /* L30f5 */
                L34be (regptr);
                goto L3109;
            case FT_FLOAT:      /* L3100 */
            case FT_DOUBLE:
                L3987 (regptr);
L3109:
                if (regptr->vartyp != 128)
                {
                    gencode (127, C_REG_U, 128 NUL1);
                    gencode (122, C_REG_U NUL2);

                    switch (FuncGentyp)
                    {
                        case FT_FLOAT:      /* L313c */
                        case FT_DOUBLE:
                            gencode (C_TYFLOAT, C_BIGMOV, FuncGentyp NUL1);
                            break;
                        default:            /* L3151 */
                            gencode (C_TYLONG, C_BIGMOV NUL2);
                            break;
                    }
                }

                break;
            default:            /* L3170 */
                reg2d_add (regptr);
                break;
        }

        CCREFtoLeft (regptr);
    }

    prt_rsrvstk (0);
    gencode (FT_RETURN, 0, 0 NUL1);
    ProgFlow = FT_RETURN;
}

static void
#ifdef __STDC__
do_break (void)
#else
do_break ()
#endif
{
    nxt_word ();

    if ( ! D0033)
    {
        reprterr ("break error");
    }
    else
    {
        prt_rsrvstk (D0786);
        gencode (124, D0033, 0 NUL1);
    }
    
    ProgFlow = FT_BREAK;
}

static void
#ifdef __STDC__
do_contin (void)
#else
do_contin ()
#endif
{
    nxt_word ();

    if ( ! D0035)
    {
        reprterr ("continue error");
    }
    else
    {
        prt_rsrvstk (D0788);
        gencode (124, D0035, 0 NUL1);
    }

    ProgFlow = FT_CONTINUE;
}

static void
#ifdef __STDC__
do_goto (void)
#else
do_goto ()
#endif
{
    register LBLDEF *regptr;

    nxt_word ();

    if (D003f != C_USRLBL)
    {
        reprterr ("label required");
    }
    else
    {
        if ((regptr = loclvardef ()))
        {
            /* For non-COCO systems, we need to do the following
             * to avoid type error for assignment
             */
#ifndef COCO
            int *gflag = (int *)(&regptr->mbrlist);
#endif
            gencode (FT_GOTO, regptr->lbl_nbr, 0 NUL1);
#ifdef COCO
            (int)regptr->mbrlist |= 2;
#else
            *gflag |= 2;
#endif
        }

        nxt_word();
    }

    ProgFlow = FT_GOTO;
}

static void
#ifdef __STDC__
gen_goto (void)
#else
gen_goto ()
#endif
{
    register LBLDEF *regptr;

    if ((regptr = loclvardef ()))
    {
        if (regptr->fnccode == FT_STATIC)   /* FT_STATIC */
        {
            multdef ();
        }
        else
        {
#ifndef COCO
            int *gflag = (int *)(&regptr->mbrlist);
#endif

            regptr->fnccode = FT_STATIC;
            gencode (C_GOTO, regptr->lbl_nbr, 0 NUL1);
#ifdef COCO
            (int)regptr->mbrlist |= 1;
#else
            *gflag |= 1;
#endif
        }
    }

    nxt_word ();
    nxt_word ();
}

static LBLDEF *
#ifdef __STDC__
loclvardef (void)
#else
loclvardef ()
#endif
{
    register LBLDEF *regptr = (LBLDEF *)LblVal;

    if (regptr->gentyp != C_GOTO)        /* else L33d3 */
    {
        if (regptr->gentyp)
        {
            if (regptr->isfunction)
            {
                reprterr ("already a local variable");
                return 0;
            }

            null_lbldef (regptr); /* copy regptr to G18Current, null regptr */
        }

        regptr->gentyp = C_GOTO;
        regptr->fnccode = FT_AUTO;
        regptr->lbl_nbr = ++LblNum;
        regptr->mbrlist = 0;
        regptr->isfunction = InFunction;
        regptr->lblprev = LocLblLast;
        LocLblLast = regptr;
    }

    return regptr;
}

static CMDREF *
#ifdef __STDC__
L3319 (int p1)
#else
L3319 (p1)
    int p1;
#endif
{
    register CMDREF *regptr;

    if ((regptr = get_operand (p1)))  /* else return regptr */
    {
        regptr = L3342 (L111d (regptr));
    }

    return regptr;
}

static CMDREF *
#ifdef __STDC__
L3342 (register CMDREF *regptr)
#else
L3342 (regptr)
    register CMDREF *regptr;
#endif
{
    ck_declared (regptr);

    switch (regptr->vartyp)
    {
        case C_INCREMENT:     /* L3357 */  /* '>' */
            regptr->vartyp = C_PLUSPLUS;
            break;
        case C_DECREMENT:     /* L335c */  /* '?' */
            regptr->vartyp = C_MINMINUS;
            break;
    }

    regptr = L6f83 (regptr);
    CCREFtoLeft (regptr);
    return regptr;
}

static CMDREF *
#ifdef __STDC__
L337e (void)
#else
L337e ()
#endif
{
    CMDREF *v0;

    lookfor (C_LPAREN);
    v0 = get_condition ();
    lookfor (C_RPAREN);
    return v0;
}

static CMDREF *
#ifdef __STDC__
get_condition (void)
#else
get_condition ()
#endif
{
    register CMDREF *regptr;

    if ((regptr = L111d (get_operand (0))))
    {
        ck_declared (regptr);
    }
    else
    {
        reprterr ("condition needed");
    }

    return regptr;
}

/* ******************************************************************** *
 * L33d7 () -                                                           *
 * Passed : (1) regptr - CMDREF under consideration                     *
 *          (2) p1 - Label Number                                       *
 *          (3) p2 - Label Number                                       *
 *          (4) p4 - Flag                                               *
 * ******************************************************************** */

static void
#ifdef __STDC__
L33d7 (register CMDREF *regptr, int p2, int p3, int p4)
#else
L33d7 (regptr, p2, p3, p4)
    register CMDREF *regptr;
    int p2;
    int p3;
    int p4;
#endif
{
    if (regptr)
    {
        L939d (regptr, p2, p3, p4);
        CCREFtoLeft (regptr);
    }
}
