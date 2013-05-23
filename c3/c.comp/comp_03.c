/* ************************************************************ *
 * ccomp_03.c                                                   *
 *                                                              *
 * comes from p1_02.c                                           *
 *                                                              *
 * $Id:: comp_03.c 73 2008-10-03 20:20:29Z dlb                $ *
 * ************************************************************ */

/* Sun, 01 Jun 2008 14:33:24 -0500
 * This version matches the original except for a couple of jump
 * variations
 */

#include "ccomp.h"

static CMDREF *get_lftcmd ();
static int math_op ();
static CMDREF *paren_refs ();
static CMDREF *setup_cast ();
static int find_size (
#ifdef __STDC__
        CMDREF **p1
#endif
);


/* ************************************************************ *
 * get_operand () - get the operand(s) of a statement           *
 * Passed : maxlblval - the max value that can be in LblVal.    *
 * ************************************************************ */

CMDREF *
#ifdef __STDC__
get_operand (int maxlblval)
#else
get_operand (maxlblval)
int maxlblval;
#endif
{
    CMDREF *my_left;
    int _prevtyp;    /* vartype ??? */
    int my_lval;
    CMDREF *cmdRight;
    int _line;
    char *_cmdstr;
    int srch_lval;

    register CMDREF *oprnd;
    
    if ( ! (my_left = get_lftcmd ()))  /* If no primary, return 0 */
    {
        return 0;
    }

    /* Loop as long as there is any assignment
     * This takes care of such things as a = b = c = .... */

    while ((math_op ()) && (maxlblval <= LblVal))    /* L087e */
    {
        _prevtyp = D003f;        /* L07a0 */
        srch_lval = my_lval = LblVal;
        _line = InpLinNum;
        _cmdstr = InpLinStr;
        nxt_word();

        switch (_prevtyp)
        {
            case C_QUESTION:    /* _10 (L07de) */
            case C_EQUAL:
                break;
            default:            /* _14 (L07b9) */

                /* Not <something>=  i.e. +=, -=, %=, etc */

                if ((_prevtyp < C_PLUSEQ) || (_prevtyp > C_EOREQ))
                {
                    srch_lval = my_lval + 1;
                }

                break;
        }

        /* recurse into self to get operand */

        if ((oprnd = get_operand (srch_lval)))      /* L07de else L0873 */
        {
            /*  if ? then : else */

            if (_prevtyp == C_QUESTION)       /* else L0852 */
            {
                if (lookfor (C_COLON))  /* fail */     /* else L0847 */
                {
                    CCREFtoLeft (oprnd);     /* L0847 */
                    goto retrn_ref;     /* Wouldn't "break" work? */
                }
                else    /* success - found colon    process third expr */
                {
                    _cmdstr = InpLinStr;
                    _line = InpLinNum;

                            /* If operand is int, char, or union */

                    if ((cmdRight = get_operand (FT_UNION)))  /* else L083c */
                    {

                        oprnd = add_cmdref ( C_COLON,
                                              oprnd, cmdRight, /* lft, rght */
                                              3,                /* cmdval    */
                                              _line, _cmdstr);
                        /* go to L0852 */
                    }
                    else
                    {
                        reprterr ("third expression missing");  /* L083c */

                        /* Same procedure as above if lookfor (C_COLON)
                         * is true
                         */

                        CCREFtoLeft (oprnd);
                        goto retrn_ref;
                    }
                }
            }

            /* L0852 */
            my_left = add_cmdref ( _prevtyp,
                                   my_left, oprnd,  /* left, right */
                                   my_lval,         /* cmdval      */
                                   _line, _cmdstr );
        }
        else
        {
            reprterr ("operand expected");  /* L0873 */
        }
    }       /* end while (math_op ()) */

retrn_ref:
    return my_left;
}

