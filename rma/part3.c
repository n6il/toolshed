/* Part3.c */

#include "rma.h"

/*
 * extern direct int  NumBytes, doList, Glbls, dGlbls, edatas, ddatas,
 * InMacro; extern direct char *CmdCod, AIMFlg; extern direct short d0033;
 * 
 * extern direct char     *Rmb_Ptr, *S_Rmbend, *Nam_Ptr, *Cod_End, C_Flg,
 * *_If_Dpth, *_IfIsTru, *d002d; extern direct long nmbr_int; extern direct
 * char     *d0049, **S_Addrs, *CodeSize, **Adrs_Ptr, d0053, Pass2, d005e,
 * MLCod.prebyte, d0060, d0062, d0063, *SrcChar, *Label, *d006a, CodTbBgn,
 * *CodTbEnd, *d007c, MLCod.opcode; extern direct struct symblstr d007e;
 * 
 * extern direct CODFORM   *Cod_Ent;
 * 
 * extern direct union storer1 *opr_ptr;
 * 
 * extern struct ref_str d0791[], *OptBPtr;
 * 
 * extern direct JMPTBL *jmp_ptr; extern JMPTBL CnstTbl[], d03b6[], _if_tbl[];
 * 
 * extern CODFORM S_Bsr, S_Lbr, S_Rmb, Data_Equ, S_Rmb3, S_Nam;
 * 
 * extern char LabelName[];
 */

int             is_pass1();

#ifdef __STDC__

int
                l_brnch(void)
#else

l_brnch()
#endif
/* lbra & lbsr */
{
	NumBytes = 3;
	l0ff5();
}

#ifdef __STDC__

int
                do_aim(void)
#else

do_aim()
#endif
/* aim, oim, tim, eim  */
{

	char            acod = MLCod.opcode,
	                itm;

	AIMFlg = 1;
	if (cc_stuff())
	{			/* first, get memory byte */
		if (*SrcChar == ',')
		{
			++SrcChar;
			++(char *) opr_ptr;	/* Bump union ptr by one byte */
			/* to allow for extra byte    */
			return bit_stuf();
		}
	}
	return 0;
}

#ifdef __STDC__

int
                do_band(void)
#else

do_band()
#endif
{
	char            bitpchr,
	                namhold[10],
	               *sr_tmp;
	unsigned        bitpos;
	int             count;
	char           *boff = "01234567CVZNIHFE";

	NumBytes = 4;

	switch (getreg(0))
	{
	case 0x08:		/* accA */
		opr_ptr[0] |= 0x40;
		break;
	case 0x09:		/* accB */
		opr_ptr[0] |= 0x80;
		break;
	case 0x0a:		/* CC   */
		break;		/* default */
	default:
		return bd_rgnam();
	}
	if (*SrcChar == '.')
	{
		bitpchr = *(++SrcChar);
		if (islower(bitpchr))
		{
			bitpchr = _toupper(bitpchr);
		}
		if (bitpos = (int) strchr(boff, bitpchr))
		{
			++SrcChar;
			bitpos -= (int) boff;
			if (((opr_ptr[0]) && (bitpos < 8)) || !(opr_ptr[0]))
			{
				if (bitpos > 7)
				{
					bitpos -= 8;
				}
				opr_ptr[0] |= bitpos;
				if (*SrcChar == ',')
				{
					++SrcChar;
					if (*SrcChar == '<')
					{
						++SrcChar;
					}
					count = 0;
					while ((count < 9) && (namhold[count] = *(SrcChar++)) &&
					       (namhold[count] != '.'))
					{
						++count;
					}
					namhold[count] = '\0';
					sr_tmp = --SrcChar;	/* Tmp save SrcChar */
					SrcChar = namhold;
					if (getNmbr() == 0)
					{
						SrcChar = sr_tmp;
						return (e_report("bad number for band operation"));
					}
					SrcChar = sr_tmp;
					if ((nmbr_int > -256) && (d0033 < 256))
					{
						d0033 = nmbr_int;
						l143e();
						l1389();
						opr_ptr[1] = d0033;
						if (*SrcChar == '.')
						{
							++SrcChar;
							if (bitpos = (int) strchr(boff, *SrcChar))
							{
								++SrcChar;
								bitpos -= (int) boff;
								if (bitpos < 8)
								{
									opr_ptr[0] |= (bitpos << 3);
									return 1;
								}
							}
						}
					}
				}
			}
		}
	}
	return ilAdrMod();
}

