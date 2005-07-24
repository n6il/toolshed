/* part5.c - for "rma"  */

#include "rma.h"

/*
 * extern direct int  PgDepth, PgWidth, NumBytes; extern direct char
 * *Lbr_Ptr, *Rmb3_Ptr; extern direct FILE *SrcFile; extern direct unsigned
 * d0027; extern direct long nmbr_int; extern direct int  doList, dGlbls,
 * edatas, ddatas; extern direct char **Glbls, **S_Addrs, **Adrs_Ptr; extern
 * direct char Pass2, d005e, d005f, *SrcChar; extern direct int  d0051,
 * *d0053; extern direct short comn_cnt; extern direct char d0060, d0062,
 * *Label, *operatr, *oprand, *coment; extern direct int ((**jmp_ptr)())[];
 * extern direct char *CodTbBgn, *CodTbEnd, *d007c; extern direct struct
 * symblstr d0092, d00a6; extern direct char ListFlg, C_Flg, OFlg, ff_flg,
 * g_flg, e_flg, S_Flg, x_flg, R_Keep; extern direct char listON; extern
 * direct int  CmdCod;
 * 
 * extern direct char MLCod.opcode; extern direct union storer1 *opr_ptr; extern
 * direct int MacFlag, d00d2, d00d4, macfp2;
 * 
 * extern direct struct symblstr *SmblDsc;
 * 
 * extern CODFORM S_Psec, Data_Equ, srmb3; extern JMPTBL  drectives[], j_secs,
 * d035c[]; extern FILE **ThisFile; extern char _ttl[]; extern struct
 * symblstr d0587; extern struct ref_ent d0791[], *OptBPtr;
 */

extern long     reptFpos;

struct optns    cmdopts[] = {
	{'l', &ListFlg},
	{'o', &OFlg},
	{'c', &C_Flg},
	{'f', &ff_flg},
	{'g', &g_flg},
	{'e', &e_flg},
	{'s', &S_Flg},
	{'x', &x_flg},
	{'k', &R_Keep}
};

/*
 * d0387[3], d038a[3], d038d[3], d0390[3], d0393[3], d0396[3], d0399[3];
 */

#ifdef __STDC__

int
                do_rzb(void)
#else

do_rzb()
#endif
{
	if (l1723())
	{
		while (nmbr_int--)
		{
			l18ed(0);
		}
		return 1;
	}
	return 0;
}

#ifdef __STDC__

int
                l16f3(void)
#else

l16f3()
#endif
{
	if (l1723())
	{
		*(Adrs_Ptr = &d0051) = l174f();
		if (Label)
		{
			l2069(d005f & 0xfe);
			l20a4(d0051);
		}
		l175f(d0051 + nmbr_int);	/* check out to see if long
						 * needed */
		return 1;
	}
	return 0;
}

#ifdef __STDC__

int
                l1723(void)
#else

l1723()
#endif
{
	register struct symblstr *smbl_tmp = SmblDsc;
	int             flag = 0;	/* CHECK THIS OUT .. I THINK it's
					 * right */

	if (getNmbr())
	{
		l143e();
		if (OptBPtr == d0791)
		{
			flag = 1;
		}
		else
		{
			nmbr_int = ilExtRef();
		}
	}
	SmblDsc = smbl_tmp;
	return flag;
}

#ifdef __STDC__

unsigned int
                l174f(void)
#else

unsigned        l174f()
#endif
{
	if (d0053 == 0)
	{
		return 0;
	}
	else
	{
		return (*d0053);
	}
}

#ifdef __STDC__

int
                l175f(int parm)
#else

l175f(parm)
	int             parm;

#endif
{
	if (d0053)
	{
		d0053[0] = parm;
	}
	return parm;
}


#ifdef __STDC__

int
                do_equ(void)
#else

do_equ()
#endif
{
	return (l1780(LT_EQU));
}

#ifdef __STDC__

int
                do_set(void)
#else

do_set()
#endif
{
	return (l1780(LT_SET));
}

#ifdef __STDC__

int
                l1780(int parm)
#else

l1780(parm)
	int             parm;