static CMDREF *
#ifdef __STDC__
get_lftcmd (void)
#else
get_lftcmd ()
#endif
{
    CMDREF *v10;
    int _ctyp;
    int _lin;
    char *_strpt;
#ifdef COCO
    int v2;    /* This var is not used, I think */
#endif
    CMDREF *v0;

    register CMDREF *maincref = 0;

    switch (D003f)      /* L0a01 */
    {
        case C_DQUOT:
        case C_USRLBL:
        case C_LONG:
        case C_DOUBLE:
        case C_INT:    /* L08ab */
            maincref = add_cmdref (D003f,
                                   0, 0,    /* No left or right cmd */
                                   LblVal, InpLinNum, InpLinStr);
            nxt_word ();
            break;

        case C_LPAREN:     /* L08d0 */  /* '-' */
            nxt_word ();

            if (is_vardef ())   /* If it's a basic var type, or typedef */
            {
                maincref = setup_cast ();
                lookfor (C_RPAREN);

                /* Recurse into self */

                if ( ! (maincref->cr_Left = get_lftcmd()))   /* else break */
                {
                    mk_lftcmd (maincref);
                    maincref = 0;
                }

                break;
            }

            if ( ! (maincref = get_operand (0)))  /* L08fe */  /* else L0934 */
            {
need_expression:
                exprmsng ();
                maincref = add_cmdref ( C_INT,
                                        0, 0,   /* No left or right cref */
                                        0,      /* cmdval */
                                        InpLinNum, InpLinStr);
            }

            lookfor (C_RPAREN);

            break;

        case C_EXCLAM:      /* L093f */
        case C_MINUS:
        case C_TILDE:
        case C_ASTERISK:
        case C_MINMINUS:
        case C_PLUSPLUS:
        case C_AMPERSAND:
            _ctyp = D003f;
            _lin = InpLinNum;
            _strpt = InpLinStr;
            nxt_word ();

            if ((v10 = get_lftcmd ()))  /* self */     /* else L097a */
            {
                maincref = add_cmdref ( _ctyp,
                                        v10,        /* cr_Left     */
                                        0,          /* no cr_Right */
                                        FT_EXTERN,  /* myval       */
                                        _lin, _strpt);
            }
            else {
                reprterr ("primary expected");
            }

            break;

        case C_SIZEOF:     /* L0986 */
            _lin = InpLinNum;
            _strpt = InpLinStr;
            nxt_word ();

            if (D003f == C_LPAREN)        /* else L09c8 */
            {
                nxt_word ();

                if (is_vardef ())          /* else L09aa */
                {
                    v10 = setup_cast ();    /* go to L09bc */
                }
                else
                {
                    if ( ! (v10 = get_operand (0)))
                    {
                        goto need_expression;
                    }
                }

                lookfor (C_RPAREN);     /* L09bc */
            }
            else
            {
                v10 = get_lftcmd ();     /* L09c8 */
            }

            v0 = v10;           /* L09cd */
            maincref = add_cmdref ( C_INT,
                                    0, 0,       /* no left or right cref */
                                    find_size (&v0),/* cmdval                */
                                    _lin, _strpt);
            CCREFtoLeft (v0);

            break;
    }

    if (maincref == 0)        /* _37 */
    {
        return 0;
    }

    /* Loop as long as we have
     *          left parentheses,
     *          left brace,
     *          struct/union ref (either "->" or ".")
     */

    for (;;)
    {
        switch (D003f)      /* L0b94 */
        {
            case C_LPAREN:     /* L0a73 */  /* '-' */
                _strpt = InpLinStr;
                _lin = InpLinNum;
                nxt_word ();
                maincref = add_cmdref ( C_PARENS,
                                        maincref,       /* Left  */
                                        paren_refs (),  /* Right */
                                        FT_STATIC,      /* cmdval */
                                        _lin, _strpt);
                lookfor (C_RPAREN);

                continue;

            case C_LBRKET:     /* L0aa4 */  /* '+' */
                nxt_word ();
                
                /* Must be int or char */

                if ( ! (v10 = get_operand (FT_CHAR)))     /* else L0ad8 */
                {
                    exprmsng ();
                    v10 = add_cmdref ( C_INT,
                                       0, 0,    /* left, right */
                                       0,       /* cmdval      */
                                       InpLinNum, InpLinStr);
                }

                maincref = add_cmdref ( C_PLUS,
                                        maincref, v10,  /* left, right */
                                        12,             /* cmdval      */
                                        InpLinNum, InpLinStr);
                maincref = add_cmdref ( C_ASTERISK,
                                        maincref, 0,    /* left, right */
                                        15,
                                        InpLinNum, InpLinStr);
                lookfor (C_RBRKET);

                continue;

            case C_PERIOD:     /* L0b24 */
            case C_PTRREF:
                _ctyp = D003f;
                _lin = InpLinNum;
                _strpt = InpLinStr;
                
                ++Struct_Union;
                nxt_word ();    /* Get member */
                --Struct_Union;

                /* if not USRLBL, it's not a proper member name ! */

                if (D003f != C_USRLBL)
                {
                    noidentf ();           /* go to L0bb0 */
                    break;
                }
                else
                {
                    /* CMDREF for ptr variable itself */
                    v10 = add_cmdref ( D003f,
                                       0, 0,    /* left, right */
                                       LblVal, InpLinNum, InpLinStr);
                    
                    /* CMDREF for ptr symbol */
                    maincref = add_cmdref ( _ctyp,
                                            maincref, v10,  /* left, right */
                                            15,             /* cmdval */
                                            _lin, _strpt);
                    nxt_word ();
                    continue;
                }

        }

        break;
    }

    /* L0bb0 */
    switch (D003f)
    {
        case C_PLUSPLUS:   /* L0bb4 */ /* 0x3c */
            D003f = C_INCREMENT;
            goto L0bc4;
        case C_MINMINUS:   /* L0bbd */ /* 0x3d */
            D003f = C_DECREMENT;
L0bc4:
            maincref = add_cmdref ( D003f,
                                    maincref, 0,    /* left, right */
                                    14,             /* cmdval      */
                                    InpLinNum, InpLinStr);
            nxt_word ();

            break;
    }

    return maincref;
}

