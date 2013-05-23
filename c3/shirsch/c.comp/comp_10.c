/* ************************************************************************ *
 * comp_10.c - part 10 for c.comp                                           *
 *                                                                          *
 * This file does much of the output to the destination file                *
 *                                                                          *
 * Comes from p2_06.c                                                       *
 * $Id:: comp_10.c 73 2008-10-03 20:20:29Z dlb                            $ *
 * ************************************************************************ */

#include "ccomp.h"

/* The following variables (at least) should be moved from ccomp.h to here
char *Ofst_y = ",y";
char *Ofst_s = ",s";
char *P_lbsr = "lbsr ";
char *P_lbra = "lbra ";
char *P_clra = "clra";
 */

static int get_regname (
#ifdef __STDC__
    int
#endif
);

static void tfr_regreg (
#ifdef __STDC__
        int, int
#endif
);

static void codgenlong (
#ifdef __STDC__
        int, int
#endif
);

static void codgenfloat (
#ifdef __STDC__
        int, int *
#endif
);

static void const_realfunc (
#ifdef __STDC__
        int *arra, int count
#endif
);

static void call_intfunc (
#ifdef __STDC__
    char *
#endif
);

static void call_longfunc (
#ifdef __STDC__
        char *
#endif
);

static void call_realfunc (
#ifdef __STDC__
        char *
#endif
);

static void do_bitops (
#ifdef __STDC__
    int, int, CMDREF *
#endif
);

static void valtoreg (
#ifdef __STDC__
    int, int, CMDREF *, int
#endif
);

static void prtplusval (
#ifdef __STDC__
    int
#endif
);

static char * br_rel_op (
#ifdef __STDC__
    int
#endif
);

static void prnt_chr (
#ifdef __STDC__
    int
#endif
);

static void lea_reg (
#ifdef __STDC__
    int
#endif
);

static void lea_llblpcr (
#ifdef __STDC__
    int, int
#endif
);

/* This was L3292 in c.pass1 */

/* ******************************************************************** *
 * gencode () - Main code generator.  This function handles most        *
 *              of the code generation, although some is handled in     *
 *              other functions.  Some of the work passed to gencode () *
 *              is passed  on to other functions.                       *
 * Passed:  Parameter count and types vary.  The following              *
 *          prototype is set up, but in many cases, the parameters need *
 *          to be cast to avoid warnings in the non-COCO compilers      *
 * ******************************************************************** */

void
#ifndef __STDC__
gencode (parm1, parm2, parm3, parm4)
    int parm1;
    int parm2;
    int parm3;
    register CMDREF *parm4;
#else
gencode (int parm1, int parm2, int parm3, CMDREF *parm4)
        /*  +12        +14        +16          +18 */