#endif
{
	register struct symblstr *reg;

	if (Label == 0)
		return e_report("label missing");
	l2069(parm);
	reg = SmblDsc;
	if (getNmbr() == 0)
	{
		return 0;
	}
	SmblDsc = reg;
	l143e();
	l20a4((short) nmbr_int);
	Adrs_Ptr = &nmbr_int;	/* Gotta fake it here, first pt to long value */
#ifdef COCO
	++Adrs_Ptr;		/* no??? then bump to second half go get an
				 * int     */
#endif
	return 1;
}

#ifdef __STDC__

int
                do_comon(void)
#else

do_comon()
#endif
{
	if (Label == 0)
	{
		return e_report("label missing");
	}
	if (l1723() == 0)
	{
		return 0;
	}
	l2069(7);
	SmblDsc->w1 |= 4;
	if (Pass2)
	{
		++comn_cnt;
	}
	if (SmblDsc->s_ofst < nmbr_int)
	{
		SmblDsc->s_ofst = nmbr_int;
	}
	Adrs_Ptr = &nmbr_int;
#ifdef COCO
	++Adrs_Ptr;
#endif
	return 1;
}

#ifdef __STDC__

int
                set_fcc(void)
#else

set_fcc()
#endif
{
	return (l180e(1));	/* return may not be needed */
}

#ifdef __STDC__

int
                set_fcs(void)
#else

set_fcs()
#endif
{
	return (l180e(2));	/* return may not be needed */
}

#ifdef __STDC__

int
                l180e(int parm)
#else

l180e(parm)
	int             parm;

#endif
{
	char           *var1;
	char            var2,
	                var3;

	if ((var2 = *SrcChar) != '\0')
	{
		var1 = SrcChar;
		while ((var3 = *(++var1)) != '\0')
		{
			if (var3 == var2)
			{
				break;
			}
		}
		if (var3 != '\0')
		{
			if ((var1 -= 2) >= SrcChar)
			{
				while (SrcChar < var1)
				{
					l18ed(*(++SrcChar));
				}
				var3 = *(++SrcChar);
				if (parm == 2)
				{
					var3 |= 0x80;
				}
				l18ed(var3);
			}
			SrcChar += 2;
			return 1;
		}
		SrcChar = var1;
	}
	cnstDef();
}

#ifdef __STDC__

int
                cnstDef(void)
#else

cnstDef()
#endif
{
	e_report("constant definition");
}

#ifdef __STDC__

int
                set_fcb(void)
#else

set_fcb()
#endif
{

	/*
	 * CmdCod = &(nmbr_int) + 1; CmdCod = &nmbr_int; CmdCod +=3;
	 *//* Why not 1??? *//* because nmbr_int is Long */
	CmdCod = nmbr_str;
	l18cd(l2387, 1);
}

int             l2369();

#ifdef __STDC__

int
                set_fdb(void)
#else

set_fdb()
#endif
{

	/*
	 * CmdCod = &nmbr_int; CmdCod +=2;
	 *//* Why???  because nmbr_int is Long */
	CmdCod = nmbr_str;
	l18cd(l2369, 2);
}

#ifdef __STDC__

int
                l18cd(int (*adr) (void), int cnt)
#else

l18cd(adr, cnt)
	int             (*adr) ();
	int             cnt;

#endif
{

	for (;;)
	{
		NumBytes = cnt;
		(*adr) ();
		switch (cnt)
		{
		case 1:
			*nmbr_str = (char) nmbr_int;
			break;
		case 2:
			storInt(nmbr_str, nmbr_int);
			break;
		default:
			break;
		}
		if (*SrcChar != ',')
		{
			return 1;
		}
		l1911();
		++SrcChar;
	}
}

#ifdef __STDC__

int
                l18ed(int parm)
#else

l18ed(parm)
	char            parm;

#endif
{
	if (NumBytes >= 4)
	{
		l1911();
	}
	*(char *) (CmdCod + (NumBytes++)) = parm;
}

/* FILE OK TO HERE */

#ifdef __STDC__

int
                l1911(void)
#else

l1911()
#endif
{
	w_ocod(CmdCod, NumBytes);
	if (doList)
	{
		ListLine();
		if (Pass2 && listON && (ListFlg > 0) && (g_flg > 0))
		{
			doList = 1;
		}
		else
		{
			doList = 0;
		}
		Label = operatr = oprand = coment = 0;
		if (S_Addrs)
		{
			d0051 = *S_Addrs;
		}

	}
	NumBytes = 0;
}

#ifdef __STDC__

int
                do_os9(void)
#else

