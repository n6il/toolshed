/* **************************************************************** *
 * comp_14.c - part 14 for c.comp                                   *
 * comes from p1_05.c                                               *
 *                                                                  *
 * This file contains the "head" routines.  That is, the mainline   *
 * routines for parsing a file.  These routines call initialize     *
 * the LBLDEF for a variable or routine, and do the primary setups  *
 *                                                                  *
 * $Id:: comp_14.c 73 2008-10-03 20:20:29Z dlb                $     *
 * **************************************************************** */

#include "ccomp.h"

/* A structure defining an initialized variable defined within a block */

struct blockvartbl {
    struct blockvartbl *ss_nxt;
    CMDREF *bvarcmd;
    LBLDEF *mylbl;
};

static direct int RegClsDpth;
static direct struct blockvartbl *BlkIniFrst,
                                 *BlkIniLast,
                                 *CurBlkVar;

/* Following are function prototypes for static vars */

static int getSC_word ();
static void sizundef ();
static void func_param ();
static void blockvardef ();

static int declrcmp (
#ifdef __STDC__
    LBLDEF *, int, struct memberdef *
#endif
);

static void dofunction (
#ifdef __STDC__
    LBLDEF *, int
#endif
);

static void do_parentheses (
#ifdef __STDC__
    LBLDEF **
#endif
);

static int mrgdepths (
#ifdef __STDC__
    int ftype, int depthflgs
#endif
);


static int set_regclass (
#ifdef __STDC__
        int sclass, int genty
#endif
);


/* ******************************************************************** *
 * funcmain () - The mainline function for processing a command or      *
 *          function.  This function is called from main() to process   *
 *          each command.                                               *
 *          When a function is encountered, control branches out into   *
 *          functions that handle parameters, block vars, and commands  *
 *          within the function.  Control returns to funcmain () after  *
 *          the end of the function                                     *
 *                                                                      *
 * On entry, we have already run nxt_word().  Any "^#" directives have  *
 * already been taken care of, so now we are at the begin of a variable *
 * definition, function prototype, or function entry.                   *
 *                                                                      *
 * If it's a variable def, extern def, or function prototype,           *
 * funcmain () process that command line (until a semicolon is          *
 * encountered), and then returns to main ().                           *
 * ******************************************************************** */

void
#ifdef __STDC__
funcmain (void)
#else
funcmain ()
#endif
{
    int _varsize;
    int _varsz;
    int real_ftyp;
    struct brktdef *brktfrst;
    int _vt_ftyp;
    int varclass;
    int _lbl_gentyp;
    struct brktdef *typdfmbr;
    LBLDEF *tmplbldef;
    struct memberdef *strcmbr;

    register LBLDEF *mylbldef;     /* I'm sure this is correct */

    /* This is my addition.  It seems that c.comp (even on the COCO)
     * erroneously reports an error when multiple ";"'s are encountered
     * outside any blocks.  This doesn't seem to happen when inside a
     * block.  All the other compilers I use (gcc, c68) permit this,
     * which is definitely not illegal.
     */

    /* FIXME
     * Second thought...  it _could_ be useful to _not_ allow this,
     * although it _is_ legal, and although this is a bug, it could be
     * used as a "feature".  Where I noticed this was in my header
     * file and for an #ifdef COCO... I'd placed a semicolon outside
     * a second-level #ifdef block when it belonged inside the first level...
     * Oh, well... if anyone thinks it's better to eliminate this, it can
     * be done.
     */

    while (*InpLinStr == ';')
    {
        nxt_word ();
    }       /* end of patch */

    /* We never see either Brace here.  Blocks are handled further down */

    while (D003f == C_RBRACE)
    {
        reprterr ("too many brackets");
        nxt_word();
    }

    /* If it's a left brace, it's illegal at this point.
     * Process the block and return.
     */

    if (D003f == C_LBRACE)
    {
        reprterr ("function header missing");
        /* I believe the parameter 0 for do_block() is invalid.  We'll
         * leave it like this till we verify that we have correct code
         */
#ifdef COCO
        do_block (0);
#else
        do_block ();
#endif
        finishfunc (&LocLblLast);
        nxt_word();
        return;
    }

    switch (_vt_ftyp = getSC_word ())
    {
        case FT_REGISTER:
        case FT_AUTO:
            /* At this point, we're still outside the function
             * we cannot have register or auto types */

            reprterr ("storage error");
                /* Fall through to next case */
        case 0:
            /* We didn't have a type definition, default to FT_NONDPDAT */
            _vt_ftyp = FT_NONDPDAT;
    }

    /* Here, we should be at _some_ kind of a data type - int, char, etc
     * Go process the definition
     */

    if ( ! (varclass = do_vartype (&_varsize, &typdfmbr, &strcmbr)))
    {                       /* else L8228 */
        varclass = FT_INT;
    }

                        /* L8228 */
    /* We should now be at the variable (or function) name,
     * with its storage class (if any) defined.
     * The following loop will process the variable (or extern) def,
     * and if it is a comma-separated list of names, all these.
     * If it's a function, call dofunction () to process the whole function
     */

    for (;;)
    {
        brktfrst = typdfmbr;    /* Non-zero if it's a typedef ? */

        /* proc_varname will, among other things, determine if the
         * current command is beginning a function
         */

        _lbl_gentyp = proc_varname (&tmplbldef, &brktfrst, varclass);

        if ( ! (mylbldef = tmplbldef))    /* else L825c */
        {
            if ((_lbl_gentyp != FT_STRUCT) && (_lbl_gentyp != FT_UNION))
            {
                noidentf ();
            }

            goto L841c;
        }

        /* if inparentheses, is it function or func prototype? */

        if (inparentheses (_lbl_gentyp))      /* else L8281 */
        {
            if ((D003f == C_COMMA) || (D003f == C_SEMICOLON))
            {
                real_ftyp = FT_EXTERN;      /* function prototype */
            }
            else
            {
                real_ftyp = FT_NONDPDAT;       /* parameter assignment */
            }
        }
        else
        {
            real_ftyp = _vt_ftyp;   /* Any variable */          /* L8283 */
        }

        if (mylbldef->gentyp)     /* else L82cf */
        {
                    /* 0 = match */
            if ( ! (declrcmp (mylbldef, _lbl_gentyp, strcmbr)) &&
                            (real_ftyp != FT_EXTERN)             )
            {                   /* else L83a3 */
                if ( (mylbldef->fnccode != FT_EXTERN) &&
                     (mylbldef->fnccode != FT_DPXTRN)    )  /*else L82be*/
                {
                    multdef();
                }
                else
                {
                    mylbldef->fnccode = real_ftyp;       /* L82be */
                    mylbldef->arrmbrs = brktfrst;
                    mylbldef->mbrlist = strcmbr;
                    goto L82e1;
                }
            }
        }
        else    /* mylbldef->gentyp = 0 --- new definition? */
        {       /* L82cf */
            mylbldef->gentyp = _lbl_gentyp;
            mylbldef->isfunction = 0;
            mylbldef->fnccode = real_ftyp;
            mylbldef->mbrlist = strcmbr;
L82e1:
            _varsz = setlblsize (mylbldef, brktfrst, _varsize);

            if ( ! (inparentheses (_lbl_gentyp)))   /* if not function */
            {                                           /* else L83a3 */
                if (D003f == C_EQUAL)  /* assignment */
                {
                    do_initvar (mylbldef, real_ftyp, _lbl_gentyp);
                }
                else
                {
                    if ( ! _varsz && (real_ftyp != FT_EXTERN)) /* L831f */
                    {
                        sizundef ();
                    }
                    else    /* It's a legal var. print to output stream */
                    {
                        switch (real_ftyp)
                        {
                            case FT_DPSTATIC:       /* L8336 */
                            case FT_STATIC:
                                localdata (mylbldef, _varsz,
                                            real_ftyp == FT_DPSTATIC);
                                break;
                            case FT_DIRECT:         /* L8353 */
                            case FT_NONDPDAT:
                                globldata (mylbldef, _varsz,
                                            real_ftyp == FT_DIRECT);
                                break;
                        }
                    }
                }

                /* L8389 */

                /* Reset specific types to generic types */

                if (real_ftyp == FT_STATIC)
                {
                    mylbldef->fnccode = FT_NONDPDAT;
                }
                else
                {
                    if (real_ftyp == FT_DPSTATIC)
                    {
                        mylbldef->fnccode = FT_DIRECT;
                    }
                }
            }
        }

        /* It's a function, deal with it */        /* L83a3 */

        if (inparentheses (_lbl_gentyp))   /* else L841f */
        {
            /* Get rid of one level of parentheses depth */

            _lbl_gentyp = MSBrshft2 (_lbl_gentyp);

                /* Illegal return type for function */

            if (    (inbrkets (_lbl_gentyp))      ||
                    (inparentheses (_lbl_gentyp)) ||
                    (_lbl_gentyp == FT_STRUCT)    ||
                    (_lbl_gentyp == FT_UNION))
            {
                reprterr ("function type error");
                mylbldef->gentyp = 0x30 + FT_INT;   /* inparentheses + INT */
                _lbl_gentyp = FT_INT;
            }

            if (real_ftyp == FT_EXTERN)     /* prototype */ /* L83f4 */
            {
                finishfunc (&D0027);
            }
            else        /* it's a real function, process */
            {
                FuncGentyp = _lbl_gentyp;     /* L8409 */
                dofunction (mylbldef, _vt_ftyp);
                return;
            }
        }
L841c:
        if (D003f == C_COMMA)
        {
            nxt_word();
        }
        else
        {
            break;
        }
    }       /* end of "for (;;)" at L8228 */

    if (lookfor (C_SEMICOLON))     /* _21 ( L842d ) */
    {
        cmma_rbrkt ();
    }
}