#endif
{
    int _reg_name;
    LBLDEF *var4;
    int _typ;
    int _offset;

    if (parm1 == C_TYLONG)   /* longs */
    {
        codgenlong (parm2, parm3);
        return;
    }

    if (parm1 == C_TYFLOAT)   /* floats or doubles */
    {
        codgenfloat (parm2, (int *)parm3);
        return;
    }

    switch (parm1)
    {
        case 122:          /* L465e */
            fprintf (OutPath, " pshs %c\n", get_regname (parm2));
            
            if ((StkUsed -= INTSIZ) < BlkStkReq)
            {
                BlkStkReq = StkUsed;
            }
            return;
        case 125:          /* L468b */
                /* cast parm 4 to satisfy prototype */
            gencode (129, C_REG_X, C_INT, (CMDREF *)parm3);
            parm3 = parm2;
            parm2 = C_EQEQ;
            /* fall through to next */
        case 130:     /* output "lb<eq,ne,...> <parm3> */      /* L46ae */
            prt_bgnfld ("lb");
            prnt_strng (br_rel_op (parm2));  /* jumping to L479f */
            loclbl_CR (parm3);          /* jumping to L479f */
            return;
        case FT_RETURN:           /* L46c5 */
            if (DoProfil)
            {
                prt_profend ();
            }

            prt_opcode ("puls u,pc\n");  /* jumping to L46cc */
            return;
        case C_AND:
        case C_VBAR:       /* L46d3 */
        case C_CARET:
            do_bitops (parm1, parm3, parm4);
            return;
        case C_MULT:       /* L46e7 */
            call_intfunc ("ccmult");
            return;
        case C_UDIV:           /* L46ed */
            call_intfunc ("ccudiv");
            return;
        case C_SLASH:      /* L46f3 */
            call_intfunc ("ccdiv");
            return;
        case C_LSHIFT:     /* L46f9 */
            call_intfunc ("ccasl");
            return;
        case C_RSHIFT:     /* L46ff */
            call_intfunc ("ccasr");
            return;
        case C_URSHFT:           /* L4705 */
            call_intfunc ("cclsr");
            return;
        case C_UMOD:           /* L470b */
            call_intfunc ("ccumod");
            return;
        case C_PERCENT:    /* L4711 */
            call_intfunc ("ccmod");
            return;
        case C_MINUS:      /* L471d */
            prt_opcode ("nega\n negb\n sbca #0");
            return;
        case C_TILDE:      /* L4724 */
            prt_opcode ("coma\n comb");
            return;
        case FT_GOTO:           /* L472b */
            prt_bgnfld ("leax ");
            prnt_integer (-StkUsed);
            prnt_strng (Ofst_s);
            prntCR ();
        case 124:          /* L4751 */
            prt_bgnfld (P_lbra);
            loclbl_CR (parm2);
            return;
        case C_GOTO:     /* Generate a "goto" dest */            /* L4760 */
            prt_bgnfld (P_lbra);
            loclbl_CR (parm3 = ++LblNum);
            loclbl_CR (parm2);
            prt_bgnfld ("leas ");
            prnt_integer (StkUsed);
            prnt_strng (",x\n");
            loclbl_CR (parm3);
            return;
        case C_PARENS:     /* L47af */
            D0013 = 4;

            if ( (((CMDREF *)parm3)->vartyp == C_USRLBL) &&
                   ((var4 = (LBLDEF *)((CMDREF *)parm3)->cmdval)) )
            {
                prt_bgnfld (P_lbsr);
                prt_label (var4->fnam, 0);
                return;
            }

            prt_bgnfld ("jsr ");
            prt_rgofst (parm2, parm3, 0);
            prntCR ();
            return;
        case C_CHR2INT:    /* L4805 */
            prt_opcode ("sex");
            return;
        case C_LNG2INT:    /* L480b */
            prt_bgnfld ("ld");
            valtoreg ('d', parm2, (CMDREF *)parm3, INTSIZ);
            return;
        case 152:          /* L4830 */
            prt_opcode ("aslb\n rola");  /* jumping to L4840 */
            return;
        case 150:          /* L4836 */
            prt_opcode ("asra\n rorb");  /* jumping to L4840 */
            return;
        case 151:          /* L483c */
            prt_opcode ("lsra\n rorb");
            return;
        case C_REG_Y:    /* L4845 */
            prt_bgnfld ("ldy ");
            goto L4861;
        case C_REG_U:      /* L4854 */
            prt_bgnfld ("ldu ");
L4861:
            prnt_integer (parm2);
            prnt_strng (Ofst_s);
            prntCR ();
            return;
        case C_REGOFST:          /* L487d */
            prt_bgnfld ("leax ");

            switch (parm2)
            {
                case 119:   /* L488c */
                    if ((_reg_name = ((CMDREF *)parm3)->vartyp) == C_isRgY)
                    {
                        _reg_name = 'y';
                    }
                    else
                    {
                        _reg_name = ((_reg_name == C_isRgU) ? 'u' : 'x');
                    }

                    fprintf (OutPath, "%d,%c\n", ((CMDREF *)parm3)->cmdval,
                                                    _reg_name);
                    return;
                case C_REG_D:   /* L48c9 */
                    fprintf (OutPath, "d,%c\n", get_regname (parm3));
                    /* fall through to default */
                default:
                    return;
            }
    }

    _reg_name = get_regname (parm2);

    if (parm3 == 119)       /* else L4a40 */
    {
        if (((CMDREF *)parm4)->vartyp == C_CHR2INT)    /* else L4a23 */
        {
            gencode (parm1, C_REG_D, 119, ((CMDREF *)parm4)->cr_Left);

            switch (parm1)
            {
                case 117:          /* L49ec */
                    prt_opcode ("sex");
                    break;
                case 79:           /* L49f2 */
                case C_PLUS:
                    prt_opcode ("adca #0");
                    break;
                case C_NEG:        /* L49f8 */
                    prt_opcode ("sbca #0");
                    break;
            }

            parm4->vartyp = C_REG_D;
            return;
        }
        else
        {
            if ( (parm4->ft_Ty == FT_CHAR) && (parm1 != 127) &&
                 (_reg_name != 'x')                          )
            {
                _reg_name = 'b';
            }
        }
    }

    switch (parm1)  /* L4a40 */
    {
        case 117:          /* L4a45 */
            if (parm3 == 119)       /* else L4b04 */
            {
                _offset = parm4->cmdval;
                
                switch (_typ = parm4->vartyp)
                {
                    case C_REG_X:          /* L4a5d */
                    case C_REG_Y:
                    case C_REG_U:
                        if (parm2 != C_REG_D)
                        {
                            /* gen "leaRG? _offset,_typ" */
                            lea_reg (_reg_name);
                            fprintf (OutPath, "%d,%c\n",
                                               _offset, get_regname (_typ));
                        }
                        else
                        {
                            /* gen "tfr rg?,d" */
                            tfr_regreg (get_regname (_typ), 'd');  /* L4a82 */

                            if (_offset != 0)
                            {
                                /* gen "addd ...whatever" */
                                gencode (C_PLUS, C_REG_D, C_INT,
                                         /* to satify proto */
                                         (CMDREF *)_offset);
                            }
                        }

                        return;
                    case C_REG_D:          /* L4ab4 */
                        if (parm2 != C_REG_D)
                        {
                            /* gen "tfr d,RG?" */
                            tfr_regreg ('d', _reg_name);
                        }

                        return;
                    case C_DQUOT:      /* L4ac8 */
                        /* gen "leaRG? ?,pc */
                        lea_llblpcr (_reg_name, parm4->cmdval);
                        return;
                    case C_INT:   /* L4ad6 */
                        parm3 = C_INT;
                        parm4 = (CMDREF *)_offset; /* to satify prototype */
                        break;
                }
            }

            /* L4b04 */
            if ((parm2 == C_REG_D) && (parm3 == C_INT) && (parm4 == 0))
            {
                prt_opcode ("clra\n clrb");
            }
            else
            {
                prt_bgnfld ("ld"); /* gen "ldRG? ..." */
                goto L4bc9;
            }

            return;
        case 129:          /* L4b30 */
            if ((parm3 == C_INT) && (parm4 == 0))
            {
                /* Compare a function return with 0 */
                fprintf (OutPath, " st%c -2,s\n", _reg_name);
                return;
            }

            prt_bgnfld ("cmp");

            if (_reg_name == 'b')
            {
                _reg_name = 'd';
            }

            goto L4bc9;
        case 121:          /* L4b6f */
            if ((parm3 == 119) && (isRegU_Y (_typ = parm4->vartyp)))
            {
                if (_typ != parm2)
                {
                    tfr_regreg (_reg_name, get_regname (_typ));
                }

                return;
            }
            else
            {
                prt_bgnfld ("st");
                goto L4bc9;
            }

        case C_NEG:        /* L4ba9 */
            prt_bgnfld ("sub");
            goto L4bc9;
        case 79:           /* L4bb8 */
            gencode (C_MINUS NUL3);
        case C_PLUS:       /* L4bc2 */
            prt_bgnfld ("add");
L4bc9:
            valtoreg (_reg_name, parm3, parm4, 0);
            break;
        case 127:          /* L4bce */
            if (parm3 == 119)   /* else break */
            {
                switch (_typ = ((LBLDEF *)(parm4->cmdval))->fnccode)
                {
                    case FT_DIRECT:    /* L4be3 */
                    case FT_DPXTRN:
                    case FT_DPSTATIC:
                        lea_reg (_reg_name);
                        prnt_chr ('>');

                        if (_typ == FT_DPSTATIC)
                        {
                            prt_loclbl (((LBLDEF *)parm4->cmdval)->lbl_nbr);
                        }
                        else
                        {
                            prt_lblnam (((LBLDEF *)parm4->cmdval)->fnam);
                        }

                        prtplusval (parm4->ptrdstval);
                        prnt_strng (Ofst_y);
                        prntCR ();
                        return;
                }
            }

            prt_bgnfld ("lea");
            valtoreg (_reg_name, parm3, parm4, 0);
            break;

        case 115:          /* L4c5d */
            fprintf (OutPath, " exg %c,%c\n", _reg_name, get_regname (parm3));
            return;
        case C_LEA_RG:          /* L4c7d */
            lea_reg (_reg_name);

            switch (parm3)
            {
                case C_REG_D:           /* L4c8b */
                    prnt_strng ("d,");
                    goto prtregnam;
                case C_INT:    /* L4c9a */
                    prnt_integer ((int)parm4);  /* cast to satify prototype */
                    prnt_chr (',');
prtregnam:
                    prnt_chr (_reg_name);
                    prntCR ();
                    return;
                default:            /* L4cbe */
                    reprterr ("LEA arg");
                    return;
            }
        default:           /* L4cd8 */
            reprterr (UnknOpratr);
            return;
    }
}

