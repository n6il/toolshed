/* rma.c first section of "RMA" assembler program */

#define MAIN

#include "rma.h"
#include <errno.h>

/*
 * extern FILE *f_opn(); char *itoa();
 * 
 * extern direct char *Bsr_Ptr, *Cod_End; extern CODFORM S_Lbr, S_Nam, S_Bsr;
 * extern JMPTBL Drectives[];
 */

/*
 * extern char *O_Nam, d07c9[], *SrcNam[]; extern char **LastSrc;
 */

#ifdef __STDC__
static          AsmblLin(void);

#endif

#ifdef __STDC__

int
                main(int argc, char *argv[])
#else

main(argc, argv)
	int             argc;
	char           *argv[];

#endif
{
	char          **rd_fil;

	/* Start off by inserting current directory as a place to look for includes. */
	incdirs[0] = ".";
	inccount++;

	GetOpts(argc, argv);
	if (ListFlg)
	{
		listON = ON;
	}
	strcpy(cmntlen, " %1.");/* Initialize cmntlen */
	d007e.smbltyp = 4;
	d0092.smbltyp = 3;
	d00a6.smbltyp = 1;
	d007e.w1 = d0092.w1 = d00a6.w1 = 2;
	now_mac = 0;

	if (OFlg)
	{
		OutFile = f_opn(O_Nam, "w");
	}
	rd_fil = SrcNam;

	while (rd_fil < LastSrc)
	{
		SrcFile = f_opn(*rd_fil, "r");
		d007e.wrd = d0092.wrd = d00a6.wrd = (struct ref_str *) 0;
		Pass2 = *d0591 = _ttl[0] = '\0';
		ReadFil();	/* PASS 1 */
		rewind(SrcFile);
		Pass2 = TRUE;

		ReadFil();	/* PASS 2 */
		l2cf1();
		wrt_refs();
		if (fclose(SrcFile) != 0)
		{
			e_report("file close error");
		}
		if (S_Flg > 0)
		{
			DoSymTbl();
		}
		++rd_fil;
	}
	closmac();
	if (had_err)
	{

		/*
		 * if an output file exists, and "keep r-file" has not been
		 * specified then delete it
		 */
		if (OutFile && (R_Keep < 1))
		{
			fclose(OutFile);
			unlink(O_Nam);
		}
		exit(1);
	}
	exit(0);
}

#ifdef __STDC__

int
                ReadFil(void)
#else

ReadFil()
#endif
{
	char            var1;

	setsect();
	had_err = d0025 = FALSE;
	d0078 = d068f;
	_If_Dpth = _IfIsTru = d0027 = ListLin = ListPag = (unsigned) 0;
	list_lin = d00da = 0;
	if (Pass2 && listON && (ListFlg > 0))
	{
		doList = ON;
	}
	else
	{
		doList = OFF;
	}
	prnthdr();
	while (getUseLn())
	{
		if (AsmblLin())
		{
			coment = SrcChar;
		}
		ListLine();
	}
	if (Pass2 && listON)
	{
		if (ff_flg)
		{
			putc('\f', PrtPth);
		}
		else
		{
			while (list_lin < PgDepth)
			{
				putc('\n', PrtPth);
				++list_lin;
			}
		}
	}
}

#define TRM_0064 if ( oprand == SrcChar ) { oprand=0; } else { if ( oprand != 0 ) { if (*SrcChar) { *SrcChar++=0; SkipSpac();}}} return 1;

#ifdef __STDC__

static
                AsmblLin(void)
#else