/* ******************************************************************** *
 * paren_refs () - generates a cmdval for parentheses                   *
 * ******************************************************************** */

static CMDREF *
#ifdef __STDC__
paren_refs (void)
#else
paren_refs ()
#endif
{
    CMDREF *var0;

    register CMDREF *regint = 0;

    while (1)
    {
        if (D003f == C_RPAREN)      /* else L0c50 */
        {
            break;
        }

        if ((var0 = get_operand (FT_CHAR)))     /* if char or int */
        {                                       /* else L0c43     */
            var0 = add_cmdref ( 11,             /* 11 = FT_PARAM  */
                                var0, regint,   /* left, right    */
                                0,              /* cmdval         */
                                var0->_cline, var0->_lpos);
            regint = var0;
        }

        if (D003f != C_COMMA)
        {
            break;
        }

        nxt_word ();
    }

    return regint;
}

int
#ifdef __STDC__
getconst (int parm1)
#else
getconst (parm1)
    int parm1;
#endif
{
    int _valu;
    register CMDREF *_cref;

    _cref = L111d (get_operand (parm1));

    if ((_cref) && (_cref->vartyp == C_INT))
    {
        _valu = _cref->cmdval;
    }
    else
    {
        _valu = 0;
        reprterr ("constant required");
    }

    if ( _cref)
    {
        CCREFtoLeft (_cref);
    }

    return _valu;
}

/* ******************************************************************** *
 * math_op () - tests to see if current symbol is some sort of math     *
 *      symbol, comma, or "?", and returns TRUE if so.   Returns FALSE  *
 *      for anything else                                               *
 * ******************************************************************** */