#ifdef __STDC__

int
                cc_stuff(void)
#else

cc_stuff()
#endif
/* orcc, andcc, & cwai */
{
	NumBytes = 2;
	if (immediat())
	{
		d0063 |= 8;
		l2387();
		opr_ptr[0] = nmbr_int;
		return 1;
	}
	ilAdrMod();
}

#ifdef __STDC__

int
                q_immed(void)
#else

q_immed()
#endif
/* does q register (needed for long usage on immediate mode */
{
#ifndef COCO
	register char  *nmptr;
	register int    count;

#endif

	if (immediat())
	{
		NumBytes += 4;
		do_long();
#ifdef COCO
		opr_ptr->q_long = nmbr_int;
#else
		/* nmptr = &nmbr_int; */

		/*
		 * (int *)nmptr = &nmbr_int; for( count =0; count < 4;
		 * count++ ) opr_ptr[count] = nmptr[count];
		 */
		storInt(opr_ptr, (nmbr_int >> 16) & 0xffff);	/* save 16 MSB */
		storInt(&(opr_ptr[2]), nmbr_int & 0xffff);	/* and 16 LSB */
#endif
		l0fe0();
	}
	else
	{
		MLCod.prebyte = 0x10;	/* all other modes require
					 * MLCod.prebytee */
		CmdCod = &MLCod.prebyte;
		++NumBytes;
		MLCod.opcode = 0xcc;	/* Opcode is also different */
		regofst();
	}
}

#ifdef __STDC__

int
                do_md(void)
#else

do_md()
#endif
{
	if (!immediat())
	{
		return 0;
	}
	++NumBytes;
	l2387();
	opr_ptr[0] = nmbr_int;
	return 1;
}

#ifdef __STDC__

int
                int_stuf(void)
#else

int_stuf()
#endif
/* add's etc for 16-bit stuff */
{
#ifndef COCO
	register char  *nmptr;

#endif

	if (immediat())
	{
		NumBytes += 2;
		l2369();
#ifdef COCO
		opr_ptr->C_Int[0] = nmbr_int;
#else

		/*
		 * nmptr = (char *)(&nmbr_int); nmptr += 2;
		 *//* to pt to "int" part */

		/*
		 * opr_ptr[0] = *(nmptr++); opr_ptr[1] = *nmptr;
		 */
		storInt(opr_ptr, nmbr_int);
#endif
		l0fe0();
	}
	else
	{
		regofst();
	}
}

/* add, sub, cmp, ld */

/*
 * for special cases that include "E" & "F" regs if "E" or "F" not found,
 * falls through to chr_stuf()
 */
#ifdef __STDC__

int
                chr_ef(void)
#else

chr_ef()
#endif
/* 8-bit processes */
{
	int             lngreg = 0;

	switch (*d006a)
	{
	case 'e':
		*(--CmdCod) = 0x11;
		++NumBytes;
		break;
	case 'f':
		*(--CmdCod) = 0x11;
		++NumBytes;
		MLCod.opcode |= 0x40;
		break;
	default:
		return (chr_stuf());	/* then check for standard regs */
	}
	return poprnd(lngreg);
}

/* sbc,and, bit, st, eor, adc, etc */
#ifdef __STDC__

int
                chr_stuf(void)
#else