static          AsmblLin()
#endif
{
	char            var2;
	MACDSCR        *var1;

	NumBytes = d0033 = d001d = d0021 = HadArrow = undeflbl = Is_W = AIMFlg =
		d0039[0] = 0;
	/* opr_ptr = &C_Oprnd; */
	opr_ptr = MLCod.opers;

	if ((Adrs_Ptr = S_Addrs))
	{

		/*
		 * Note: see if the two lines can be integrated such that:
		 * leax >d0051  \  stx Adrs_Ptr  \ ldd [S_Addrs,y] \ std ,x
		 */
		/* (Adrs_Ptr=&d0051)->(?) = *S_Addrs ??? */
		Adrs_Ptr = &d0051;
		d0051 = *S_Addrs;	/* S_Addrs is char * ?? */
	}
	Label = operatr = oprand = coment = CmdCod = 0;
	d005f = d005e;
	LblTyp = d0060;
	d0063 = d0062;
	OptBPtr = &d0791[0];
	if (Pass2 && listON && (ListFlg > 0) && (_IfIsTru == 0 || C_Flg > 0))
	{
		doList = ON;
	}
	else
	{
		doList = OFF;
	}
	d00d8 = 0;
	if (MacFlag && (x_flg <= OFF))
	{
		doList = OFF;
	}
	if (((var2 = *SrcChar) == '\0') || (var2 == '*'))
	{
		return 1;
	}
	if (var2 != ' ' && var2 != '\t')
	{
		Label = SrcChar;
		if ((!InMacro) && (!_IfIsTru) && (!d00d4))
		{
			if (l1e4d() == 0)
			{
				return (e_report("bad label"));
			}
		}
		else
		{
			while ((var2 = *SrcChar) != '\0')
			{
				if (var2 == ' ' || var2 == '\t')
				{
					break;
				}
				++SrcChar;
			}
		}
		if (*SrcChar == ' ' || *SrcChar == '\t')
		{
			*(SrcChar++) = '\0';
		}
	}
	if ((var2 = SkipSpac()) == 0)
	{
		return (1);
	}
	if (MoveLabel(LabelName) != 0)
	{
		operatr = LabelName;
		SkipSpac();
		if ((d00d4 == 0) && ((var1 = McNamCmp(LabelName)) != 0))
		{
			if (InMacro != 0)
			{
				mcWrtLin();
				return 1;
			}
			else
			{
				addMac(var1);
				return 1;
			}
		}
		else
		{
			if ((Cod_Ent = FindCmd(LabelName, S_Nam, SNamEnd)))
			{
				jsrOfst = Cod_Ent->tofst & '\x0f';
				if (d00d4 != 0)
				{
					if ((jsrOfst == 0) && (Cod_Ent->opcode == '\x0b'))
					{
						/* line was "endr" */
						return (end_rept());
					}
					else
					{
						doList = OFF;
						return 1;
					}
				}
				else
				{	/* in case of macro stuff */
					if (InMacro != FALSE)
					{
						if (jsrOfst == 4)
						{
							return (nstmacdf());	/* can't nest macro defs */
						}
						if (jsrOfst == 5)
						{	/* if "endm" */
							add_nul();
							InMacro = 0;
							return 1;
						}
						else
						{
							mcWrtLin();
							return (1);
						}
					}
					if (_IfIsTru != 0)
					{
						if (jsrOfst != 1)
						{	/* if not "if" statememt */
							if ((jsrOfst != 2) || (--_IfIsTru == 0) ||
							    (Cod_Ent->opcode == 0))
							{
								return 1;
							}
						}
						++_IfIsTru;
						return 1;
					}
					MLCod.opcode = Cod_Ent->opcode;
					oprand = SrcChar;

					if ((*Drectives[jsrOfst]) () == 0)
					{
						return 0;
					}
					else
					{
						/* TRM_0064; */
						if (oprand == SrcChar)
						{
							oprand = 0;
						}
						else
						{
							if (oprand != 0)
							{
								if (*SrcChar)
								{
									*(SrcChar++) = '\0';
									SkipSpac();
								}
							}
						}
						return 1;
					}
				}
			}
			else
			{
				if (d00d4 != 0)
				{
					doList = OFF;
					return 1;
				}
				else
				{
					if (InMacro != 0)
					{
						mcWrtLin();
						return 1;
					}
					else
					{
						if (_IfIsTru != 0)
						{
							if ((doList == OFF) || (*SrcChar == '\0'))
							{
								return 1;
							}
							else
							{
								oprand = SrcChar;
								while (*(++SrcChar) != '\0')
								{
									if (*SrcChar == ' ' || *SrcChar == '\t')
									{
										break;
									}
								}
								if (*SrcChar != 0)
								{
									*SrcChar++ = '\0';
								}
								return 1;
							}
						}
						if (CodTbBgn == S_Lbr)
						{
							if ((LabelName[0] == 'b') ||
							    (LabelName[0] == 'B'))
							{
								Cod_Ent = FindCmd(LabelName, S_Bsr, SBsr_End);
							}
						}
						if (Cod_Ent == 0)
						{
							Cod_Ent = FindCmd(LabelName, CodTbBgn, CodTbEnd);
						}
						if (Cod_Ent != 0)
						{
#ifndef COCO
							register int    count;

#endif
							jsrOfst = Cod_Ent->tofst & 0x1f;
							NumBytes = 1;
							CmdCod = &MLCod.opcode;

							/*
							 * Set up Prebyte if
							 * applicable
							 */

							if ((MLCod.prebyte = Cod_Ent->tofst & (PRE_10 | PRE_11)) != 0)
							{
								if ((MLCod.prebyte & PRE_10))
								{
									MLCod.prebyte = 0x10;
								}
								else
								{
									MLCod.prebyte = 0x11;
								}
								NumBytes = 2;
								CmdCod = &MLCod.prebyte;
							}
							MLCod.opcode = Cod_Ent->opcode;
#ifndef COCO
							for (count = 0; count < 4; count++)
								MLCod.opers[count] = '\0';
#else
							C_Oprnd.q_long = 0;
#endif
							oprand = SrcChar;
							if (!(*jmp_ptr[jsrOfst]) ())
							{
								return 0;
							}
							else
							{
								w_ocod(CmdCod, NumBytes);
								/* TRM_0064; */
								if (oprand == SrcChar)
								{
									oprand = 0;
								}
								else
								{
									if (oprand != 0)
									{
										if (*SrcChar)
										{
											*(SrcChar++) = '\0';
											SkipSpac();
										}
									}
								}
								return 1;
							}
						}
					}
				}
			}
		}
	}
	coment = SrcChar;
	e_report("bad mnemonic");
}