do_os9()
#endif
{
	NumBytes = 3;
	l2387();		/* SEE IF parameter is needed */
	MLCod.opcode = 16;
	opr_ptr[0] = 0x3f;
	opr_ptr[1] = nmbr_int;
	l1911();
	return 1;
}

#ifdef __STDC__

int
                iscomma(void)
#else

iscomma()
#endif
{
	if (*SrcChar == ',')
	{
		++SrcChar;
		return 1;
	}
	else
	{
		e_report("comma expected");
	}
}

#ifdef __STDC__

int
                l198d(void)
#else

l198d()
#endif
{
	l1723();
	if (S_Addrs)
	{
		*S_Addrs = nmbr_int;
	}
}

/*
 * This following null routine is ??????    . It is listed in one of the
 * JMPTBL entries. It might be a deleted function.. -w  maybe ???
 * Inspect later...
 */
#ifdef __STDC__

int
                l199e(void)
#else

l199e()
#endif
{
}

#ifdef __STDC__

int
                do_nam(void)
#else

do_nam()
#endif
{
	copy_to(d0591);
}

#ifdef __STDC__

static
                copy_to(	/* copy a string to dest_str */
			                char *dest_str
)
#else

static          copy_to(dest_str)	/* copy a string to dest_str */
	char           *dest_str;

#endif
{
	register char  *dst = dest_str;

	if (Pass2 || (dst[0] == '\0'))
	{
		while (*(dst++) = *(SrcChar++))
		{
		};
		--SrcChar;
	}
	return 1;
}

#ifdef __STDC__

int
                do_ttl(void)
#else

do_ttl()
#endif
{
	copy_to(_ttl);
}

#ifdef __STDC__

int
                do_pag(void)
#else

do_pag()
#endif
{
	prnthdr();
	doList = 0;
}

#ifdef __STDC__

int
                do_spc(void)
#else

do_spc()
#endif
{
	if (_isnum())
	{
		while (nmbr_int--)
		{
			new_pag();
		}
		doList = 0;
	}
}

#ifdef __STDC__

int
                _isnum(void)
#else

_isnum()
#endif
{
	if (_getnum())
	{
		return 1;
	}
	else
	{
		e_report("bad number");
	}
}

#ifdef __STDC__

int
                set_opts(void)
#else

set_opts()
#endif
{
	int             ch_opt;
	char            t_char;
	register struct optns *tcmd;

	t_char = SkipSpac();

	for (;;)
	{
		if ((ch_opt = (t_char != '-')) == 0)
		{
			ch_opt = -1;	/* to turn OFF */
			t_char = *(++SrcChar);
		}
		else		/* not needed?? */
			ch_opt = 1;	/* to turn ON option */

		t_char = _tolower(t_char);
		tcmd = &cmdopts[0];

		while (tcmd < endof(cmdopts))
		{
			if (t_char == tcmd->optn)
			{
				*tcmd->optadr += ch_opt;
				++SrcChar;	/* Bump for nxt chr (fixed
						 * from orig) */
				break;
			}
			++tcmd;
		}
		if (tcmd >= endof(cmdopts))
		{
			switch (t_char)
			{
			case 'd':
				++SrcChar;
				if (_isnum())
				{
					PgDepth = nmbr_int;
				}
				else
				{
					return (bad_opt());
				}
				break;
			case 'w':
				++SrcChar;
				if (_isnum())
				{
					PgWidth = nmbr_int;
				}
				else
				{
					return (bad_opt());
				}
				break;
			case 'v':
				++SrcChar;
				if (_isnum() && (nmbr_int >= 0) && (nmbr_int < 2))
				{
					rversion = nmbr_int;
				}
				else
				{
					return (bad_opt());
				}
				break;
			default:
				return (bad_opt());
			}
		}

		/*
		 * Original pre-bump here moved up to while... loop above to
		 * make d-w-v opts work correctly
		 */
		if (*SrcChar != ',')
		{
			break;
		}
		t_char = *(++SrcChar);
	}
	return 1;
}

#ifdef __STDC__

static
                bad_opt(void)
#else

static          bad_opt()
#endif
/* l1ad9()     */
{
	e_report("bad option");
}

#ifdef __STDC__

int
                l1ae2(void)
#else