/* **************************************************************** *
 * func_param () - process a function parameter definition          *
 *                                                                  *
 * **************************************************************** */

static void 
#ifdef __STDC__
func_param (void)
#else
func_param ()
#endif
{
    int _strgclass;
    int parmftyp;
    struct brktdef *mbrdf_v12;
    struct brktdef *typdfmbr;
    int _varsize;
    int ffcod;
    int _genty;
    LBLDEF *tmplbldf;
    struct memberdef *strctmbr;
    register LBLDEF *mylbldf;

    switch ((_strgclass = getSC_word()))
    {
        default:        /* L8456 */
            reprterr ("argument storage");
            /* fall through to next case to reset to a valid storage class */
        case 0:         /* L8461 */
            _strgclass = FT_AUTO;
            break;
        case FT_REGISTER:   /* Only valid storage class for func args */
            break;
    }

    if ( ! (parmftyp = do_vartype (&_varsize, &typdfmbr, &strctmbr)))
    {                                   /* L8474 */
        parmftyp = FT_INT;      /* default */
    }

    /* We have the type, and are now poised on the variable name(s) */
    
    for (;;)        /* Loop through for all comma-separated varnames */
    {
        mbrdf_v12 = typdfmbr;                  /* L848e */
        _genty = proc_varname (&tmplbldf, &mbrdf_v12, parmftyp);
        mylbldf = tmplbldf;
        
        /* Cannot declare argument as function, struct, or union */

        if (    (inparentheses (_genty)) ||
                (_genty == FT_STRUCT)    ||
                (_genty == G_STRCNAM)   )
        {
            reprterr ("argument error");
            goto namedone;
        }
        else
        {
            if (inbrkets (_genty))
            {
                _genty = incptrdpth (MSBrshft2 (_genty));   /* go to L84f5 */
            }
            else
            {
                if (_genty == FT_FLOAT)     /* Promote */
                {
                    _genty = FT_DOUBLE;
                }
            }

            if (mylbldf == 0)
            {
                noidentf ();
                goto namedone;
            }
        }

        ffcod = set_regclass (_strgclass, _genty);       /* L8506 */

        switch (mylbldf->fnccode)
        {
            case FT_PARAM:    /* L851a */
                mylbldf->gentyp = _genty;
                mylbldf->fnccode = ffcod;
                mylbldf->mbrlist = strctmbr;
                
                if ( ! (setlblsize (mylbldf, mbrdf_v12, _varsize)))
                {
                    sizundef ();
                }

                break;

            case FT_AUTO:       /* L853e */
            case FT_REGISTER:
                multdef ();
                break;

            default:
                reprterr ("not an argument");
                break;
        }

namedone:
        if (D003f == C_EQUAL)
        {
            cant_init ();
        }

        if (D003f == C_COMMA)
        {
            nxt_word ();
        }
        else
        {
            break;
        }
    }       /* end for (;;) loop - variable name(s) */

    if (lookfor (C_SEMICOLON))     /* L857f */     /* else L87e1 */
    {
        cmma_rbrkt ();
    }
}

