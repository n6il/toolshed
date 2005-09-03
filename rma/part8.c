/* part 8 - for RMA */

#include <time.h>
#include "rma.h"

#define BEGIN 0			/* fseek "place" */
#define FormFeed putc( 0x0c, PrtPth );
#define CarrgRet putc ( '\n', PrtPth );

#ifdef COCO
#define PutInt(i,c) fwrite(&i,2,1,c)
#else
#define PutInt(i,c) putc( i>>8&0xff,c);putc(i&0xff,c);
#endif

long            d03dc = 0;

struct binhead
{
#ifdef COCO
	long            h_sync;	/* d03e0 */
	_ushort         h_tylan;/* d03e4 */
#else
	char            h_sync[4];
	char            h_tylan[2];
#endif
	char            h_valid,/* d03e6 */
	                h_date[5],	/* d03e7 */
	                h_edit,	/* d03ec */
	                h_spare;/* d03ed */
#ifdef COCO
	_ushort         h_glbl,	/* d03ee */
	                h_dglbl,/* d03f0 */
	                h_data,	/* d03f2 */
	                h_ddata,/* d03f4 */
	                h_ocode,/* d03f6 */
	                h_stack,/* d03f8 */
	                h_entry;/* d03fa */
}               RofHdr =

{
	0x62cd2387, 0, 0,
	{
		0, 0, 0, 0, 0
	}, 0, 1, 0, 0, 0, 0, 0, 0, 0
};

#else
	char            h_glbl[2],
	                h_dglbl[2],
	                h_data[2],
	                h_ddata[2],
	                h_ocode[2],
	                h_stack[2],
	                h_entry[2];
}               RofHdr =

{
	{
		'\x62', '\xcd', '\x23', '\x87'
	},
	{
		0, 0
	}, 0,
	{
		0, 0, 0, 0, 0
	}, 0, 1,
	{
		0, 0
	},
	{
		0, 0
	}
	,
	{
		0, 0
	},
	{
		0, 0
	},
	{
		0, 0
	},
	{
		0, 0
	},
	{
		0, 0
	}
};

/* static struct sgtbuf D_Time; */
#endif

/*
 * extern direct int PgDepth, PgWidth, rversion; extern direct FILE *SrcFile,
 * *OutFile, *PrtPth; extern direct long nmbr_int; extern direct unsigned
 * ListPag, ListLin; extern direct int doList; extern direct unsigned Glbls,
 * dGlbls, edatas, ddatas; extern direct char **S_Addrs; extern direct
 * unsigned CodeSize; extern direct int had_err, list_lin; extern direct
 * short GlobCnt, ref_cnt, comn_cnt, locl_cnt; extern direct char OFlg,
 * ff_flg, d0060, d00c4, d00c7, Pass2, d005f; extern direct struct symblstr
 * d007e, d0092, d00a6; extern direct char *SrcChar; extern direct int
 * undeflbl;
 * 
 * extern struct symblstr d0587; extern long RofHdLoc, d07b7, d07bb, d07bf;
 * extern char _ttl[]; extern struct symblstr *d068f[];
 */
int             PrtSmbl(), wrtGlbls(), wrtxtrns(), wrtCmmns();

#ifdef __STDC__

int
                GetOpts(	/* parse parameter line */
			                unsigned _argc,
			                char **_argv
)
#else

GetOpts(_argc, _argv)		/* parse parameter line */
	unsigned        _argc;
	char          **_argv;