static int
#ifdef __STDC__
math_op (void)
#else
math_op ()
#endif
{
    switch (D003f)
    {
        case C_AMPERSAND:  /* L0cac */
            D003f = C_AND;
            LblVal = 8;
            return 1;
        case C_ASTERISK:   /* L0cb6 */
            D003f = C_MULT;
            LblVal = 13;
            return 1;
        case C_MINUS:      /* L0cc0 */
            D003f = C_NEG;
            LblVal = 12;
            /* Fall through to next case */
        case C_COMMA:      /* L0cca */
        case C_EQUAL:
        case C_QUESTION:
            return 1;
        case C_COLON:      /* retrn0 */
            return 0;
        default:           /* L0ccf */
            if ( ((D003f >= C_ANDAND) && (D003f <= C_GT))    ||
                 ((D003f >= C_PLUSEQ) && (D003f <= C_EOREQ))    )
            {
                return 1;
            }
            return 0;
    }

    /*return 1;*/
}

int
#ifdef __STDC__
do_math (int vrtyp, int firstval, int secondval)
#else
do_math (vrtyp, firstval, secondval)
    int vrtyp;
    int firstval;
    int secondval;
#endif
{
    switch (vrtyp)
    {
        case C_PLUS:       /* addem */
            return (firstval + secondval);
        case C_NEG:        /* subtract */
            return (firstval - secondval);
        case C_MULT:        /* multiply */
            return (firstval * secondval);
        case C_SLASH:      /* divide */
            if (secondval == 0)
            {
                divby_0 ();
                return 0;
            }

            return (firstval / secondval);
        case C_PERCENT:    /* do_mod */
            if (secondval)
            {
                return (firstval % secondval);
            }
            
            divby_0 ();
            return 0;

        case C_AND:        /* do_and */
            return (firstval & secondval);
        case C_VBAR:       /* do_or */
            return (firstval | secondval);
        case C_CARET:      /* do_eor */
            return (firstval ^ secondval);
        case C_LSHIFT:     /* ashft_l */
            return (firstval << secondval);
        case C_RSHIFT:     /* ashft_r */
            return (firstval >> secondval);
        case C_EQEQ:       /* tst_eq */
            return (firstval == secondval);
        case C_NOTEQ:      /* tst_neq */
            return (firstval != secondval);
        case C_GT:         /* tst_gt */
            return ((firstval > secondval) ? 1 : 0);
        case C_LT:         /* tst_lt */
            return ((firstval < secondval) ? 1 : 0);
        case C_GT_EQ:      /* tst_le */
            return ((firstval >= secondval) ? 1 : 0);
        case C_LT_EQ:      /* L0de7 */
            return ((firstval <= secondval) ? 1 : 0);
        case C_MINUS:      /* negate */
            return (-firstval);
        case C_EXCLAM:     /* is_zero */
            return (!firstval);
        case C_TILDE:      /* do_com */
            return (~firstval);
        case C_ANDAND:     /* both_nul */
            return (firstval && secondval);
        case C_OROR:       /* neit_nul */
            return (firstval || secondval);

            /* The following deal with unsigned equality/inequality */

        case C_U_LTEQ:
        case C_U_LT:        /* L0e32 */
        case C_U_GTEQ:
        case C_U_GT:
            {
                unsigned int usecond;
                unsigned int ufirst;

                usecond = secondval;
                ufirst = firstval;

                switch (vrtyp)
                {
                    case C_U_LTEQ:          /* L0e40 */
                        return (ufirst <= usecond);
                    case C_U_LT:            /* L0e4c */
                        return (ufirst < usecond);
                    case C_U_GTEQ:          /* L0e58 */
                        return (ufirst >= usecond);
                    default:                /* L0e64 */
                        return (ufirst > usecond);
                }
            }

        default:
            reprterr ("constant operator");
            return 0;
    }
}