/* **************************************************************** *
 * blockvardef () - Processes a label definitions within a block    *
 * **************************************************************** */

static void 
#ifdef __STDC__
blockvardef (void)
#else
blockvardef ()
#endif
{
    int storg_clas;
    int genty;
    int loc_gentyp;
    struct brktdef *typdfmbr;
    int _varsiz;
    CMDREF *my_cmdref;
    LBLDEF *varlbldf;
    struct memberdef *strcmbr;
    struct blockvartbl *curnt_some;

    switch (storg_clas =  getSC_word())
    {
        case FT_DIRECT:                 /* Cannot have "direct" local vars */
            reprterr ("storage error");
        case 0:                         /* Default type is "auto" */
            storg_clas = FT_AUTO;
    }

    if ( ! (genty = do_vartype(&_varsiz, &typdfmbr, &strcmbr)))
    {
        genty = FT_INT;
    }

    for (;;)    /* Process var name (or comma-separated list) */    /* L85de */
    {
        int _loc_ftyp;
        int _loc_siz;
        struct brktdef *locstrcmbr;

        register LBLDEF *_curntlbl;

        locstrcmbr = typdfmbr;
        loc_gentyp = proc_varname (&varlbldf, &locstrcmbr, genty);
        
        if ( ! (_curntlbl = varlbldf)) /* Not a USRLBL */     /* else L861a */
        {
            if ((loc_gentyp != FT_STRUCT) && (loc_gentyp != FT_UNION))
            {                               /* else L8685 */
                noidentf ();                /* go to L8666 */
            }

            goto L87c0;
        }
        else
        {
            if ( (inparentheses (loc_gentyp)) || (storg_clas == FT_EXTERN) )
            {                           /* L861a */     /*else L8666*/
                if ( ! _curntlbl->gentyp)  /* L862f */     /* else L8654 */
                {
                    _curntlbl->gentyp = loc_gentyp;
                    _curntlbl->fnccode = FT_EXTERN;    /* 14 */
                    _curntlbl->isfunction = 0;
                    _curntlbl->mbrlist = strcmbr;
                    setlblsize (_curntlbl, locstrcmbr, _varsiz);
                }
                else
                {
                    declrcmp (_curntlbl, loc_gentyp, strcmbr);    /* L8654 */
                }
                    
                goto L87c0;
            }
        }

        _loc_ftyp = set_regclass (storg_clas, loc_gentyp);      /* L8666 */

        /* If it's a previously-defined name */

        if (_curntlbl->gentyp)     /* else L8692 */
        {
            if (_curntlbl->isfunction == InFunction)  /* Already a var in this block? */
            {               /* else L868b */
                multdef ();
                goto L87c0;
            }

            null_lbldef (_curntlbl);  /* reuse this label's storage */
        }

        _curntlbl->gentyp = loc_gentyp;          /* L8692 */
        _curntlbl->fnccode = _loc_ftyp;   /* was FT_EXTERN (switched from L862f */
        _curntlbl->mbrlist = strcmbr;
        _curntlbl->isfunction = InFunction;           /* ??? Parentheses depth ??? */
        _curntlbl->lblprev = LastLbl;
        LastLbl = _curntlbl;
        
        if ( ! (_loc_siz = setlblsize (_curntlbl, locstrcmbr, _varsiz)))
        {
            if (D003f != C_EQUAL)
            {
                sizundef ();
            }
        }

        switch (_loc_ftyp)
        {
            case FT_AUTO: /* L86cc */
                StkTot -= _loc_siz;
                _curntlbl->lbl_nbr = StkTot;
                break;
            case FT_DPSTATIC: /* L86d6 */
            case FT_STATIC:
                _curntlbl->lbl_nbr = (++LblNum);
                break;
        }

        if (D003f == C_EQUAL)   /* else L878f */       /* L86f2 */
        {
            if ((_loc_ftyp == FT_STATIC) || (_loc_ftyp == FT_DPSTATIC))
            {
                do_initvar (_curntlbl, _loc_ftyp, loc_gentyp);    /* go to L87c3 */
            }
            else
            {
                nxt_word ();      /* L871f */

                if (( ! (inbrkets (loc_gentyp)))  &&
                        (loc_gentyp != FT_STRUCT) &&
                        (my_cmdref = get_operand (FT_CHAR)))
                {                       /* else L878a */
                    if (CurBlkVar)  /* else L875e */
                    {
                        CurBlkVar = (curnt_some = CurBlkVar)->ss_nxt;
                        curnt_some->ss_nxt = 0;
                    }
                    else
                    {
                        /*curnt_some = addmem (6);*/
                        curnt_some = addmem (sizeof (struct blockvartbl));
                    }

                    curnt_some->bvarcmd = my_cmdref;
                    curnt_some->mylbl = _curntlbl;

                    /* Chain structs or initialize pointer */

                    if (BlkIniFrst)
                    {
                        BlkIniLast->ss_nxt = curnt_some;
                    }
                    else
                    {
                        BlkIniFrst = curnt_some;
                    }

                    BlkIniLast = curnt_some;
                }
                else
                {
                    cant_init ();   /* L878a */
                }
            }
        }
        else
        {
            switch (_loc_ftyp)
            {
                case FT_STATIC:     /* L8793 */ /*15*/
                case FT_DPSTATIC:
                    rmbnolbl (_curntlbl->lbl_nbr, _loc_siz,
                                (_loc_ftyp == FT_DPSTATIC));
                    break;
                default:            /* L87c3 */
                    break;
            }
        }

L87c0:
        if (D003f != C_COMMA)    /* L87c3 */
        {
            break;
        }

        nxt_word ();
    }       /* End of the for (;;) loop holding 6 bytes of data */

    lookfor (C_SEMICOLON);
}

static int
#ifdef __STDC__
declrcmp (register LBLDEF *regptr, int gntyp, struct memberdef *mbr)
#else
declrcmp (regptr, gntyp, mbr)
    register LBLDEF *regptr;
    int gntyp;
    struct memberdef *mbr;
#endif
{
    if ( (regptr->gentyp != gntyp)                         ||
         ((gntyp == FT_STRUCT) && (regptr->mbrlist != mbr))   )
    {
        reprterr ("declaration mismatch");
        return 1;
    }

    return 0;
}