#endif
{
	char            var;

	SrcFile = OutFile = 0;
	PrtPth = stdout;
	/* OutFile += 2; */
	++_argv;
	while (--_argc)
	{
		if (**_argv == '-')
		{
			SrcChar = *_argv + 1;
			set_opts();	/* Check to see if SrcChar must be
					 * passed */
			if ((OFlg) && ((var = _toupper(SrcChar[-1])) != '\0') &&
			    (var == 'O') && (*SrcChar == '='))
			{
				if (O_Nam)
				{
					errexit("too many object files");
				}
				else
				{
					O_Nam = ++SrcChar;
				}
			}

			if (_toupper(*SrcChar) == 'I')
			{
				char *p = ++SrcChar;

				if (*p == '=')
				{
					p++;
				}

				incdirs[inccount++] = p;
			}
		}
		else
		{
			if (LastSrc > &SrcNam[31])
			{
				errexit("too many input files");
			}
			*(LastSrc++) = *_argv;
		}
		++_argv;
	}
	if (LastSrc == SrcNam)
	{
		errexit("no input file");
	}
	if (O_Nam == 0)
	{
		O_Nam = "output.r";
	}
}

#ifdef __STDC__

FILE
* f_opn(char *fna, char *fmo)
#else

FILE           *f_opn(fna, fmo)
	char           *fna,
	               *fmo;

#endif
{
	FILE           *fptr;

	if ((fptr = fopen(fna, fmo)) == 0)
	{
		fprintf(stderr, "\"%s\" - ", fna);
		errexit("can't open file");
	}
	return fptr;
}

#ifdef __STDC__

FILE
* u_opn(char *fna, char *fmo)
#else

FILE           *u_opn(fna, fmo)
	char           *fna,
	               *fmo;

#endif
{
	FILE           *fptr;
	char           f[128];
	int            i;

	for (i = 0; i < inccount; i++)
	{	
		/* try prepending include dirs in front */

		sprintf(f, "%s/%s", incdirs[i], fna);

		if ((fptr = fopen(f, fmo)) != 0)
		{
			return fptr;
		}
	}

	fprintf(stderr, "\"%s\" - ", fna);
	errexit("can't open file");
}

#ifdef __STDC__

int
                prnthdr(void)
#else

prnthdr()
#endif
/* Prints header for listing */
{
	time_t          now;
	struct tm      *tm;

	if (Pass2 && doList)
	{
		if (list_lin)
		{
			if (ff_flg)
			{
				FormFeed;
			}
			else
			{
				while (list_lin < PgDepth)
				{
					CarrgRet;
					++list_lin;
				}
			}
		}
		now = time(0);
		tm = localtime(&now);
		/* getime( &D_Time ); */
		printf("Microware OS-9 %s  %02d/%02d/%02d  %02d:%02d", "RMA - V2.0",
		       tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900,
		       tm->tm_hour, tm->tm_min, tm->tm_sec);
		printf("   %-22s Page %03d\n", SrcNam[0], ++ListPag);
		printf("%s - %s\n\n", _nam, _ttl);
		list_lin = 3;
	}
}

#ifdef __STDC__

int
                new_pag(void)
#else

new_pag()
#endif
{
	if (Pass2 && doList)
	{
		if ((PgDepth - 3) < list_lin)
		{
			prnthdr();
		}
		CarrgRet;
		++list_lin;
	}
}

#ifdef __STDC__

int
                getUseLn(void)
#else

getUseLn()
#endif
{
	char            s_chr;
	register char  *dest;

	if (readmacs() == 0)
	{
		do
		{
			if (d00c4 && (ThisFile == FrstFil))
			{
				printf("Asm:");
			}
			dest = SrcChar = d0821;
			while ((s_chr = getc(SrcFile)) != -1)
			{
				if (s_chr == '\n')
				{	/* Carriage Return */
					*dest = '\0';
					++ListLin;
					return 1;
				}
				*(dest++) = s_chr;
			}
		} while (DecSFil());
		return 0;
	}
	else
	{
		return 1;
	}
}

#ifdef __STDC__

static
                DecSFil(void)
#else

static          DecSFil()
#endif
{
	if (ThisFile == FrstFil)
	{
		return 0;
	}
	if (fclose(SrcFile))
	{
		e_report("file close error");
	}
	SrcFile = *(--ThisFile);
	return 1;
}

#ifdef __STDC__

int
                DoSymTbl(void)
#else