chr_stuf()
#endif
/* 8-bit processes */
{
	int             lngreg = 0;

	switch (*d006a)
	{
	case 'd':
		*(--CmdCod) = 0x10;
		++NumBytes;
		lngreg = 1;
		break;
	case 'b':
		MLCod.opcode |= 0x40;
		break;
	case 'a':
		break;
	default:
		return (bd_rgnam());
	}
	return poprnd(lngreg);
}

#ifdef __STDC__

int
                poprnd(int _lngreg)
#else

poprnd(_lngreg)
	int             _lngreg;

#endif
{
#ifndef COCO
	register char  *nmptr;

#endif


	if (immediat())
	{
		++NumBytes;
		if (_lngreg)
		{
			++NumBytes;
			l2369();
#ifdef COCO
			opr_ptr->C_Int[0] = nmbr_int;
#else

			/*
			 * nmptr = (char *)(&nmbr_int); nmptr += 2;
			 * opr_ptr[0] = *(nmptr++); opr_ptr[1] = *nmptr;
			 */
			storInt(opr_ptr, nmbr_int);
#endif
		}
		else
		{
			d0063 |= 8;
			l2387();
			opr_ptr[0] = nmbr_int;
		}
		return l0fe0();
	}
	else
	{
		return regofst();
	}
}

#ifdef __STDC__

int
                do_divd(void)
#else

do_divd()
#endif
{
	return poprnd(0);
}

#ifdef __STDC__

int
                bit_stuf(void)
#else

bit_stuf()
#endif
{
	if ((MLCod.opcode != 1) && !AIMFlg && (MLCod.opcode != 5) && (MLCod.opcode != 0x0b) &&
	    (MLCod.opcode != 0x0e) && *d006a)
	{
		switch (*d006a)
		{
		case 'w':
			/* If not neg, asr, lsl, asl */
			if (MLCod.opcode && (MLCod.opcode != 7) && (MLCod.opcode != 8))
			{
				MLCod.opcode |= 0x50;
				*(--CmdCod) = 0x10;
				++NumBytes;
				break;
			}
		case 'd':
			MLCod.opcode |= 0x40;
			*(--CmdCod) = 0x10;
			++NumBytes;
			break;
		case 'a':
			MLCod.opcode |= '\x40';
			break;
		case 'b':
			MLCod.opcode |= '\x50';
			break;
		case 'e':
		case 'f':
			if ((MLCod.opcode == 3) || (MLCod.opcode >= 0x0a))
			{
				MLCod.opcode |= ((*d006a == 'e') ? 0x40 : 0x50);
				*(--CmdCod) = 0x11;
				++NumBytes;
				break;
			}
		default:
			return (bd_rgnam());
		}
		return 1;
	}
	if (regofst())
	{
		if (MLCod.opcode & 0xe0)
		{
			MLCod.opcode |= 0x40;
		}
		return 1;
	}
	return 0;
}

#ifdef __STDC__

int
                no_opcod(void)
#else

no_opcod()
#endif
/* No opcode, just a null subroutine */
{
	return 1;
}

#ifdef __STDC__

int
                lea_s(void)
#else

lea_s()
#endif
/* leax, leas, etc. */
{
	if (regofst())
	{
		return 1;
	}
	else
	{
		ilAdrMod();
	}
}

#ifdef __STDC__

int
                do_tfm(void)
#else