/* ******************************************************************** *
 * set_regclass () - Sets FT_REGISTER storage-class variables if        *
 *          applicable.                                                 *
 * Passed:  (1) sclass - original storage class for variable            *
 *          (2) genty - The gentyp for the variable                     *
 * Returns: func-code. Unchanged if not register variable               *
 *          C_REG_U if RegClsDpth was 0 (no prev reg var declared)      *
 *              and var is INT, LONG, or anything not a pointer         *
 *          C_REG_Y if RegClsDpth was < 0 ???                           *
 *          FT_AUTO on anything else                                    *
 *          Increments RegClsDpth by one                                *
 * ******************************************************************** */

static int
#ifdef __STDC__
set_regclass (int sclass, int genty)
#else
set_regclass (sclass, genty)
    int sclass;
    int genty;
#endif
{
    if (sclass == FT_REGISTER)       /* else L886b */
    {
        if (RegClsDpth < 1)  /* else L8866 */
        {
            switch (genty)
            {
                default:    /* L8836 */
                    if ( ! ispointer (genty))
                    {
                        return FT_AUTO;
                    }

                case FT_INT:     /* L8841 */
                case FT_UNSIGNED:
                    /* C_REG_Y if RegClsDpth was < 0 ...
                     * When would this happen??
                     */

                    return ((++RegClsDpth == 1) ? C_REG_U : C_REG_Y);
            }
        }

        return FT_AUTO;
    }

    return sclass;
}

/* **************************************************************** *
 * dofunction () - Process a single function                        *
 *          On entry, the function name, and all varnames within    *
 *          the parentheses have been processed, as well as the     *
 *          closing parenthesis.                                    *
 *          We are now at the parameter definitions or the opening  *
 *          brace.                                                  *
 * **************************************************************** */

static void
#ifdef __STDC__
dofunction (LBLDEF *my_lbl, int funcftyp)
#else
dofunction (my_lbl, funcftyp)
LBLDEF *my_lbl;
int funcftyp;
#endif
{
    int stk_offset;
    int rg_fncode;
    LBLDEF *d0027_sav;
    char *_lbl_nam;
    int _lblnum;
    register LBLDEF *curnt_lbl;

    InFunction = 1;
    D0013 = RegClsDpth = StkTot = BlkStkReq = StkUsed = 0;

    /* Get parameter definitions (anything up to the L Bracket) */

    while (D003f != C_LBRACE)     /* L888f */
    {
        func_param ();
    }

    _lbl_nam = my_lbl->fnam;

    if (DoProfil)
    {
        prtprofil (_lbl_nam, (_lblnum = ++LblNum));
    }

    func_prolog (_lbl_nam, (funcftyp != FT_STATIC), _lblnum);

    /* Now process local variables */

    curnt_lbl = D0027;
    stk_offset = 4;     /* We already have return and u on stack */

    while (curnt_lbl)
    {
        int __varsiz;     /* L88df */

        curnt_lbl->lbl_nbr = stk_offset;
        
        switch (curnt_lbl->gentyp)
        {
            case FT_LONG:      /* L88e9 */
            case FT_FLOAT:
                __varsiz = FLOATSIZ;
                break;
            case FT_DOUBLE:      /* L88ee */
                __varsiz = DBLSIZ;
                break;
            case FT_CHAR:      /* L88f3 */
                ++(curnt_lbl->lbl_nbr);
            default:
                __varsiz = INTSIZ;
                break;
        }

        switch ((rg_fncode = curnt_lbl->fnccode))
        {
            case C_REG_U:   /* L8921 */
            case C_REG_Y:
#ifdef COCO
                gencode (rg_fncode, stk_offset);
#else
                gencode (rg_fncode, stk_offset, 0, 0);
#endif
                break;
            case FT_PARAM:    /* L8930 */
                curnt_lbl->fnccode = FT_AUTO;     /* 13 */
                break;
        }

        stk_offset += __varsiz;
        curnt_lbl = curnt_lbl->lblprev;
    }

    d0027_sav = D0027;
    D0027 = 0;

    do_block ();       /* go process commands within block */

    finishfunc (&d0027_sav);
    finishfunc (&LocLblLast);

    /* do a return */

    if (ProgFlow != FT_RETURN)
    {
        gencode ( FT_RETURN, 0, 0 NUL1);
    }

    ProgFlow = 0;
    prtstkreq ();   /* print stack requirement */
    InFunction = 0;

    if (D003f == -1)
    {
        reprterr ("function unfinished");
    }

    nxt_word ();
}

/* ******************************************************************** *
 * do_block () - Process a block level.                                 *
 * ******************************************************************** */

void
#ifdef __STDC__
do_block (void)
#else
do_block ()
#endif
{
    struct blockvartbl *_thisvar;
    LBLDEF *v2;
    int _stkorig;

    v2 = LastLbl;
    LastLbl = 0;
    nxt_word ();
    ++InFunction;
    _stkorig = StkTot;

    /* Process any block-level variables */

    while (is_sc_specifier () || is_vardef ())        /* L89dc */
    {
        blockvardef ();
    }

    if (BlkStkReq > StkTot)
    {
        BlkStkReq = StkTot;
    }

    StkUsed = prt_rsrvstk (StkTot);  /* go "leas ..."  if needed */  /* L89f7 */

    /* Build cmdrefs for each initialized variable local to block
     * and chain them together
     */

    while (BlkIniFrst)       /* L8a75 */
    {
        register CMDREF *__cref;

        _thisvar = BlkIniFrst;     /* L8a05 */
        __cref = _thisvar->bvarcmd;

        __cref = add_cmdref ( C_USRLBL,
                               0, 0,                    /* cr_Left, _Right */
                               (int)_thisvar->mylbl,    /* value           */
                             __cref->_cline, __cref->_lpos);

        __cref = add_cmdref ( C_EQUAL,
                               __cref,                  /* cr_Left   */
                               _thisvar->bvarcmd,       /* cr__Right */
                               0,                       /* value     */
                             __cref->_cline, __cref->_lpos);

        CCREFtoLeft (L6f83 (L111d (__cref)));
        _thisvar = _thisvar->ss_nxt;        /* _thisvar is tmp storage */
        BlkIniFrst->ss_nxt = CurBlkVar;     /* prev's next = CurBlkVar */
        CurBlkVar = BlkIniFrst;             /* CurBlkVar = current bv  */
        BlkIniFrst = _thisvar;              /* ++BlkIniFrst, see above */
    }

    /* Process each command in the block
     * Continue until either a right-brace is encountered or error
     */

    while ((D003f != C_RBRACE) && (D003f != -1))
    {
        do_command ();
    }

    finishfunc (&LastLbl);
    LastLbl = v2;
    --InFunction;
    StkTot = _stkorig;

    StkUsed = (ProgFlow != FT_RETURN) ? prt_rsrvstk (_stkorig) : _stkorig;
}