DoSymTbl()
#endif
{
	printf("\nSymbol Table\n");
	PgWidth -= 24;
	PrnTree(PrtSmbl);
	if (SColumn > 0)
	{
		printf("\n");
	}
	printf("\n");
}

#ifdef __STDC__

static
                PrtSmbl(struct symblstr * smbl)
#else

static          PrtSmbl(smbl)
	struct symblstr *smbl;

#endif
{
	register struct symblstr *sblptr = smbl;

	if (SColumn > 0)
	{
		printf("  |");
	}
	printf("  %-9s %02x %02x %04x",
	       sblptr->symblnam, sblptr->smbltyp, (sblptr->w1) & 0xff, sblptr->s_ofst);
	if (((SColumn = SColumn + 25) > PgWidth) || d00c7)
	{
		printf("\n");
		SColumn = 0;
	}
}

#ifdef __STDC__

int
                do_psect(void)
#else

do_psect()
#endif
{
	char            rof_nam[16];
	register char  *reg;
	time_t          now;
	struct tm      *tm;


	if (*SrcChar != '\0')
	{
		int             tvar;

		reg = rof_nam;

		tvar = 16;
		while ((tvar--) && (*SrcChar != '\0') && (*SrcChar != ','))
		{
			*(reg++) = *(SrcChar++);
		}
		*reg = '\0';
		if (iscomma())
		{		/* get TyLan */
			getNmbr();
			e_xtrnl();
#ifdef COCO
			RofHdr.h_tylan = nmbr_int << 8;
#else
			RofHdr.h_tylan[0] = nmbr_int & 0xff;
#endif
			if (iscomma())
			{
				getNmbr();
				e_xtrnl();
#ifdef COCO
				RofHdr.h_tylan |= nmbr_int;
#else
				RofHdr.h_tylan[1] = nmbr_int & 0xff;
#endif
				if (iscomma())
				{
					getNmbr();
					e_xtrnl();
					RofHdr.h_edit = nmbr_int;
					if (iscomma())
					{
						getNmbr();
						e_xtrnl();
#ifdef COCO
						RofHdr.h_stack = nmbr_int;
#else
						RofHdr.h_stack[0] = (nmbr_int >> 8) & 0xff;
						RofHdr.h_stack[1] = nmbr_int & 0xff;
#endif
						if (iscomma())
						{
							getNmbr();
							e_xtrnl();
#ifdef COCO
							RofHdr.h_entry = nmbr_int;
#else
							RofHdr.h_entry[0] = (nmbr_int >> 8) & 0xff;
							RofHdr.h_entry[1] = nmbr_int & 0xff;
#endif
						}
					}
				}
			}
		}
	}
	else
	{
		strcpy(rof_nam, "program");
	}
	now = time(0);
	tm = localtime(&now);
	/* getime( &D_Time ); */
	RofHdr.h_date[0] = (tm->tm_year) & 0xff;
	RofHdr.h_date[1] = (tm->tm_mon) & 0xff;
	RofHdr.h_date[2] = (tm->tm_mday) & 0xff;
	RofHdr.h_date[3] = (tm->tm_hour) & 0xff;
	RofHdr.h_date[4] = (tm->tm_min) & 0xff;
	/* _strass( RofHdr.h_date, &D_Time, 5 ); */
	RofHdr.h_valid = had_err;
	RofHdr.h_spare = rversion;
#ifdef COCO
	RofHdr.h_glbl = Glbls;
	RofHdr.h_dglbl = dGlbls;
	RofHdr.h_data = edatas;
	RofHdr.h_ddata = ddatas;
	RofHdr.h_ocode = CodeSize;
#else
	RofHdr.h_glbl[0] = (Glbls >> 8) & 0xff;
	RofHdr.h_glbl[1] = Glbls & 0xff;
	RofHdr.h_dglbl[0] = (dGlbls >> 8) & 0xff;
	RofHdr.h_dglbl[1] = dGlbls & 0xff;
	RofHdr.h_data[0] = (edatas >> 8) & 0xff;
	RofHdr.h_data[1] = edatas & 0xff;
	RofHdr.h_ddata[0] = (ddatas >> 8) & 0xff;
	RofHdr.h_ddata[1] = ddatas & 0xff;
	RofHdr.h_ocode[0] = (CodeSize >> 8) & 0xff;
	RofHdr.h_ocode[1] = CodeSize & 0xff;
#endif

	if (Pass2 && OutFile)
	{
		RofHdLoc = ftell(OutFile);	/* ?? OutFile assumed */
		fwrite(&RofHdr, sizeof(RofHdr), 1, OutFile);
		fputs(rof_nam, OutFile);
		putc('\0', OutFile);
		/* fwrite( &GlobCnt, 2, 1, OutFile ); */
		PutInt(GlobCnt, OutFile);
		if (GlobCnt)
		{
			PrnTree(wrtGlbls);
		}
		d07bf = (d07bb = (d07b7 = d03dc = ftell(OutFile)) + (long) CodeSize) +
			(long) ddatas;
	}
}