static int
#ifndef __STDC__
get_regname (rgcode)
    int rgcode;
#else
get_regname (int rgcode)
#endif
{
    switch (rgcode)
    {
        case C_REG_D:          /* L4d2d */
            return 'd';
        case C_REG_X:          /* L4d32 */
            return 'x';
        case C_REG_Y:   /* C_REG_Y */
            return 'y';
        case C_REG_U:   /* C_REG_U */
            return 'u';
        default:
            return ' ';
    }
}

static void
#ifndef __STDC__
tfr_regreg (rgfrom, rgto)
    int rgfrom;
    int rgto;
#else
tfr_regreg (int rgfrom, int rgto)
#endif
{
    fprintf (OutPath, " tfr %c,%c\n", rgfrom, rgto);
}

/* ******************************************************************** *
 * codgenlong () - Generate code for longs                              *
 * ******************************************************************** */

static void
#ifndef __STDC__
codgenlong (typ, longptr)
    int typ;
    int longptr;
#else
codgenlong (int typ, int longptr)
#endif
{
#ifndef COCO
    int intarr[2];
    int _ofset;
#endif
    switch (typ)
    {
        case C_RgStk:          /* L4d80 */
                /* cast parm 4 to satify prototype */
            gencode (117, C_REG_D, C_isRgX, (CMDREF *)2); /* ldd 2,x */
#ifdef COCO
            gencode (122, C_REG_D);
            gencode (117, C_REG_D, C_isRgX, 0);
            gencode (122, C_REG_D);
#else
            gencode (122, C_REG_D, 0, 0);                 /* "pshs d"  */
            gencode (117, C_REG_D, C_isRgX, 0);           /* "ldd 0,x" */
            gencode (122, C_REG_D, 0, 0);                 /* "pshs d"  */
#endif
            break;
        case 139:          /* L4dd0 */
            prt_opcode ("lda 0,x\n ora 1,x\n ora 2,x\n ora 3,x");
            break;
        case C_BIGMOV:          /* L4ddc */
            call_longfunc ("_lmove");
            StkUsed -= INTSIZ; /* WARNING StkUsed _may_ be an int * */
            break;
        case C_PLUS:       /* L4de2 */
            call_longfunc ("_ladd");
            break;
        case C_NEG:        /* L4de8 */
            call_longfunc ("_lsub");
            break;
        case C_MULT:       /* L4dee */
            call_longfunc ("_lmul");
            break;
        case C_SLASH:      /* L4df4 */
            call_longfunc ("_ldiv");
            break;
        case C_PERCENT:    /* L4dfa */
            call_longfunc ("_lmod");
            break;
        case C_AND:        /* L4e00 */
            call_longfunc ("_land");
            break;
        case C_VBAR:       /* L4e06 */
            call_longfunc ("_lor");
            break;
        case C_CARET:      /* L4e0c */
            call_longfunc ("_lxor");
            break;
        case C_LSHIFT:     /* L4e12 */
            call_longfunc ("_lshl");
            StkUsed -= INTSIZ; /* WARNING StkUsed _may_ be an int * */
            break;
        case C_RSHIFT:     /* L4e18 */
            call_longfunc ("_lshr");
            StkUsed -= INTSIZ; /* WARNING StkUsed _may_ be an int * */
            break;
        case C_EQEQ:
        case C_NOTEQ:
        case C_GT_EQ:
        case C_LT_EQ:      /* L4e2b */
        case C_GT:
        case C_LT:
            call_longfunc ("_lcmpr");
            break;
        case C_MINUS:      /* L4e37 */
            call_longfunc ("_lneg");
            StkUsed -= LONGSIZ;
            break;
        case C_TILDE:      /* L4e3d */
            call_longfunc ("_lcompl");
            StkUsed -= LONGSIZ;
            break;
        case C_I2LNG:      /* L4e43 */
            call_longfunc ("_litol");
            StkUsed -= LONGSIZ;
            break;
        case C_U2LNG:      /* L4e49 */
            call_longfunc ("_lutol");
            StkUsed -= LONGSIZ;
            break;
        case C_PLUSPLUS:   /* L4e4f */
        case C_INCREMENT:
            call_longfunc ("_linc");
            StkUsed -= LONGSIZ;
            break;
        case C_MINMINUS:   /* L4e55 */
        case C_DECREMENT:
            call_longfunc ("_ldec");
            StkUsed -= LONGSIZ;
            break;
        case C_LONG:       /* L4e68 */
#ifndef COCO
            intarr[0] = longptr >> 16;
            intarr[1] = longptr & 0xffff;

            /* sign extend each array element */

            for (_ofset = 0; _ofset < 2; _ofset++)
            {
                intarr[_ofset] = signextend (intarr[_ofset], 2);
            }

            const_realfunc (intarr, LONGSIZ/2);
#else
            const_realfunc (longptr, LONGSIZ/2);
#endif
            break;
        default:           /* L4e74 */
            reprterr ("codgen - longs");
            prnt_strng (UnknOpratr);
            prnt_integer (typ);
            prntCR ();
            break;
    }
}