static void
#ifdef __STDC__
do_parentheses (LBLDEF **dst_lbldef)
#else
do_parentheses (dst_lbldef)
    LBLDEF **dst_lbldef;
#endif
{
    LBLDEF *prevlbl;            /* Previous label defined in loop */
    register LBLDEF *curnt_lbl;

    *dst_lbldef = 0;


    do    /* Loop for each comma-separated variable defined on this line */
    {
        nxt_word ();

        if (D003f == C_RPAREN)        /* else L8b45 */
        {
            break;
        }

        if (D003f == C_USRLBL)      /* else L8b38 */
        {
            curnt_lbl = (LBLDEF *)LblVal;

            if (curnt_lbl->fnccode == FT_PARAM)
            {
                reprterr ("named twice");
            }
            else
            {
                if (curnt_lbl->gentyp)
                {
                    /* Previously used label name.  Save old parameters
                     * and reuse LBLDEF
                     */

                    null_lbldef (curnt_lbl);
                }
            }

            curnt_lbl->gentyp = 1;     /* L8b09 */
            curnt_lbl->fnccode = FT_PARAM;
            curnt_lbl->isfunction = 1;             /* Flag "In Function" */
            curnt_lbl->vsize = INTSIZ;      /* Default size */
            
            if (*dst_lbldef)
            {
                prevlbl->lblprev = curnt_lbl;
            }
            else
            {
                *dst_lbldef = curnt_lbl;    /* L8b29 */
            }

            curnt_lbl->lblprev = 0;
            prevlbl = curnt_lbl;
            nxt_word ();
        }
        else
        {
            noidentf ();
        }

    } while (D003f == C_COMMA);          /* end for (;;) loop */

    lookfor (C_RPAREN);     /* L8b45 */
}

/* **************************************************************** *
 * getSC_word () - Get the word following a storage class           *
 *      definition.  I.E. if it's a storage class, (extern,         *
 *      static, auto, etc,), get the following word(s), which       *
 *      define the variable name                                    *
 * Returns: If a storage class, the original FT_type                *
 *              (modified if DP)                                    *
 *          NULL if it we weren't at a storage classification word  *
 *              DP data = old FT_ + 2                               *
 * nxt_word () has also been run to retrieve the data type          *
 * **************************************************************** */

static int 
#ifdef __STDC__
getSC_word (void)
#else
getSC_word ()
#endif
{
    int _sc_fty;

    /* If it's a storage class, i.e. extern, static, auto, etc */

    if (is_sc_specifier ())     /* else L8b9a */
    {
        _sc_fty = LblVal;      /* FT_type */
        nxt_word ();

        /* Direct Page reference = old FT_ + 2 */

        if ((D003f == C_BUILTIN) && (LblVal == FT_DIRECT))    /* else L8b96 */
        {
            switch (_sc_fty)
            {
                case FT_STATIC:    /* L8b7d */
                    _sc_fty =  FT_DPSTATIC;
                    break;
                case FT_EXTERN:    /* L8b82 */
                    _sc_fty = FT_DPXTRN;
                    break;
            }

            nxt_word ();
        }

        return _sc_fty;
    }
    else
    {
        return 0;
    }
}

/* ******************************************************************** *
 * do_vartype () - process a variable type definition                   *
 * On entry, we have the first word in a variable definition.  It could *
 * be something like "short", or "long", (optionally followed by "int"  *
 * or simply "int", or "char", or a user-defined type                   *
 *                                                                      *
 * Returns: *siz (in calling function) = size of variable or whatever   *
 *          Builtins (except struct/union):                             *
 *              Updated FT_type for the variable (lbl_fttyp)            *
 *              sets p2  (in calling function) to 0                     *
 *          Struct/Union:                                               *
 *              If defining variable for existing struct/union:         *
 *                  return is FT_STRUCT                                 *
 *                  strctmbr (in caller) = structLBLDEF->mbrlist        *
 * Upon return, we have the variable name in the nxt_word() vars.       *
 * ******************************************************************** */

int 
#ifdef __STDC__
do_vartype (int *siz, struct brktdef **typdefpt, struct memberdef **strctmbr)
#else
do_vartype (siz, typdefpt, strctmbr)
    int *siz;
    struct memberdef **typdefpt;
    struct memberdef **strctmbr;
