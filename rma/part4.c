/* Part 4 of "rma" assembly */

#define TRUE 1

#include "rma.h"

#define INDX_BYT opr_ptr[0]
/* #define AIM_INDXD if (AIMFlg) {C_Oprnd.opers[0] |= 0x60;} */
#define AIM_INDXD

/*
 * extern direct unsigned NumBytes, jsrOfst, d0021; extern direct long
 * nmbr_int; extern direct short d0033; extern direct int HadBracket, HadArrow,
 * d0039; extern direct unsigned CodeSize, undeflbl;
 * 
 * extern direct short ref_cnt, locl_cnt;
 * 
 * extern direct char  **S_Addrs, Pass2, MLCod.opcode, AIMFlg;
 * 
 * extern direct unsigned d0027; extern direct char     d0063, *SrcChar, *d007c;
 * 
 * extern direct union storer1 C_Oprnd, *opr_ptr;
 * 
 * extern struct ref_ent d0791[], *OptBPtr;
 */

struct regstrct
{
	char            regnam[2],
	                RType;
};

struct regstrct RegCode[] = {
	{{'A', '\0'}, 0x02}, {{'B', '\0'}, 0x04}, {{'C', 'C'}, 0x01},
	{{'D', 'P'}, 0x08}, {{'\xfe', '\0'}, 0}, {{'\xfe', '\0'}, 0},
	{{'E', '\0'}, 0x00}, {{'F', '\0'}, 0x00},
	{{'D', '\0'}, 0x06}, {{'X', '\0'}, 0x10}, {{'Y', '\0'}, 0x20},
	{{'U', '\0'}, 0x40}, {{'S', '\0'}, 0x40}, {{'P', 'C'}, 0x80},
	{{'W', '\0'}, 0x00}, {{'V', '\0'}, 0x00}
};

/* Count of entries in "regstrct" array (6309) */
#define REGCOUNT sizeof(RegCode)/sizeof(RegCode[0])

#ifdef __STDC__

int
                immediat(void)
#else

immediat()
#endif
{
	if (SkipSpac() == '#')
	{
		++SrcChar;
		return 1;
	}
	return 0;
}

#ifdef __STDC__

int
                l0fe0(void)
#else

l0fe0()
#endif
{
	if (jsrOfst & 0x40)
	{
		NumBytes = 3;
		ilAdrMod();
	}
	else
	{
		return 1;
	}
}