l1ae2()
#endif
{
	if (getNmbr())
	{
		l143e();
	}
	if ((OptBPtr = d0791))
	{
		d0027 = nmbr_int;
		Adrs_Ptr = &nmbr_int;
#ifdef COCO
		++Adrs_Ptr;
#endif
		return 1;
	}
	ilExtRef();
}

#ifdef __STDC__

int
                do_use(void)
#else

do_use()
#endif
{
	char            var;

	*(ThisFile++) = SrcFile;
	SrcFile = u_opn(SrcChar, "r");
	while (var = *(++SrcChar))
	{
		if (var == ' ' || var == '\t')
		{
			break;
		}
	}
	return 1;
}

#ifdef __STDC__

int
                _isnul(void)
#else

_isnul()
#endif
{
	return (nmbr_int == 0);
}

#ifdef __STDC__

int
                not_nul(void)
#else

not_nul()
#endif
{
	return (nmbr_int != 0);
}

#ifdef __STDC__

int
                is_neg(void)
#else

is_neg()
#endif
{
	return (nmbr_int < 0);
}

#ifdef __STDC__

int
                le_zero(void)
#else

le_zero()
#endif
{
	return (nmbr_int <= 0);
}

#ifdef __STDC__

int
                ge_zero(void)
#else

ge_zero()
#endif
{
	return (nmbr_int >= 0);
}

#ifdef __STDC__

int
                is_pos(void)
#else

is_pos()
#endif
{
	return (nmbr_int > 0);
}

#ifdef __STDC__

int
                is_pass1(void)
#else

is_pass1()
#endif
{
	return (PASS1);
}

#ifdef __STDC__

int
                set_vsec(void)
#else

set_vsec()
#endif
{
	CodTbBgn = Data_Equ;
	CodTbEnd = DtaEqEnd;	/* &s_rmb3 */
	jmp_ptr = &d035c[2];
	d007c = &d00a6;
	d005e = 1;
	d0062 = 0;
	S_Addrs = &edatas;
	d0053 = &Glbls;
	if (is_dp())
	{			/* if vsect "dp" */
		d007c = &d0092;
		d005e = 3;
		d0062 = 16;
		S_Addrs = &ddatas;
		d0053 = &dGlbls;
	}
	d0051 = *S_Addrs;	/* CHECK OUT */
	return 1;
}

#ifdef __STDC__

int
                is_dp(void)
#else

is_dp()
#endif
{
	char            var;

	if (((var = _toupper(*SrcChar)) != '\0') && (var == 'D') &&
	    (_toupper(SrcChar[1]) == 'P'))
	{

		if ((SrcChar[2] == '\0') || (SrcChar[2] == ' ') || (SrcChar[2] == '\t') )
		{
			SrcChar += 2;
			return 1;
		}
		else
		{
			e_report("DP section ???");
		}
	}
	return 0;
}

#ifdef __STDC__

int
                setsect(void)
#else

setsect()
#endif
{
	NumBytes = 0;
	CodTbBgn = S_Psec;
	CodTbEnd = PSecEnd;

	jmp_ptr = j_secs;
	d007c = 0;
	d005e = 4;
	d0060 = 0xa0;
	S_Addrs = d0053 = 0;
	d0062 = '\0';
	return 1;
}

#ifdef __STDC__

int
                tel_fail(void)
#else

tel_fail()
#endif
{
	e_report((oprand == 0) ? "fail" : oprand);
}

#ifdef __STDC__

int
                do_rept(void)
#else

do_rept()
#endif
{
	if (reptFpos)
	{
		e_report("nested REPT");
	}
	else
	{
		if (getNmbr() == 0)
		{
			return 0;
		}
		l143e();
		if (OptBPtr != d0791)
		{
			return ilExtRef();
		}
		if ((d00d2 = nmbr_int) != 0)
		{
			reptFpos = ftell(MacFlag ? macfp2 : SrcFile);
			return 1;
		}
		d00d4 = 1;
		return 1;
	}
}

#ifdef __STDC__

int
                end_rept(void)
#else

end_rept()
#endif
{
	if (d00d4)
	{
		d00d4 = 0;
		return 1;
	}
	if (reptFpos == 0)
	{
		e_report("ENDR without REPT");
	}
	else
	{
		if (--d00d2 > 0)
		{
			fseek((MacFlag ? macfp2 : SrcFile), reptFpos, 0);
			doList = 0;
		}
		else
		{
			reptFpos = 0;
		}
		return 1;
	}
}