/* ******************************************************************** *
 * codgenfloat () - Generate code to handle floats                      *
 * Note : in four of the below tests, real_array is compared to         *
 *        FT_FLOAT.  FT_FLOAT is cast to int * to satisfy gcc's         *
 *        prototypes.                                                   *
 * ******************************************************************** */

static void
#ifndef __STDC__
codgenfloat (parm1, real_array)
    int parm1;
    register int *real_array;
#else
codgenfloat (int parm1, int *real_array)
#endif
{
    switch (parm1)
    {
        case C_DOUBLE:     /* L4f6b */
#ifdef COCO
            const_realfunc (real_array, DBLSIZ/2);
#else
            const_realfunc (((struct dbltree *)real_array)->cocoarr, DBLSIZ/2);
#endif
            return;             /* break */
        case C_RgStk:          /* L4f79 */
            call_realfunc ("_dstack");
            StkUsed -= DBLSIZ;
            return;
        case 139:          /* L4f8c */
            fprintf (OutPath, " lda %c,x\n",
                       ((real_array == (int *)FT_FLOAT) ? '3' : '7'));
            return;
        case C_BIGMOV:          /* L4fad */
            call_realfunc ((real_array ==
                                (int *)FT_FLOAT) ? "_fmove" : "_dmove");
            StkUsed += INTSIZ;    /* jump to L5205 */
            return;
        case C_PLUS:       /* L4fc7 */
            call_realfunc ("_dadd");
            StkUsed += DBLSIZ;
            return;
        case C_NEG:        /* L4fcd */
            call_realfunc ("_dsub");
            StkUsed += DBLSIZ;
            return;
        case C_MULT:       /* L4fd3 */
            call_realfunc ("_dmul");
            StkUsed += DBLSIZ;
            return;
        case C_SLASH:      /* L4fd9 */
            call_realfunc ("_ddiv");
            StkUsed += DBLSIZ;
            return;
        case C_EQEQ:       /* L4fdf */
        case C_NOTEQ:
        case C_GT_EQ:
        case C_LT_EQ:
        case C_GT:
        case C_LT:
            call_realfunc ("_dcmpr");
            StkUsed += DBLSIZ;
            return;
        case C_MINUS:      /* L4ff2 */
            call_realfunc ("_dneg");
            return;
        case C_PLUSPLUS:   /* L4ff8 */
        case C_INCREMENT:
            call_realfunc ((real_array == (int *)FT_FLOAT) ? "_finc" : "_dinc");
            return;
        case C_MINMINUS:   /* L500a */
        case C_DECREMENT:
            call_realfunc ((real_array == (int *)FT_FLOAT) ? "_fdec" : "_ddec");
            return;
        case C_TOFLOAT:    /* L5020 */
            call_realfunc ("_dtof");
            return;
        case C_FLT2DBL:    /* L5026 */
            call_realfunc ("_ftod");
            return;
        case C_L2DBL:      /* L502c */
            call_realfunc ("_ltod");
            return;
        case C_I2DBL:      /* L5032 */
            call_realfunc ("_itod");
            return;
        case C_U2DBL:      /* L5038 */
            call_realfunc ("_utod");
            return;
        case C_DBL2LNG:    /* L503e */
            call_realfunc ("_dtol");
            return;
        case C_DBL2INT:    /* L5044 */
            call_realfunc ("_dtoi");
            return;
        default:           /* L5050 */
            reprterr ("codgen - floats");
            prnt_strng (UnknOpratr);
            prnt_integer (parm1);
            prntCR ();
            break;
    }
}