static CMDREF *
#ifdef __STDC__
setup_cast (void)
#else
setup_cast ()
#endif
{
    int lblsiz;
    struct brktdef *_membr;
    int __ftTy;
#ifdef COCO
    LBLDEF *ref;
#else
    void *ref;  /* Multiple usage pointer in function */
#endif
    int _linnbr;
    char *_linptr;
    struct memberdef *var0; /* Not used here, just a dummy for do_vartype () */

    _linnbr = InpLinNum;
    _linptr = InpLinStr;
    __ftTy = do_vartype (&lblsiz, &_membr, &var0);
    __ftTy = proc_varname ((LBLDEF **)(&ref), &_membr, __ftTy);
    finishfunc (&D0027);

    /* proc_varname() nulls out LBLDEF passed to it, then sets it if
     * the word being processed is a USRLBL.
     * A cast should not be a USRLBL unless it's a typedef'ed name
     */

    if (ref)
    {
         reprterr ("name in a cast");
    }

    ref = add_cmdref ( 32,
                       0, 0, 0,             /* left, right, cmdval */
                       _linnbr, _linptr );
    ((CMDREF *)ref)->ft_Ty = __ftTy;

    setlblsize (ref, _membr, lblsiz);

    return ref;
}

/* **************************************************************** *
 * add_cmdref () - Checks to see if CurntCREF points to a CMDREF    *
 *      if so, points CurntCREF to the previous cmdref              *
 *      and returns CurntCREF after filling with parameter data     *
 *      Otherwise, creates a new CMDREF and initializes it          *
 * Passed:  See below in the prototype                              *
 * Returns: Either CurntCREF or the new CMDREF - whichever applies  *
 * **************************************************************** */

CMDREF *
#ifdef __STDC__
add_cmdref ( int __ccode,           /* (1) Applicable C_type        */
             CMDREF *_leftcrf,      /* (2) cr_Left                  */
             CMDREF *_rightcrf,     /* (3) cr_Right                 */
             int myval,             /* (4) The value, or LBLDEF     */
             int __myline,          /* (5) The line number          */
             char *_line_pos        /* (6) The position in the line */
           )
#else
add_cmdref (__ccode, _leftcrf, _rightcrf, myval, __myline, _line_pos)
    int __ccode;
    CMDREF *_leftcrf;
    CMDREF *_rightcrf;
    int myval;
    int __myline;
    char *_line_pos;
#endif
{
    register CMDREF *regptr;

    if (CurntCREF)
    {
        regptr = CurntCREF;
        CurntCREF = regptr->cr_Left;
    }
    else
    {
        regptr = addmem (sizeof (CMDREF));
    }

    regptr->vartyp = __ccode;
    regptr->cr_Left = _leftcrf;
    regptr->cr_Right = _rightcrf;
    regptr->cmdval = myval;
    regptr->_cline = __myline;
    regptr->_lpos = _line_pos;
    regptr->ptrdstval = 0;

    return regptr;
}

void
#ifdef __STDC__
exprmsng (void)
#else
exprmsng ()
#endif
{
    reprterr ("expression missing");
}

static int
#ifdef __STDC__
find_size (CMDREF **base_cref)
#else
find_size (base_cref)
    CMDREF **base_cref;
#endif
{
    register CMDREF *regptr = *base_cref;

    switch (regptr->vartyp)
    {
        default:            /* L103d */
            regptr = *base_cref = L111d (regptr);
            goto retrn_siz;
        case C_USRLBL:      /* L104d */
            regptr->ft_Ty = ((CMDREF *)(regptr->cmdval))->ft_Ty;
            ck_declared (regptr);
            regptr = (CMDREF *)(regptr->cmdval);
        case 32:    /* cast */    /* L105f */

retrn_siz:
            return (get_varsize ( regptr->ft_Ty,
                                  regptr->varsize,
                                  regptr->arrdefs)    );
            break;

            /* In the case of struct/union, we want the size of the member */

        case C_PERIOD:
        case C_PTRREF:     /* L1072 */
            return find_size (&(regptr->cr_Right));
            break;
    }
}