#endif
{
    LBLDEF *_strctlbl;
    int _strtotsz;
    int _mmbrsiz;
    int __mbr_fty;
    int _old_struct;
    int one_gentyp;
    struct brktdef *mbr_typdef;
    int one_siz;                /* size of one of a comma-separated var */
    struct memberdef *nwstruct;
    struct memberdef *v6;
    struct memberdef *_last_mbr;
    int lbl_fttyp = 0;
    int _varsiz = INTSIZ;

    register LBLDEF *regptr;

    *strctmbr = 0;

    /* On entry here, we have some type of label.  It's either a
     * data type (int, char, etc), or a function name */

    if (D003f == C_BUILTIN)             /* else L8ebc */
    {
        switch ((lbl_fttyp = LblVal))    /* L8e7a */
        {
            case FT_SHORT:              /*  */
                lbl_fttyp = FT_INT; /* Promote to "int" fall through to next */

            case FT_UNSIGNED:           /* L8bd1 */
                nxt_word ();

                
                if ((D003f == C_BUILTIN) && (LblVal == FT_INT))
                {               /* else L8ee4 */
                    nxt_word ();
                }

                break;

            case FT_CHAR:      /* L8bea */
                _varsiz = CHARSIZ;
            case FT_INT:      /* L8c22 */
                nxt_word ();
                break;

            case FT_LONG:      /* L8bef */
                nxt_word ();
                _varsiz = LONGSIZ;
                
                /* Check for "long int" or "long float" */

                if (D003f == C_BUILTIN)
                {
                    if (LblVal == FT_INT)
                    {
                        nxt_word ();
                    }
                    else
                    {
                        if (LblVal == FT_FLOAT)
                        {
                            /* "long float" means double */
                            lbl_fttyp = FT_DOUBLE;
                            _varsiz = DBLSIZ;
                            nxt_word ();
                        }
                    }
                }
                break;

            case FT_DOUBLE:      /* L8c13 */
                lbl_fttyp = FT_DOUBLE;
                _varsiz = DBLSIZ;
                nxt_word ();
                break;

            case FT_FLOAT:      /* L8c1d */
                _varsiz = FLOATSIZ;
                nxt_word ();
                break;

            default:     /* L8c28 */
                lbl_fttyp = 0;
                break;

                /* Begin struct */
            case FT_UNION:      /* L8c2f */
            case FT_STRUCT:
                _varsiz = _strtotsz = 0;
                ++Struct_Union;     /* Go into "struct mode" for FindLbl() */
                _strctlbl = 0;
                nxt_word ();        /* get struct name or "{" */
                --Struct_Union;     /* Get back out of "struct mode" */

                /* Process struct name if applicable */

                if (D003f == C_USRLBL)          /* We have a struct name */
                {                               /* else L8cc8 */
                    _strctlbl = (LBLDEF *)LblVal;      /* Pointer to LBLDEF */

                    if ( ! (_strctlbl->gentyp))
                    {
                        /* FindLbl sets gentyp to 0 for new labels
                         * Therefore, we have a new struct def...
                         * Set it up
                         */

                        _strctlbl->gentyp = G_STRCNAM;
                        _strctlbl->fnccode = FT_LONG;     /* 8 */
                    }
                    else
                    {
                        /* In this case, we need to have
                         * "struct <existing-struct> varname
                         * If it's not a struct, report error
                         */

                        if (_strctlbl->fnccode != FT_LONG)
                        {
                            reprterr ("name clash");
                        }
                    }
                    
                    /* Next "word" is either variable name or "{" */
                    
                    nxt_word ();        /* L8c86 */

                    if (D003f != C_LBRACE)
                    {
                        /* something like "struct somestruct structname" */

                        if (_strctlbl->gentyp == G_STRCT)
                        {
                            *siz = _strctlbl->vsize;
                            *strctmbr = _strctlbl->mbrlist;
                            return FT_STRUCT;
                        }                        
                        else
                        {
                                    /* cast correct?? */
                            *siz = (int)_strctlbl;   /* L8cb0 */
                            return G_STRCNAM;
                        }
                    }

                    /* We are now at a LBracket.  If the following is true,
                     * we are trying to define a new struct or union with
                     * a struct name we have already defined as a struct
                     */

                    if (_strctlbl->gentyp == G_STRCT)
                    {
                        multdef ();
                    }
                }       /* end process struct name */

                if (D003f != C_LBRACE)    /* L8cc8 */
                {
                    reprterr ("struct syntax");
                    break;
                }

                /* At this point, we have the brace.  We are now
                 * ready to start defining each struct member
                 */

                ++Struct_Union;        /* L8cde */

                do           /* Loop for each struct member */
                {
                    /* Temp reset Struct_Union to non-struct to get member
                     * def
                     */

                    _old_struct = Struct_Union;    /* L8ce5 */
                    Struct_Union = 0;
                    nxt_word ();
                    Struct_Union = _old_struct;

                    if (D003f == C_RBRACE)    /* else L8e4d */
                    {
                        break;
                    }

                    /* Process member - recurse into self (this function)
                     * This is equivalent to doing any other type, except
                     * it's within a struct/union.
                     */

                    __mbr_fty = do_vartype ( &_mmbrsiz,
                                             &mbr_typdef,
                                             &_last_mbr);

                    /* At this point, __mbr_fty = FT_code for member variable
                     * D003f contains C_type for the character or name
                     */
                    /* process comma-list variable name(s) for the type */
                    
                    while (1)
                    {
                        struct brktdef *_my_mbr;
                        LBLDEF *membrlbl;

                        _my_mbr = mbr_typdef;

                        if (D003f == C_SEMICOLON)  /* Done */  /* else L8e37 */
                        {
                            break;
                        }

                        ++InFunction;
                        one_gentyp = proc_varname ( &membrlbl,
                                                  &_my_mbr,
                                                  __mbr_fty);
                        regptr = membrlbl;
                        --InFunction;

                        if (regptr == 0) /* else L8d5c */
                        {
                            noidentf ();
                            goto comma_or_brk;
                        }

                        if (regptr->gentyp)     /* L8d5c */   /* else L8d91 */
                        {
                            if (regptr->isfunction == InFunction)   /*else L8d8a */
                            {
                                if ( (regptr->gentyp != one_gentyp)   ||
                                     (regptr->fnccode  != FT_STRCMBR) ||
                                     (regptr->lbl_nbr != _strtotsz)      )
                                {
                                    reprterr ("struct member mismatch");
                                }
                            }
                            else
                            {
                                /* copy regptr to G18Current, null regptr */
                                null_lbldef (regptr);     /* L8d8a */
                            }
                        }

                        if ( one_gentyp == G_STRCNAM) /* L8d91 */
                        {
                            reprterr ("undefined structure");
                        }

                        regptr->gentyp = one_gentyp;
                        regptr->fnccode = FT_STRCMBR;
                        regptr->lbl_nbr = _strtotsz;
                        regptr->mbrlist = _last_mbr;
                        
                        if ((one_siz = setlblsize (regptr, _my_mbr, _mmbrsiz)))
                        {       /* else L8def */
                            if (lbl_fttyp == FT_STRUCT)
                            {
                                _varsiz = (_strtotsz += one_siz);
                            }
                            else    /* union - set size to bigger of two */
                            {
                                _varsiz = (one_siz > _varsiz ? one_siz :
                                                               _varsiz   );
                            }
                        }
                        else
                        {
                            sizundef ();
                        }

                        regptr->isfunction = InFunction;    /* L8df2 */
                        regptr->lblprev = LastLbl;
                        LastLbl = regptr;

                        if (lbl_fttyp == FT_STRUCT)    /* else L8e2f */
                        {
                            nwstruct = addmem (sizeof (struct memberdef));
                            nwstruct->mmbrlbl = regptr;
                            
                            if (*strctmbr)
                            {
                                v6->mbrPrev = nwstruct;
                            }
                            else
                            {
                                *strctmbr = nwstruct;    /* L8e21 */
                            }

                            v6 = nwstruct;
                        }
comma_or_brk:
                        if (D003f != C_COMMA)
                        {
                            break;
                        }

                        nxt_word ();
                        continue;

                    }   /* End of while (1) loop */
                } while (D003f == C_SEMICOLON);  /* L8e43 */

                --Struct_Union;    /* L8e4d */

                if (_strctlbl)
                {
                    _strctlbl->vsize = _varsiz;
                    _strctlbl->gentyp = G_STRCT;
                    _strctlbl->mbrlist = *strctmbr;
                }

                lookfor (C_RBRACE);
                break;              /* End struct def */
        }
    }
    else        /* NOT BUILTIN */
    {
        /* typedef */

        /* L8ebc */
        if ( (D003f == C_USRLBL)                                    &&
             (((regptr = (LBLDEF *)LblVal)->fnccode) == FT_TYPEDEF)
           )
        {
            *siz = regptr->vsize;
            *typdefpt = regptr->arrmbrs;
            *strctmbr = regptr->mbrlist;

            nxt_word ();
            return regptr->gentyp;
        }
    }

    *siz = _varsiz;     /* First parameter  */
    *typdefpt = 0;      /* Second parameter */

    return lbl_fttyp;
}