#ifdef __STDC__

int
                e_xtrnl(void)
#else

e_xtrnl()
#endif
{
	if (Pass2 && undeflbl)
	{
		e_report("no external allowed");
	}
}

#ifdef __STDC__

void
                l2cf1(void)
#else

void
                l2cf1()
#endif

{
	unsigned        hd_sz = (unsigned) (&(RofHdr.h_valid)) - (unsigned) (&RofHdr);

	if ((OutFile == 0) || (OFlg <= 0))
	{
		return;
	}
	fseek(OutFile, (RofHdLoc + (long) hd_sz), BEGIN);
	putc(had_err, OutFile);
	d03dc = RofHdLoc + (long) hd_sz + 1;
}

#ifdef __STDC__

int
                w_ocod(unsigned char *_codaddr, int _codlen)
#else

w_ocod(_codaddr, _codlen)
	char           *_codaddr;
	int             _codlen;

#endif
{
	register long  *_codloc;

	if (S_Addrs && _codlen)
	{
		*S_Addrs += _codlen;
		if (Pass2 && (d0060 & 2) && OutFile && (OFlg > 0))
		{
			if (d005f & 4)
			{
				_codloc = &d07b7;
			}
			else
			{
				if (d005f & 2)
				{
					_codloc = &d07bb;
				}
				else
				{
					_codloc = &d07bf;
				}
			}
			if (d03dc != *_codloc)
			{
				fseek(OutFile, *_codloc, BEGIN);
			}
			fwrite(_codaddr, _codlen, 1, OutFile);
			d03dc = *_codloc = *_codloc + (long) _codlen;
		}
	}
}

#ifdef __STDC__

static void
                PrnTree(int (*function) (struct symblstr * xx))
#else

static          PrnTree(function)
	int             (*function) ();

#endif
{
	register struct symblstr *refrnc;
	int             count;

	count = 0;
	while (count < 64)
	{
		if (refrnc = d068f[count])
		{
			while (refrnc->left)
			{
				refrnc = refrnc->left;
			}
			do
			{
				(*function) (refrnc);
			} while (refrnc = WlkTreLft(refrnc));
		}
		++count;
	}
}

#ifdef __STDC__

int
                wrt_refs(void)
#else

wrt_refs()
#endif
{
	if (OutFile)
	{
		fseek(OutFile, d07bf, BEGIN);
		/* fwrite( &ref_cnt, 2, 1, OutFile ); */
		PutInt(ref_cnt, OutFile);
		if (ref_cnt)
		{
			PrnTree(wrtxtrns);
		}
		/* fwrite( &locl_cnt, 2, 1, OutFile ); */
		PutInt(locl_cnt, OutFile);
		if (locl_cnt)
		{
			if (d007e.wrd)
			{
				DoRefLin(d007e.wrd);
			}
			if (d0092.wrd)
			{
				DoRefLin(d0092.wrd);
			}
			if (d00a6.wrd)
			{
				DoRefLin(d00a6.wrd);
			}
		}
		/* Write commons if rof version > 0 */
		if (rversion)
		{
			/* fwrite( &comn_cnt, 2, 1, OutFile ); */
			PutInt(comn_cnt, OutFile);
			if (comn_cnt)
			{
				PrnTree(wrtCmmns);
			}
		}
	}
}