do_tfm()
#endif
{
	int             rg0,
	                rg1;
	char            nxtc;
	int             sgn0,
	                sgn1;

	sgn0 = sgn1 = 0;
	NumBytes += 1;
	if (((rg0 = getreg(0)) >= 0x100) && (rg0 <= 0x104) &&
	    (((nxtc = *SrcChar) == ',') || (nxtc == '+') || (nxtc == '-')))
	{
		if (nxtc != ',')
		{
			++SrcChar;
			if (nxtc == '+')
			{
				++sgn0;
			}
			else
			{
				--sgn0;
			}
			if (*SrcChar != ',')
			{
				return bdrglist();
			}
		}
		++SrcChar;
		if (((rg1 = getreg(0)) >= 0x100) && (rg1 <= 0x104))
		{
			if (((nxtc = *SrcChar) == '+') || (nxtc == '-'))
			{
				++SrcChar;
				if (nxtc == '+')
				{
					++sgn1;
				}
				else
				{
					--sgn1;
				}
			}
			if (sgn0 || sgn1)
			{
				if ((sgn0 > 0) && (sgn1 > 0))
				{
					MLCod.opcode = 0x38;
				}
				else
				{
					if ((sgn0 < 0) && (sgn1 < 0))
					{
						MLCod.opcode = 0x39;
					}
					else
					{
						if ((sgn0 > 0) && (sgn1 == 0))
						{
							MLCod.opcode = 0x3a;
						}
						else
						{
							if ((sgn0 == 0) && (sgn1 > 0))
							{
								MLCod.opcode = 0x3b;
							}
							else
							{
								return bdrglist();
							}
						}
					}
				}
			}
			opr_ptr[0] = (rg0 << 4 | rg1);
			return 1;
		}
	}
	return bdrglist();
}

#ifdef __STDC__

int
                tfr_exg(void)
#else

tfr_exg()
#endif
{
	int             rg0,
	                rg1;

	NumBytes = 2;
	if ((rg0 = getreg(0)) && (*SrcChar == ','))
	{
		++SrcChar;
		if (rg1 = getreg(0))
		{
			if (((rg0 ^ rg1) & 8) == 0)
			{
				opr_ptr[0] = (rg0 << 4 | rg1);
				return 1;
			}
			else
			{
				return (e_report("register size mismatch"));
			}
		}
	}
	return bdrglist();
}

#ifdef __STDC__

int
                do_addr(void)
#else

do_addr()
#endif
{
	int             rg0,
	                rg1;

	NumBytes = 3;
	if ((rg0 = getreg(0)) && (*SrcChar == ','))
	{
		++SrcChar;
		if (rg1 = getreg(0))
		{
			if (rg1 != 0x0a)
			{
				opr_ptr[0] = (rg0 << 4 | rg1);
				return 1;
			}
			else
			{
				return (e_report("storing to CCR"));
			}
		}
	}
	return bdrglist();
}

#ifdef __STDC__

int
                stk_stuf(void)
#else

stk_stuf()
#endif
/* Psh.. pul.. on/off stacks */
{
	int             var2;

	NumBytes = 2;
	SkipSpac();
	while (var2 = getreg(1))
	{
		opr_ptr[0] |= var2;
		if (*SrcChar != ',')
		{
			return 1;
		}
		++SrcChar;
	}
	bdrglist();
}

#ifdef __STDC__

int
                lb_(void)
#else

lb_()
#endif
/* Long branches */
{
	NumBytes = 4;
	if (Cod_Ent = FindCmd(&(LabelName[1]), S_Bsr, SBsr_End))
	{
		MLCod.opcode = Cod_Ent->opcode;
		l0ff5();	/* in part4.c */
		return;
	}
	else
	{
		e_report("bad mnemonic");
	}
}

#ifdef __STDC__

int
                do_brnch(void)
#else

do_brnch()
#endif
{
	int             var2;

	NumBytes = 2;
	if (getNmbr())
	{
		if (Pass2)
		{
			var2 = nmbr_int - (unsigned) CodeSize - 2;
			d0063 |= '\x88';
			l143e();
			if (d0791 == OptBPtr)
			{
				if ((var2 > 127) || (var2 < -128))
				{
					var2 = -2;
					e_report("branch out of range");
				}
			}
			else
			{
				l1389();
			}
			opr_ptr[0] = var2;
		}
		return 1;
	}
	return 0;
}

/* equset(): handles fcc's, fcb's , etc */
#ifdef __STDC__

int
                equset(void)
#else

equset()
#endif
{
	NumBytes = 0;
	(*CnstTbl[MLCod.opcode]) ();
}

#ifdef __STDC__

int
                asm_dirct(void)
#else

asm_dirct()
#endif
/* setdp, org, etc */
{
	NumBytes = 0;
	(*d03b6[MLCod.opcode]) ();
}