/* ******************************************************************** *
 * proc_varname () - Processes an ft_type and returns the gentyp        *
 *          reflecting the pointer/brace/bracket depth                  *
 * Passed:  (1) parnt_lbldef - POINTER to caller's LBLDEF *             *
 *          (2) ptr to brktdef *  in parent                             *
 *          (3) FT_type of current "word" (for local use)               *
 * Returns: gentyp , if not USRLBL                                      *
 * Affects: if USRLBL, parnt_lbldef (1) points to var's LBLDEF          *
 *                      else returns NULL                               *
 *          If array, parentmdf is chained onto array defs here         *
 *                      else returns unchanged                          *
 * ******************************************************************** */

int
#ifdef __STDC__
proc_varname ( LBLDEF **parnt_lbldef,
             struct brktdef **parntbrkt,
             int parnt_ftyp )
#else
proc_varname (parnt_lbldef, parntbrkt, parnt_ftyp)
    LBLDEF **parnt_lbldef;
    struct brktdef **parntbrkt;
    int parnt_ftyp;
#endif
{
    int _ptrdpth,
        _strctstate;
    struct brktdef *prev_mbr;
    int _bracdpth;
    struct brktdef *frst_mbr;
    LBLDEF *_lblptr;

    /* Null parnt_lbldef - if it's not a USRLBL, it will remain nulled
     * on return
     */

#ifdef COCO
    _ptrdpth = *parnt_lbldef = 0;
#else
    _ptrdpth = 0;
    *parnt_lbldef = 0;
#endif

    /* Get past any pointer states (if any) */

    while (D003f == C_ASTERISK)    /* Increase pointer count */
    {
        _ptrdpth = incptrdpth (_ptrdpth);
        nxt_word ();
    }

    if (D003f == C_USRLBL)
    {
        *parnt_lbldef = (LBLDEF *)LblVal;        /* variable name's LBLDEF * */
        nxt_word ();
    }
    else
    {
        /* This is where we're NOT doing a function parameter list
         * Something like a cast, or parentheses for priority, etc
         */

        if (D003f == C_LPAREN)
        {
            nxt_word ();
            ++InFunction;        /* insure we're flagged NOT PARAMETER */
            parnt_ftyp = proc_varname (parnt_lbldef, parntbrkt, parnt_ftyp);
            --InFunction;        /* restore parameter status to original */
            lookfor (C_RPAREN);
        }
    }

    /* I think _here_ is a function's parameter list... */

    if (D003f == C_LPAREN)        /* else L8fa1 */
    {
        _ptrdpth = ((_ptrdpth << 2) + 0x30);

        if ( ! InFunction) /* Function parameter list ??? */
        {
            do_parentheses (&D0027);
        }
        else
        {   /* Parenthesized variable */
            do_parentheses (&_lblptr);
            finishfunc (&_lblptr);        /* go to L0924 */
        }
    }
    else        /* NOT C_LPAREN */
    {
        register struct brktdef *__member;

#ifdef COCO
        frst_mbr = _bracdpth = prev_mbr = 0;       /* L8fa1 */
#else
        frst_mbr = prev_mbr = 0;
        _bracdpth = 0;
#endif

        _strctstate = Struct_Union;
        Struct_Union = 0;

        /* Array definition - each loop gets an array dimension (bracket) */

        while (D003f == C_LBRKET)     /* L9008 */
        {
            /* increase brket depth, read next word, and
             * add a brktdef struct to memory
             */

            _ptrdpth = (_ptrdpth << 2) + 0x20;      /* L8fb4 */
            nxt_word ();
            __member = addmem (sizeof (struct brktdef));

            /* Read element count and store into ->elcount*/

            if ((_bracdpth == 0) && (D003f == C_RBRKET))
            {
                /* simply a "[]" */

                __member->elcount = 0;  
            }
            else            /* L8fde */
            {
                __member->elcount = getconst (0);
            }

            if (prev_mbr)     /* L8fe7 + 1 */
            {
                prev_mbr->brPrev = __member;
            }
            else
            {
                frst_mbr = __member;    /* L8ff3 */
            }

            prev_mbr = __member;
            lookfor (C_RBRKET);
            ++_bracdpth;
        }           /* end "while (D003f == C_LBRKET" */

        Struct_Union = _strctstate; /* restore original status */

        /* If we had an array, chain the member defs to
         * any prev array def we had on entry (if any
         */

        if (frst_mbr)
        {
            __member->brPrev = *parntbrkt;
            *parntbrkt = frst_mbr;
        }
    }

    return (mrgdepths (parnt_ftyp, _ptrdpth));
}

/* **************************************************************** *
 * mrgdepths () - adds new pointer, brace, and parentheses shifts   *
 *          onto the original ft_type.  It does this by creating a  *
 *          tmp ftyp, then r-shifting with a corresponding L-shift  *
 *          of the new depthflag until the old ftyp shows no more   *
 *          depths.  then the new, shifted flag is added (actually  *
 *          it amounts to or-ing it into the unused MSB of the old  *
 *          ftyp.                                                   *
 * Passed:  (1) bare ft_type                                        *
 *          (2) combined pointer, brace, and parentheses shift      *
 *              flags                                               *
 * Returns: The updated ft_type which will become a gentype in      *
 *          most cases.                                             *
 * **************************************************************** */