#ifdef __STDC__

int
                wrtGlbls(struct symblstr * parm)
#else

wrtGlbls(parm)
	struct symblstr *parm;

#endif
{
	register struct symblstr *reg = parm;
	short           sbloffst;
	char            styp;

	if (((reg->w1 & 0xc6) == 0x86) && (reg->smbltyp != 7))
	{
		fputs(reg->symblnam, OutFile);
		putc('\0', OutFile);
		styp = reg->smbltyp;
		sbloffst = reg->s_ofst;
		/* fwrite( &styp, 3, 1, OutFile ); */
		putc(styp, OutFile);
		PutInt(sbloffst, OutFile);
	}
}

#ifdef __STDC__

int
                wrtxtrns(struct symblstr * parm1)
#else

wrtxtrns(parm1)
	struct symblstr *parm1;

#endif
{
	register struct symblstr *reg = parm1;
	short           numrefs;

	if (((reg->w1 & 0x42) == 0) && (reg->wrd) &&
	    ((numrefs = reg->smbltyp & 0x0f) != 6) && (numrefs != 5))
	{
		fputs(reg->symblnam, OutFile);
		putc('\0', OutFile);
		numrefs = CountRfs(reg);
		/* fwrite( &numrefs, 2, 1, OutFile ); */
		PutInt(numrefs, OutFile);
		DoRefLin(reg->wrd);
	}
}

/*
 * CountRfs() : counts references and apparently reverses the direction
 * ->NxtRef points in the line
 */

#ifdef __STDC__

int
                CountRfs(struct symblstr * smbls)
#else

CountRfs(smbls)
	struct symblstr *smbls;

#endif
{
	register struct ref_str *reg;
	struct ref_str *next;
	int             rf_cnt;

	rf_cnt = 0;
	next = smbls->wrd;	/* init next */
	smbls->wrd = 0;
	while (reg = next)
	{
		next = reg->NxtRef;
		reg->NxtRef = smbls->wrd;
		smbls->wrd = reg;
		++(rf_cnt);
	}
	return rf_cnt;
}

#ifdef __STDC__

static
                DoRefLin(struct ref_str * parm)
#else

static          DoRefLin(parm)
	struct ref_str *parm;

#endif
{
	register struct ref_str *sref = parm;

#ifndef COCO
	short           ofst;
	char            flg;

#endif

	while (sref)
	{
#ifdef COCO
		fwrite(sref, 3, 1, OutFile);
#else

		/*
		 * flg = sref->RfTyp; ofst = sref->r_offset; fwrite( &flg, 3,
		 * 1, OutFile );
		 */
		putc(sref->RfTyp, OutFile);
		PutInt((int) sref->r_offset, OutFile);
#endif
		sref = sref->NxtRef;
	}
}

#ifdef __STDC__

int
                wrtCmmns(struct symblstr * parm)
#else

wrtCmmns(parm)
	struct symblstr *parm;

#endif
{
	register struct symblstr *reg = parm;
	int             var;

	if (((reg->w1 & 0xc6) == 0x86) && ((reg->smbltyp & 0x0f) == 7))
	{
		fputs(reg->symblnam, OutFile);
		putc('\0', OutFile);
		/* fwrite( &reg->s_ofst, 2, 1, OutFile ); */
		PutInt(reg->s_ofst, OutFile);
		var = CountRfs(reg);
		/* fwrite( &var, 2, 1, OutFile ); */
		PutInt(var, OutFile);
		DoRefLin(reg->wrd);
	}
}