#ifdef __STDC__

int
                do_ifs(void)
#else

do_ifs()
#endif
/* handles if statememts */
{
	NumBytes = 0;
	++_If_Dpth;
	if (_if_tbl[MLCod.opcode] != is_pass1)
	{
		if (getNmbr() == 0)
		{
			return 0;
		}
		if (d0791 != OptBPtr)
		{
			ilExtRef();
			return;
		}
	}
	if ((*_if_tbl[MLCod.opcode]) () == 0)
	{
		++_IfIsTru;
	}
	doList &= (C_Flg > 0);
	return 1;
}

#ifdef __STDC__

int
                ilExtRef(void)
#else

ilExtRef()
#endif
{
	e_report("illegal external reference");
}

#ifdef __STDC__

int
                do_endc(void)
#else

do_endc()
#endif
{
	NumBytes = 0;
	if (_If_Dpth)
	{
		if (MLCod.opcode == 0)
		{
			--_If_Dpth;
			if (_IfIsTru)
			{
				--_IfIsTru;
			}
		}
		else
		{
			(_IfIsTru) ? --_IfIsTru : ++_IfIsTru;
		}
		doList &= (C_Flg > 0);
		return 1;
	}
	e_report("conditional nesting error");
}

#ifdef __STDC__

int
                _psect(void)
#else

_psect()
#endif
{
	codsetup();
	do_psect();
	CodeSize = 0;
	ddatas = dGlbls = edatas = Glbls = 0;
	return 1;
}

/* sets up for FindCmd() for all CPU directives */
#ifdef __STDC__

int
                codsetup(void)
#else

codsetup()
#endif
{
	NumBytes = 0;
	CodTbBgn = S_Lbr;
	CodTbEnd = SLbrEnd;
	jmp_ptr = codtabl;
	d007c = &d007e;
	d005e = 4;
	d0060 = 0xa2;		/* 162; */
	d0062 = 0x20;
	S_Addrs = &CodeSize;
	d0053 = 0;
	return 1;
}

/* sets up for csect stuff */
#ifdef __STDC__

int
                do_csec(void)
#else

do_csec()
#endif
{
	NumBytes = 0;
	CodTbBgn = S_Rmb;
	CodTbEnd = S_Rmbend;
	jmp_ptr = d035c;
	d007c = 0;
	d005e = 6;
	d0060 = (d0060 & '\x2') | '\x80';
	S_Addrs = (int *) 0;
	d0062 = '\0';
	Adrs_Ptr = d0053 = &d0049;
	d0049 = 0;
	if (SkipSpac())
	{
		if (getNmbr() == 0)
		{
			return 0;
		}
		if (OptBPtr != d0791)
		{
			return ilExtRef();
		}
		d0049 = nmbr_int & 0xffff;
	}
	return 1;
}

#ifdef __STDC__

int
                setrmb3(void)
#else

setrmb3()
#endif
{
	NumBytes = 0;
	CodTbBgn = S_Rmb3;
	CodTbEnd = Rmb3End;
	jmp_ptr = d0364;
	d005e = 0;
	if (is_dp())
	{
		d005e = 2;
	}
	return 1;
}

#ifdef __STDC__

int
                l0ef0(void)
#else

l0ef0()
#endif
{
	Adrs_Ptr = d0053;
	if (d0060 & 2)
	{
		codsetup();
	}
	else
	{
		setsect();
	}
}

/* sets up a macro definition */
#ifdef __STDC__

int
                setmac(void)
#else

setmac()
#endif
{
	if (InMacro)
	{
		nstmacdf();
	}
	else
	{
		if (Label == 0)
		{
			e_report("label missing");
		}
		else
		{
			l2069(8);
			if (PASS1)
			{
				newMac();
			}
			InMacro = 1;
		}
	}
}

#ifdef __STDC__

int
                noMac(void)
#else

noMac()
#endif
{
	e_report("ENDM without MACRO");
}