/* **************************************************************** *
 * const_realfunc () - Generate code to call a real (dbl or long)   *
 *          to deal with a constant real or long                    *
 * Passed:  (1) arrptr pointer to array for value of real           *
 *          (2) count (of 16-bit words to generate real )           *
 * **************************************************************** */

static void
#ifndef __STDC__
const_realfunc (arrptr, count)
    int *arrptr;
    int count;
#else
const_realfunc (int *arrptr, int count)
#endif
{
    int _mylblnum;

    prt_bgnfld ("bsr ");
    loclbl_CR (_mylblnum = ++LblNum);
    prnt_words (arrptr, count);
    prt_loclbl (_mylblnum);
    prt_opcode ("puls x");
}

/* **************************************************************** *
 * prnt_words () - Write a set of fdb's.                            *
 * Passed: (1) intarray - the data to print, depending on siz       *
 *         (2) count - the number of words to print.  count == 1,   *
 *             intarray is a single integer, otherwise, if intarray *
 *             is null, it's a string of 'count' nulls.  if not     *
 *             null then intarray is a pointer to an array of ints  *
 * **************************************************************** */

void
#ifndef __STDC__
prnt_words (intarray, count)
    register int *intarray;
    int count;
#else
prnt_words (int *intarray, int count)
#endif
{
    int var0;

    prnt_fdb ();

    if ( count == 1)
    {
        prnt_integer ((int)intarray);
    }
    else
    {
        if (intarray == 0) /* L518b */     /* else L51bc */
        {
            var0 = 1;

            while (var0++ < count)
            {
                prnt_strng ("0,");
            }

            prnt_chr ('0');
        }
        else
        {
            var0 = 0;       /* L51bc */

            while (var0 < count)
            {
                prnt_integer (*(intarray++));

                if ((count - 1) != var0)
                {
                    prnt_chr (',');
                }

                ++var0;
            }
        }
    }

    prntCR ();
}

/* ******************************************************************** *
 * call_intfunc (), call_longfunc(), call_realfunc ()                   *
 *                                                                      *
 *      - Print code to call library functions for the corresponding    *
 *        data type.                                                    *
 * Passed : The name of the library function                            *
 * ******************************************************************** */

static void
#ifndef __STDC__
call_intfunc (fncnam)
    char *fncnam;
#else
call_intfunc (char *fncnam)
#endif
{
    prt_bgnfld (P_lbsr);
    prt_opcode (fncnam);
    StkUsed += INTSIZ;
}

static void
#ifndef __STDC__
call_longfunc (fncnam)
    char *fncnam;
#else
call_longfunc (char *fncnam)
#endif
{
    prt_bgnfld (P_lbsr);
    prt_opcode (fncnam);
    StkUsed += LONGSIZ;
}

static void
#ifndef __STDC__
call_realfunc (strng)
    char *strng;
#else
call_realfunc (char *strng)
#endif
{
    prt_bgnfld (P_lbsr);
    prt_opcode (strng);
}