l0ff5()
{
	d0063 |= 0x80;
	if (l2369())
	{
#ifndef COCO
		short           newloc;
		register char  *nbrpt = (char *) &newloc;

		newloc = nmbr_int - (int) CodeSize - NumBytes;

		/*
		 * opr_ptr[0] = *(nbrpt++); opr_ptr[1] = *nbrpt;
		 */
		storInt(opr_ptr, newloc);

		if (newloc > 127 || newloc < -128)
		{
#else
		if (((opr_ptr->C_Int[0] = (int) nmbr_int - CodeSize - NumBytes) > 127) ||
		    opr_ptr->C_Int[0] < -128)
		{
#endif
			d0039[0] = 1;	/* CHECK THIS OUT !!! */
		}
		else
		{
			d0039[0] = 0;	/* CHECK THIS OUT !!! */
		}
		return 1;
	}
	return 0;
}

/*
 * getreg(PhsPul) = l102b() : Parses the Register name structure to match up
 * register bit patterns returns: if not PshPul case not register
 * (X,Y,U,S,PC), offset into table | 8 case register, offset-4 | $100 if
 * PshPul push/pull code for register (0 if invalid)
 */


getreg(PshPul)
	int             PshPul;
{
	int             regptr;
	char            var2;

	regptr = 0;
	while (regptr < REGCOUNT)
	{
		if (RegCode[regptr].regnam[0] == _toupper(*SrcChar))
		{
			/* First char matched */
			if ((var2 = RegCode[regptr].regnam[1]))
			{
				/* second char not null, check if it matches */
				if (_toupper(SrcChar[1]) == var2)
				{
					++SrcChar;	/* bump ptr for extra
							 * char */
				}
				else
				{
					++regptr;	/* Bump regptr and get
							 * next */
					continue;
				}
			}
			++SrcChar;
			if ((regptr == 4) || (regptr == 5))
			{
				return 0;	/* Blank areas */
			}
			if (PshPul == 0)
			{
				if (regptr < 8)
				{	/* Not register (X,Y,U,S,PC) */
					return (regptr | 8);
				}
				else
				{
					return ((regptr - 8) | 0x0100);
				}
			}

			/*
			 * No psh/pul for E,F,W or V not needed?? RType
			 * returns correct value??
			 */

			/*
			 * if ( (regptr!=6) && (regptr!=7) && (regptr < 14) )
			 * {
			 */
			return (RegCode[regptr].RType);

			/*
			 * } else { return 0; }
			 */
		}
		++regptr;	/* bump to next positin */
	}
	return 0;		/* No match found */
}

regofst()			/* l10d1()                 */
{				/* Parses indexed mode for A/B/D offset */
	char            itm;

	if (HadBracket = (((itm = SkipSpac()) == '[') ? 1 : 0))
	{
		itm = *(++SrcChar);
	}
	if ((itm = skparrow(itm)) == ',')
	{
		AIM_INDXD;
		return l1215();
	}
	if (SrcChar[1] == ',')
	{
		switch (_toupper(itm))
		{
		case 'A':
			INDX_BYT = 0x86;
			AIM_INDXD;
			return ((_toupper(SrcChar[2]) == 'W') ? ilAdrMod() : ckpcr());
		case 'B':
			INDX_BYT = 0x85;
			AIM_INDXD;
			return ((_toupper(SrcChar[2]) == 'W') ? ilAdrMod() : ckpcr());
		case 'D':
			INDX_BYT = 0x8b;
			AIM_INDXD;
			return ((_toupper(SrcChar[2]) == 'W') ? ilAdrMod() : ckpcr());
		case 'E':
			INDX_BYT = 0x87;
			AIM_INDXD;
			return ((_toupper(SrcChar[2]) == 'W') ? ilAdrMod() : ckpcr());
		case 'F':
			INDX_BYT = 0x8a;
			AIM_INDXD;
			return ((_toupper(SrcChar[2]) == 'W') ? ilAdrMod() : ckpcr());
		case 'W':
			INDX_BYT = 0x8e;
			AIM_INDXD;
			return ((_toupper(SrcChar[2]) == 'W') ? ilAdrMod() : ckpcr());
		}
	}
	getNmbr();
	d0033 = nmbr_int;
	if (*SrcChar == ',')
	{
		AIM_INDXD;
		return l156b();
	}
	else
	{
		/* if  no '[' && dp referencing */
		if ((HadBracket == 0) && (HadArrow >= 0))
		{
			if ((HadArrow > 0) || ((d0033 >> 8) == d0027))
			{
				/* if dp referencing || == DP_blk */
				return l11e8();
			}
		}

		/*
		 * if ( AIMFlg ) { C_Oprnd.opers[0] |= 0x70; }
		 */
		return l11be();
	}
}

skparrow(s_char)
	char            s_char;
{
	if (s_char == '>')
	{
		HadArrow = -1;
	}
	else
	{
		if (s_char == '<')
		{
			HadArrow = 1;
		}
		else
		{
			HadArrow = 0;
		}
	}
	if (HadArrow)
	{
		return (*(++SrcChar));
	}
	else
	{
		return (s_char);
	}
}

ckpcr()
{
	SrcChar += 2;
	if (not_pcr())
	{
		l1326();
	}
	else
	{
		ilIdxReg();
	}
}

l11be()
{
#ifndef COCO
	register int   *nptr;

#endif
	NumBytes += 2;
	if (HadBracket)
	{
		INDX_BYT = 0x9f;/* Let "C_Oprnd" be struct ref_ent */
#ifdef COCO
		opr_ptr->shrtlng.f_int = d0033;	/* for now                   */
#else

		/*
		 * nptr = &d0033; nptr += sizeof(d0033)-2;
		 *//* int part */

		/*
		 * opr_ptr[1] = *(nptr++); opr_ptr[2] = *nptr;
		 */
		storInt(&(opr_ptr[1]), d0033);
#endif
		l1326();
	}
	else
	{
		l143e();
		l1389();
#ifdef COCO
		opr_ptr->C_Int[0] = d0033;
#else

		/*
		 * nptr = &d0033; nptr += sizeof(d0033)-2;
		 *//* int part */

		/*
		 * opr_ptr[0] = *(nptr++); opr_ptr[1] = *nptr;
		 */
		storInt(opr_ptr, d0033);
#endif
		MLCod.opcode |= 0x30;
		return 1;
	}
}

l11e8()
{
	++NumBytes;
	d0063 |= 8;
	l143e();
	l1389();
	INDX_BYT = d0033;
	if (MLCod.opcode & 0xf0)
	{
		MLCod.opcode |= 0x10;
	}
	return 1;
}

l1215()
{
	++SrcChar;
	d0033 = 0;
	if (*SrcChar == '-')
	{
		++SrcChar;
		if (*SrcChar == '-')
		{
			++SrcChar;
			INDX_BYT = 0x83;	/* ,--R */
		}
		else
		{
			l12a2();
			INDX_BYT = 0x82;	/* ,-R  */
		}
		if (not_pcr())
		{
			l1326();
		}
		else
		{
			ilIdxReg();
		}
	}
	else
	{
		if (not_pcr() == 0)
		{
			return pc_rel();
		}
		if (*SrcChar == '+')
		{
			++SrcChar;
			if (*SrcChar == '+')
			{
				++SrcChar;
				if (!Is_W)
				{
					INDX_BYT |= 0x81;	/* ,R++ */
				}
				else
				{
					INDX_BYT |= 0x40;
				}
			}
			else
			{
				if (Is_W)
				{
					return 0;
				}
				l12a2();
				INDX_BYT |= 0x80;	/* ,R+  */
			}
			return (l1326());
		}
		else
		{
			return l1584();
		}
	}

	/* Was: return 0, I suspect this was causing clr ,-s and others to be incorrect - BGP */
	return 1;
}

l12a2()
{
	if (HadBracket)
	{
		ilAdrMod();
	}
	else
	{
		return 1;
	}
}

ilAdrMod()
{
	e_report("illegal addressing mode");
}

not_pcr()			/* process 0,[reg] */
{
	char            var = 0xff;

#ifndef COCO
	register char  *nptr;

#endif

	switch (_toupper(*SrcChar))
	{
	case 'X':
		var = '\0';
		break;
	case 'Y':
		var = '\x20';
		break;
	case 'U':
		var = '\x40';
		break;
	case 'S':
		var = '\x60';
		break;
	case 'W':		/* do most of processing here */
		Is_W = TRUE;
		/* ,-W  not allowed */
		if ((INDX_BYT == '\x82'))
		{
			return 0;	/* let fall through to error
					 * condition */
		}
		if (INDX_BYT == '\x83')
		{		/* if ,--W       */
			INDX_BYT = '\xef';
		}
		else
		{
			INDX_BYT = '\x8f';
			if (HadArrow)
			{
				HadArrow = -1;	/* Force extended addressing
						 * if spec'd. */
			}

			/*
			 * if ( SrcChar[1] == '+' ) { if ( SrcChar[2] == '+'
			 * ) { SrcChar += 2; INDX_BYT |= 0x40; } else {
			 * /* If ,W+, let caller fall return 0;
			 * /*    to error   } } else { if ( d0033 ||
			 * HadArrow<0 ) { INDX_BYT |= 0x01; NumBytes += 2;
			 * #ifdef COCO opr_ptr->shrtlng.f_int = d0033; #else
			 * nptr = &d0033; nptr += sizeof(d0033)-2;   /* int
			 * part opr_ptr[1] = *(nptr++); opr_ptr[2] = *nptr;
			 * #endif /* force extended addressing, no other
			 * offset modes allowed HadArrow = -1; } }
			 */
		}
		++SrcChar;

		/*
		 * if (HadBracket) { if ( *(SrcChar) != ']' ) { return(
		 * e_report( "bracket missing" ) ); } INDX_BYT ^= 0x1f;
		 * ++SrcChar; HadBracket = 0; }
		 */
		return 1;
	default:
		return 0;
	}
	/* Cleanup for an X-Y-U-S match */
	++SrcChar;
	INDX_BYT |= var;
	return 1;
}

/*
 * This was the way to get the original compilation for the above function
 * (to be used after the switch, but it would not work without the "default"
 * above.  For some extremely strange reason, the below method would always
 * return 1, even though the assembly code matched the original WORD FOR
 * WORD!!!  The above method produces the correct result.
 * 
 * if ( var != '\xff' ) { ++SrcChar; INDX_BYT |= var; return 1; } else { return
 * 0; }
 */

ilIdxReg()
{
	e_report("illegal index register");
}

l1326()
{
	MLCod.opcode |= 0x20;
	++NumBytes;
	++d0021;
	if (HadBracket)
	{
		if (!Is_W)
		{
			INDX_BYT |= 0x10;
		}
		else
		{
			INDX_BYT ^= 0x1f;
		}
		if (*SrcChar != ']')
		{
			e_report("bracket missing");
		}
		else
		{
			++SrcChar;
		}
	}

	switch (*SrcChar)
	{
	case ' ':
	case '\t':
	case '\0':
		l143e();
		l1389();
		return 1;
	default:
		ilAdrMod();
	}
}

l1389()				/* Good module: compiles correctly */
{
	int             offset;
	int             rtyp;

	if (Pass2 && OptBPtr > d0791)
	{
		if (S_Addrs == 0)
		{
			ilExtRef();	/* report "illegal external
					 * reference" */
			/* return; */
		}
		else
		{
			offset = *S_Addrs + NumBytes - ((d0063 & 8) ? 1 : 2);

			while (OptBPtr > d0791)
			{
				register struct symblstr *reg;

				reg = (--OptBPtr)->RAddr;
				rtyp = reg->smbltyp | d0063 | OptBPtr->ETyp;

				if ((reg->w1 & 0x42) == 0)
				{
					undeflbl = 1;
					if (reg->wrd == 0)
					{
						++ref_cnt;
					}
					RefCreat(reg, rtyp, offset);
				}
				else
				{
					if ((reg->smbltyp & 0x0f) == 7)
					{
						RefCreat(reg, rtyp, offset);
					}
					else
					{
						++locl_cnt;
						RefCreat(d007c, rtyp, offset);
					}
				}
			}
		}
	}
}

l143e()
{
	struct ref_ent *var1;

	if (Pass2 && (OptBPtr > d0791))
	{
		register struct ref_ent *reg;

		reg = d0791;
		while (reg < OptBPtr)
		{
			if (l149b(reg) == 0)
			{
				if (l1541(reg->RAddr))
				{
					var1 = reg;
					while (++var1 < OptBPtr)
					{
						(var1 - 1)->ETyp = var1->ETyp;
						(var1 - 1)->RAddr = var1->RAddr;
					}
					--OptBPtr;
				}
				else
				{
					++reg;
				}
			}
		}
	}
	return 0;
}

l149b(op)
	struct ref_ent *op;

{
	struct ref_ent *var1;
	struct symblstr *var2;
	int             var3;
	register struct ref_ent *regopt = op;

	if ((var2 = regopt->RAddr)->w1 & 2)
	{
		/* Next line may be a kludge, FIX LATER */
		var3 = var2->smbltyp | regopt->ETyp;
		var1 = regopt;
		while (++var1 < OptBPtr)
		{
			if ((var2 = var1->RAddr)->w1 & 2)
			{

				/*
				 * NOTE: the following cast for "var3" is a
				 * temporary kludge... it works, but see if
				 * can do better
				 */

				if (((var2->smbltyp | var1->ETyp) ^ var3) == NEGMASK)
				{
					while (regopt + 1 < var1)
					{
						regopt->ETyp = (regopt + 1)->ETyp;
						regopt->RAddr = (regopt + 1)->RAddr;
						++regopt;
					}
					while (++var1 < OptBPtr)
					{
						regopt->ETyp = var1->ETyp;
						regopt->RAddr = var1->RAddr;
						++regopt;
					}
					OptBPtr = regopt;
					return 1;
				}
			}
		}
	}
	return 0;
}

l1541(parm)
	struct symblstr *parm;

{
	register struct symblstr *reg = parm;

	if (((reg->smbltyp & 0x0f) == 4) && (reg->w1 & 2) && (d0063 & 0xb0) == 0xa0)
	{
		return 1;
	}
	return 0;
}

l156b()				/* THIS FUNCTION IS OK */
{
	++SrcChar;
	if (!not_pcr())
	{
		pc_rel();
	}
	else
	{
		l1584();
	}
}

l1584()
{
	if ((HadArrow < 0) || ((HadArrow == 0) && ((d0033 > 127) || (d0033 < -128))))
	{
#ifndef COCO
		register char  *nptr;

		/*
		 * nptr = &d0033;i nptr += sizeof(d0033)-2;
		 *//* int part */

		/*
		 * opr_ptr[1] = *(nptr++); opr_ptr[2] = *nptr;
		 */
		storInt(&(opr_ptr[1]), d0033);
#else
		opr_ptr->shrtlng.f_int = d0033;
#endif
		NumBytes += 2;
		if (!Is_W)
		{
			INDX_BYT |= 0x89;	/* mmnn,R     */
		}
		else
		{
			INDX_BYT |= 0x20;
		}
		return (l1326());
	}
	if (!HadArrow && (d0033 == 0))
	{
		if (!Is_W)
		{
			INDX_BYT |= 0x84;	/* set  0,R  to  ,R   */
		}
		else
		{
			INDX_BYT &= 0x9f;
		}
		return (l1326());
	}
	else
	{
		if ((HadArrow > 0) || HadBracket || (d0033 > 0x0f) || (d0033 < -(0x10)))
		{
			opr_ptr[1] = d0033;
			++NumBytes;
			d0063 |= 8;
			INDX_BYT |= 0x88;
			return (l1326());
		}
		else
		{
			INDX_BYT |= (d0033 & 0x1f);
			return (l1326());
		}
	}
}

pc_rel()			/* originally  l1602() */
{
	if ((_toupper(*SrcChar) != 'P') || (_toupper(SrcChar[1]) != 'C'))
	{
		return (ilIdxReg());	/* No match, quit */
	}

	SrcChar += 2;		/* bump past "PC" string */

	if (_toupper(*SrcChar) == 'R')
	{			/* if "pcr" instead of "pc" */
		++SrcChar;
	}
	++NumBytes;
	d0063 |= 0x80;
	d0033 -= (CodeSize + NumBytes + 1);
	if (HadArrow > 0)
	{
		opr_ptr[1] = d0033;
		d0063 |= 8;
		INDX_BYT |= 0x8c;
	}
	else
	{
#ifndef COCO
		register char  *nptr;

#endif
		++NumBytes;
#ifndef COCO
		--d0033;
		nptr = &d0033;
		/* nptr += sizeof(d0033)-2; *//* int part */

		/*
		 * opr_ptr[1] = *(nptr++); opr_ptr[2] = *nptr;
		 */
		storInt(&opr_ptr[1], d0033);
#else
		opr_ptr->shrtlng.f_int = (--d0033);
#endif
		INDX_BYT |= 0x8d;
	}

	l1326();
}

/*
 * New addition: storInt() - saves an integer value in LITTLE-ENDIAN format
 */

void
                storInt(char *dst, int vlu)
{
	*dst = (vlu >> 8) & 0xff;
	dst[1] = vlu & 0xff;
}