static int
#ifdef __STDC__
mrgdepths(int ftype, int depthflgs)
#else
mrgdepths (ftype, depthflgs)
    int ftype;
    int depthflgs;
#endif
{
    int _tmpftyp = ftype;

    while (_tmpftyp & 0x30)     /* while any-depth in old ftyp */
    {
        _tmpftyp >>= 2;         /* shift next flag into position */
        depthflgs <<= 2;        /* shift new depths left two bytes */ 
    }

    return (ftype + depthflgs);
}

/* ******************************************************************** *
 * setlblsize () - Sets vsize in LBLDEF to correct size                 *
 *                  Note: I believe that LBLDF may be a CMDREF at times *
 *                  but I think that the second member of each is the   *
 *                  size of the variable                                *
 *      Also calls get_varsize () and returns the total size for the    *
 *      variable.                                                       *
 * Returns : total size for variable.
 * ******************************************************************** */

int
#ifdef __STDC__
setlblsize (register LBLDEF *lbldf, struct brktdef *mbrdf, int dfltsiz)
#else
setlblsize (lbldf, mbrdf, dfltsiz)
    register LBLDEF *lbldf;
    struct brktdef *mbrdf;
    int dfltsiz;
#endif
{
    int _gentyp;
    int _vsize;

    switch ((_gentyp = lbldf->gentyp) & 0x0f)
    {
        case FT_CHAR:      /* L9082 */
            _vsize = CHARSIZ;
            break;
        case FT_INT:      /* L9087 */
        case FT_UNSIGNED:
            _vsize = INTSIZ;
            break;
        case FT_LONG:      /* L908c */
        case FT_FLOAT:
            _vsize = LONGSIZ;
            break;
        case FT_DOUBLE:      /* L9091 */
            _vsize = DBLSIZ;
            break;
        case FT_UNION:      /* L9096 */
        case FT_STRUCT:
            _vsize = dfltsiz;
            break;
        case 10:        /* structure */     /* L909a */
            _vsize = 0;
            break;
    }

    /* Set vsize in LBLDEF * */

    if (_vsize)     /* L90d3 */
    {
        lbldf->vsize = _vsize;
    }
    else
    {
        lbldf->vsize = dfltsiz;
    }

    return get_varsize (_gentyp, _vsize, (lbldf->arrmbrs = mbrdf));
}

/* ******************************************************************** *
 * get_varsize () get the real size for the variable.                   *
 * Passed:  (1) ft_Ty for the variable                                  *
 *          (2) default size if nothing fits                            *
 *          (3) LBLDEF *                                                *
 * Returns: INTSIZ for pointers or parentheses                          *
 *          Total size for an array                                     *
 *          "dfltsiz" for anything else                                 *
 * ******************************************************************** */

/* NOTE: Some calls to this function seem to use LBLDEF *'s.
 * Research to see if the struct members used by this function
 * (offsets 0 and 4) correspond
 */

int
#ifdef __STDC__
get_varsize (int ftyp, int dfltsiz, struct brktdef *braceptr)
#else
get_varsize (ftyp, dfltsiz, braceptr)
    int ftyp;
    int dfltsiz;
    register struct brktdef *braceptr;
#endif
{
    int _arrcount;

    if ((ispointer (ftyp)) || (inparentheses (ftyp)))
    {
        return INTSIZ;   /* L9117 */
    }

    if (inbrkets (ftyp))    /* Array */       /* L911d */   /* else L916a */
    {
        _arrcount = 1;

        do
        {
            _arrcount *= (int)(braceptr->elcount);    /* L912d */
            braceptr = braceptr->brPrev;
        } while (inbrkets (ftyp = (MSBrshft2 (ftyp))));

        return (_arrcount * ((ispointer (ftyp)) ? INTSIZ : dfltsiz));
    }
    else
    {
        return dfltsiz;
    }
}

void
#ifdef __STDC__
finishfunc (LBLDEF **p1)
#else
finishfunc (p1)
    LBLDEF **p1;
#endif
{
    LBLDEF *v62;   /* Some type of pointer to a structure */
    LBLDEF *v60;
    char __tmpstr[60];

    register LBLDEF *regptr = *p1;

    while (regptr)      /* L922c */
    {
#ifndef COCO
        int *_mmbr = (int *)(&regptr->mbrlist);
#endif
        v60 = regptr->lblprev;

        /* goto label not defined in current function */
#ifdef COCO
        if ((regptr->gentyp == C_GOTO) && ! ((int)regptr->mbrlist & 1))
#else
        if ((regptr->gentyp == C_GOTO) && ! (*_mmbr & 1))
#endif
        {
            reprterr (strncat (strcpy (__tmpstr, "label undefined : "),
                        (regptr->fnam), LBLLEN));
        }

        switch (regptr->fnccode)
        {
            case C_REG_U:       /* L91c4 */
            case C_REG_Y:
                --RegClsDpth;
                break;
        }

                    /* L91d9 */
        if (((unsigned int)(v62 = (LBLDEF *)(regptr->ftop)) >
                (unsigned int)LblPtrLow) &&
                ((unsigned int)v62 < (unsigned int)LblPtrEnd))  /* else L91f4 */
        {
            /* Make regptr the base of the current LblTree ?? */
            fill_g18 (regptr);      /* go to L9229 */
        }
        else
        {
            if (regptr == *((LBLDEF **)v62))          /* L91f4 */
            {
                *((LBLDEF **)v62) = regptr->fnext;
            }
            else
            {
                /* Just to get it to compile */
                v62 = *((LBLDEF **)v62);     /* L9202 */

                /* WHOLE LOOP STILL NOT RIGHT !!!!! */

                while (regptr != v62->fnext)
                {
                    /* Just to get it to compile */
                    v62 = v62->fnext;
                }
            
                v62->fnext = regptr->fnext;
            }

            regptr->fnext = D0062;      /* L9222 */
            D0062 = regptr;
        }

        regptr = v60;
    }       /* end "while" */

    *p1 = 0;
}

static void
#ifdef __STDC__
sizundef (void)
#else
sizundef ()
#endif
{
    reprterr ("cannot evaluate size");
}

void
#ifdef __STDC__
noidentf (void)
#else
noidentf ()
#endif
{
    reprterr ("identifier missing");
}