static void
#ifndef __STDC__
do_bitops (vrtyp, parm2, cref)
    register int vrtyp;
    int parm2;
    CMDREF *cref;
#else
do_bitops (int vrtyp, int parm2, CMDREF *cref)
       /*   +10        +12            +14 */
#endif
{
    char *bitop;
    int var2 = 0;
    int valu;

    if (parm2 == 119)       /* else L5274 */
    {
        var2 = (cref->ft_Ty == FT_CHAR) ? FT_INT : 0;
        parm2 = cref->vartyp;
        cref = (CMDREF *)cref->cmdval;
    }

    /*switch (cref->ft_Ty)*/
    switch (vrtyp)
    {
        case C_AND:        /* L5278 */
            bitop = "and";
            break;
        case C_VBAR:       /* L527e */
            bitop = "or";
            break;
        case C_CARET:      /* L5284 */
            bitop = "eor";
            break;
    }

    switch (parm2)
    {
        case C_USRLBL:     /* L52a0 */
        case C_isRgY:
        case C_isRgU:
        case C_isRgX:
            if (var2)
            {
                if (vrtyp == C_AND)
                {
                    prt_opcode (P_clra);
                }

                prt_bgnfld (bitop);
                valtoreg ('b', parm2, cref, 0);
            }
            else
            {
                prt_bgnfld (bitop);       /* L52c2 */
                valtoreg ('a', parm2, cref, 0);
                prt_bgnfld (bitop);
                valtoreg ('b', parm2, cref, 1);
            }

            break;

        case C_INT:   /* L5308 */

            /* Handle MS-byte */

            switch (valu = ((int)cref >> 8) & 0xff)
            {
                case 0:     /* L5319 */
                    if (vrtyp == C_AND)
                    {
                        prt_opcode (P_clra);
                    }

                    break;
                case 255:   /* L5327 */
                    if (vrtyp == C_AND)
                    {
                        break;
                    }

                    if (vrtyp == C_CARET)
                    {
                        prt_opcode ("coma");
                        break;
                    }

                    /* fall through to default */
                default:    /* L5340 */
                    prt_bgnfld (bitop);
                            /* cast to satify prototype */
                    valtoreg ('a', C_INT, (CMDREF *)valu, 0);
                    break;
            }

            /* Now handle LS-byte */

            switch (valu = ((int)cref & 0xff))        /* L536d */
            {
                case 0:     /* L5376 */
                    if (vrtyp == C_AND)
                    {
                        prt_opcode ("clrb");
                    }

                    break;
                case 255:   /* L5384 */
                    if (vrtyp == C_AND)
                    {
                        break;
                    }

                    if (vrtyp == C_CARET)
                    {
                        prt_opcode ("comb");
                        break;
                    }

                default:    /* L53a0 */
                    prt_bgnfld (bitop);
                        /* cast to satify prototype */
                    valtoreg ('b', C_INT, (CMDREF *)valu, 0);
                    break;
            }

            break;
        case C_RgStk:          /* L53ce */
            fprintf (OutPath, " %sa ,s+\n %sb ,s+\n", bitop, bitop);
            StkUsed += INTSIZ;
            break;
        default:           /* L53ee */
            reprterr ("compiler trouble");
            break;
    }
}

static void
#ifndef __STDC__
valtoreg (regname, parm2, parm3, parm4)
    int regname;
    int parm2;
    int parm3;
    int parm4;
{
    prnt_chr (regname);
    prnt_chr (' ');
    prt_rgofst (parm2, parm3, parm4);
    prntCR ();
}

#else
valtoreg (int regname, int parm2, CMDREF *parm3, int parm4)
{
    /* I have no idea why the following commented-out stuff was there.
     * it seems to have caused things like "lda #xx" to go signed,
     * where standard coco compiler didn't
     * I'll leave it here till I know it isn't necessary
     */

/*
//    int intval;
//
//    if (parm2 == C_INT)
//    {
//        intval = (int)parm3;
//
//        switch (regname)*/    /* sign extend the value */
/*        {
//            case 'a':
//            case 'b':
//                if (intval & 0x80)
//                {
*/                    /* We go through the following convolutions to
//                     * make gcc happy
//                     */
/*
//                    intval |= (-1 ^ 0x7f);
//                    parm3 = (CMDREF *)intval;
//                }
//
//                break;
//            default:
//                if (intval & 0x8000)
//                {
//                    intval |= (-1 ^ 0x7fff);
//                    parm3 = (CMDREF *)intval;
//                }
//                
//                break;
//        }
//    }
*/

    prnt_chr (regname);
    prnt_chr (' ');
    prt_rgofst (parm2, (int)parm3, parm4);
    prntCR ();
}
#endif

/* **************************************************************** *
 * prtplusval () - Prints value to add to another                   *
 * Passed:  Value to add.  If value is 0, nothing function exits    *
 *          without doing anything.                                 *
 * **************************************************************** */

static void
#ifndef __STDC__
prtplusval (numbr, parm2, parm3)
    register int numbr;