#ifdef __STDC__

int
                ListLine(void)
#else

ListLine()
#endif
{
	register char  *regvar;

	if (doList == OFF)
		return 0;
	if ((PgDepth - 3) < list_lin)
	{
		prnthdr();
	}
	doList = OFF;
	if (d00c7 == 0)
	{
		printf("%05d", ListLin);	/* Line number */
	}
	if (Label || operatr || (NumBytes > 0))
	{			/* l05e6 */
		int             var1;

		regvar = CmdCod;

		/* Code offset */

		if (Adrs_Ptr != 0)
		{
			printf(" %04x", *Adrs_Ptr);
		}
		else
		{
			fputs("     ", stdout);
		}

		putc(undeflbl ? '=' : ' ', stdout);

		/* Code data (if any) */

		if (regvar != 0)
		{
			for (var1 = 0; var1 < 5; var1++)
			{
				if (var1 < NumBytes)
				{
					printf("%02x", (*(regvar++)) & 0xff);
				}
				else
				{
					fputs("  ", stdout);
				}
			}
			/* attempt to get byte printout in linux and osk */

			/*
			 * var1=0; if(MLCod.prebyte) {
			 * printf("%02x",MLCod.prebyte&0xff); ++var1; }
			 * printf("%02x",MLCod.opcode&0xff);++var1; regvar =
			 * MLCod.opers; for(; var1 < 5; var1++) {
			 * if(var1<NumBytes) printf( "%02x",
			 * (*(regvar++))&0xff); else fputs( "  ", stdout); }
			 */
		}
		else
		{
			fputs("          ", stdout);
		}
	}
	if (Label || operatr)
	{			/* l0689 */
		if (Label == 0)
		{
			Label = "\0";
		}
		if (operatr == 0)
		{
			operatr = "\0";
		}
		if (oprand == 0)
		{
			oprand = "\0";
		}

		/* Label-Operator-Operand */

		printf("%c%-8s %-5s ", (MacFlag && (d00d8 == 0)) ?
		       '+' : ' ', Label, operatr);
		printf(d00d8 ? "%s" : "%-10s", oprand);
	}

	/* Comment output */
	if (coment)
	{
		int             llen;

		if (Label || operatr || (NumBytes > 0))
		{
			llen = PgWidth - 39 - (strlen(oprand) <= 10 ? 10 : strlen(oprand));
		}
		else
		{
			llen = PgWidth - 7;
		}
		if (llen > 3)
		{
			itoa(llen, &cmntlen[4]);
			/* printf( " %s", coment); */
			printf(strcat(cmntlen, "s"), coment);
		}
	}
	putc('\n', stdout);
	++list_lin;
}

#ifdef __STDC__

int
                bdrglist(void)
#else

bdrglist()
#endif
{
	e_report("bad register list");
}

#ifdef __STDC__

int
                bd_rgnam(void)
#else

bd_rgnam()
#endif
{
	e_report("bad register name");
}

#ifdef __STDC__

int
                nstmacdf(void)
#else

nstmacdf()
#endif
{
	e_report("nested MACRO definitions");
}

#ifdef __STDC__

int
                e_report(char *report)
#else

e_report(report)
	char           *report;

#endif
{
	if (Pass2)
	{
		doList = 1;
		if ((list_lin == 0) || (list_lin > PgDepth - 4))
		{
			prnthdr();
		}
		printf("*** error - %s ***\n", report);
		++list_lin;
	}
	++had_err;
	return 0;
}

#ifdef __STDC__

int
                errexit(char *report)
#else

errexit(report)
	char           *report;

#endif
{
	fprintf(stderr, "asm: %s\n", report);
	exit(errno);
}

#ifdef _OSK
/* Add coco functions not included with OSK */

char           *strchr(st, c)
	char           *st,
	                c;

{
	char            c1;

	while (c1 = *(st++))
	{
		if (c1 == c)
			return --st;
	}
	return 0;
}

#endif


void            reverse();

#ifdef __STDC__

char           *
                itoa(int n, char *s)
#else

char           *itoa(n, s)
	int             n;
	char           *s;

#endif
{
	register int    i,
	                sign;

	if ((sign = n) < 0)
		n = -n;		/* abs(n) */
	i = 0;
	do
	{			/* generate digits in reverse order */
		s[i++] = n % 10 + '0';	/* get next digit */
	} while ((n /= 10) > 0);/* delete it */
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';		/* null terminator for string */
	reverse(s);
	return s;
}

#ifdef __STDC__

void
                reverse(char *s)
#else

void            reverse(s)
	char           *s;

#endif
{
	register int    c,
	                i,
	                j;

	for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