#else
prtplusval (int numbr)
#endif
{
    if (numbr)
    {
        if (numbr > 0)
        {
            prnt_chr ('+');
        }

        prnt_integer (numbr);
    }
}

/* **************************************************************** *
 * prt_rgofst () - Handles output for several types                 *
 * Passed: (1) rgcode - variable type                               *
 *         (2) parm2 - Varies, depending on rgcode.                 *
 *             Can be  integer, LBLDEF *, or CMDREF *               *
 *         (3) baseval - A base vaue to print (or offset from)      *
 * **************************************************************** */

void
#ifndef __STDC__
prt_rgofst (rgcode, parm2, baseval)
    int rgcode;
    int parm2;
    int baseval;
#else
prt_rgofst (int rgcode, int parm2, int baseval)
#endif
{
#ifdef COCO
    int var2;   /* unused - can delete when finished debugging */
#endif
    int _lblfncode;

    if (rgcode & 0x8000)    /* Offset addressing */
    {
        prnt_chr ('[');
    }

    switch (rgcode & 0x7fff)
    {
        register CMDREF *cref;

        /* Reuse cref name for coco to save space.  For other compilers,
         * create another var to avoid so much casting
         */

#ifndef COCO
        LBLDEF *_ldef;
#else
#   define _ldef cref
#endif

        case 119:          /* L5492 */
            cref = (CMDREF *)parm2;

            if (cref->vartyp == C_AMPERSAND)
            {
                prnt_chr ('#');
                prt_rgofst (119, (int)(cref->cr_Left), baseval);
            }
            else
            {
                prt_rgofst ( cref->vartyp,
                             cref->cmdval,
                             baseval + cref->ptrdstval);
            }

            return;
        case C_INT:   /* L54c9 */
            prnt_chr ('#');
            prnt_integer (parm2);
            break;
        case C_FLACC:   /* output "_flacc+<baseval>,y */      /* L54d8 */
            prnt_strng ("_flacc");
            prtplusval (baseval);
            prnt_strng (Ofst_y);
            break;
        case C_USRLBL:     /* L54f8 */
            if ((_ldef = (LBLDEF *)parm2))   /* else L5603 (= break) */
            {
                switch (_lblfncode = _ldef->fnccode)
                {
                    case FT_AUTO:      /* L5507 */
                        parm2 = (_ldef->lbl_nbr - StkUsed + baseval);
                        prnt_integer (parm2);
                        prnt_strng (Ofst_s);
                        break;
                    case FT_DPSTATIC:  /* L551d */
                        if ( ! D005a)
                        {
                            prnt_chr ((rgcode & 0x8000) ? '>' : '<');
                        }

                        /* fall through to next case */
                    case FT_STATIC:    /* L5539 */
                        prt_loclbl (_ldef->lbl_nbr);
                        goto plus_p3;
                    case FT_DIRECT:    /* L5546 */
                    case FT_DPXTRN:
                        if ( ! D005a)
                        {
                            /* 0x8000 => offset addressing mode */
                            prnt_chr ((rgcode & 0x8000) ? '>' : '<');
                        }

                        /* fall through to next case */
                    case FT_EXTERN:    /* L5562 */
                    case FT_NONDPDAT:
                        prt_lblnam (_ldef->fnam);
plus_p3:
                        prtplusval (baseval);

                        /* Direct page vars don't use indexed addressing */

                        if ( ! (rgcode & 0x8000))   /* If not offset addr */
                        {
                            if ( (D005a)                     ||
                                 (_lblfncode == FT_DIRECT)   ||
                                 (_lblfncode == FT_DPXTRN)   ||
                                 (_lblfncode == FT_DPSTATIC)     )
                            {
                                break;
                            }
                        }

                        if (inparentheses (_ldef->gentyp))
                        {
                            prnt_strng (",pcr");    /* L55c3 */
                        }
                        else
                        {
                            prnt_strng (Ofst_y);
                        }

                        break;
                    default:           /* L55c9 */
                        reprterr ("storage error");
                        break;
                }
            }       /* end initial "if" */
            else
            {
                prnt_integer (baseval);
            }

            break;
        case C_isRgX:          /* L560d */
        case C_isRgY:
        case C_isRgU:
            prnt_integer (parm2 += baseval);
            prnt_chr (',');

            switch (rgcode & 0x7fff)
            {
                case C_isRgX:   /* L562c */
                    prnt_chr ('x');
                    break;
                case C_isRgY:   /* L5631 */
                    prnt_chr ('y');
                    break;
                case C_isRgU:   /* L5636 */
                    prnt_chr ('u');
                    break;
                default:    /* L56b8 */
                    break;
            }

            break;
        case C_RgStk:          /* L5654 */
            prnt_strng (Ofst_s);
            prnt_strng ("++");
            StkUsed += INTSIZ;
            break;
        default:           /* L5673 */
            reprterr ("dereference");
            break;

            /* Release _ldef name */
#ifdef COCO
#   undef _ldef
#endif
    }

    if (rgcode & 0x8000)
    {
        prnt_chr (']');
    }
}

static char *
#ifndef __STDC__
br_rel_op (vtype)
    int vtype;
#else
br_rel_op (int vtype)
#endif
{
    switch (vtype)
    {
        default:           /* L56d6 */
            reprterr ("rel op");
        case C_EQEQ:       /* L56e1 */
            return "eq ";
        case C_NOTEQ:      /* L56e7 */
            return "ne ";
        case C_LT_EQ:      /* L56ed */
            return "le ";
        case C_LT:         /* L56f3 */
            return "lt ";
        case C_GT_EQ:      /* L56f9 */
            return "ge ";
        case C_GT:         /* L56ff */
            return "gt ";
        case C_U_LTEQ:     /* L5705 */
            return "ls ";
        case C_U_LT:       /* L570b */
            return "lo ";
        case C_U_GTEQ:     /* L5711 */
            return "hs ";
        case C_U_GT:       /* L5717 */
            return "hi ";
    }
}

void
#ifndef __STDC__
prt_bgnfld (opcod)
    char *opcod;
#else
prt_bgnfld (char *opcod)
#endif
{
    putc (' ', OutPath);
    prnt_strng (opcod);
}

/* ******************************************************************** *
 * prt_opcode () - Simply prints string passed as parameter preceded    *
 *                 by a space                                           *
 * ******************************************************************** */

void
#ifndef __STDC__
prt_opcode (opcod)
    char *opcod;
#else
prt_opcode (char *opcod)
#endif
{
    prt_bgnfld (opcod);
    prntCR ();
}

void
#ifndef __STDC__
prntCR ()
#else
prntCR (void)
#endif
{
    putc ('\n', OutPath);
}

/* **************************************************** *
 * prnt_chr () - a putchar() function for OutPath       *
 * **************************************************** */

static void
#ifndef __STDC__
prnt_chr (ch)
    int ch;
#else
prnt_chr (int ch)
#endif
{
    putc (ch, OutPath);
}

/* **************************************************** *
 * prnt_strng () writes the string provided as a        *
 *              to the output file                      *
 * **************************************************** */

void
#ifndef __STDC__
prnt_strng (strng)
    char *strng;
#else
prnt_strng (char *strng)
#endif
{
    fprintf (OutPath, strng);
}

/* ************************************************************ *
 * prnt_integer () - writes the integer value passed as a       *
 *              to the output path                              *
 * ************************************************************ */

void
#ifndef __STDC__
prnt_integer (valu)
    int valu;
#else
prnt_integer (int valu)
#endif
{
    fprintf (OutPath, "%d", valu);
}

/* ******************************************************************** *
 * loclbl_CR () - Prints a local label followed by a carriage return    *
 * Passed:  Label number                                                *
 * Returns: Nothing, but resets ProgFlow to 0                           *
 * ******************************************************************** */

void
#ifndef __STDC__
loclbl_CR (valu)
    int valu;
#else
loclbl_CR (int valu)
#endif
{
    prt_loclbl (valu);
    ProgFlow = 0;
    prntCR ();
}

void
#ifndef __STDC__
prt_loclbl (valu)
    int valu;
#else
prt_loclbl (int valu)
#endif
{
    prnt_chr ('_');
    prnt_integer (valu);
}

void
#ifndef __STDC__
prt_lblnam (lblnam)
    char *lblnam;
#else
prt_lblnam (char *lblnam)
#endif
{
#ifdef COCO
    fprintf (OutPath, "%.8s", lblnam);
#else
    fprintf (OutPath, "%.12s", lblnam);
#endif
}

/* ************************************************************ *
 * prt_rsrvstk () - generates code to move stack downward       *
 * Passed:  valu - positive value to                            * 
 * ************************************************************ */

int
#ifndef __STDC__
prt_rsrvstk (valu)
    int valu;
#else
prt_rsrvstk (int valu)
#endif
{
    int _stksz;

    if ((_stksz = (valu - StkUsed)))
    {
        prt_bgnfld ("leas ");
        prnt_integer (_stksz);
        prnt_strng (Ofst_s);
        prntCR ();
    }

    return valu;
}

/* **************************************************** *
 * prt_label () - prints a label in the label field     *
 * Passed: (1) - label name string                      *
 *         (2) - isglbl if TRUE, prints : after name    *
 * **************************************************** */

void
#ifndef __STDC__
prt_label (lblnam, isglbl)
    char *lblnam;
    int isglbl;
#else
prt_label (char *lblnam, int isglbl)
#endif
{
    prt_lblnam (lblnam);

    if (isglbl)
    {
        prnt_chr (':');
    }

    prntCR ();
}

static void
#ifndef __STDC__
lea_reg (reg)
    int reg;
#else
lea_reg (int reg)
#endif
{
    fprintf (OutPath, " lea%c ", reg);
}

static void
#ifndef __STDC__
lea_llblpcr (regnam, lblnbr)
    int regnam;
    int lblnbr;
#else
lea_llblpcr (int regnam, int lblnbr)
#endif
{
    lea_reg (regnam);
    fprintf (OutPath, "%c%d,pcr\n", '_', lblnbr);
}
