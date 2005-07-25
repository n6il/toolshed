/*
 * rdump:  prints formatted dump of .r and .l files rdump [opts] <file>[
 * <file>] [opts] options: -g - add global definition info -r - add reference
 * info -o - add reference and local offset info -a - all of the above
 */


#include <stdio.h>
#include <rof.h>

typedef unsigned short u16;	/* Little-endian coco int */

#define MAXSOURCE 20
#define puts(s) fputs(s,stdout)
#define mc(c) ((c)&0xff)
#define DEF 1
#define REF 2

char           *snames[MAXSOURCE],
               *fname;
u16             scount;
u16             gflag,
                rflag,
                oflag;
binhead         hd;
FILE           *in;

unsigned int    o9_int();
unsigned int    getwrd();


main(argc, argv)
	int             argc;
	char          **argv;
{
	register char  *p;

	while (--argc > 0)
	{
		if (*(p = *++argv) == '-')
		{
			while (*++p)
				switch (*p)
				{
				case 'g':
					gflag = 1;
					break;
				case 'r':
					rflag = 1;
					break;
				case 'o':
					oflag = 1;
					break;
				case 'a':
					gflag = rflag = oflag = 1;
					break;
				default:
					error("unknown option -%c", *p);
				}
	done:		;
		}
		else
		{
			if (scount == MAXSOURCE)
				error(0, "too many source files");
			snames[scount++] = *argv;
		}
	}
	pass1();
}

pass1()
{
	int             count;

	if (scount == 0)
		return;
	for (count = 0; count < scount; ++count)
	{
		fname = snames[count];
		if ((in = fopen(fname, "r")) == NULL)
		{
			printf("can't open '%s'\n", fname);
			continue;
		}
		for (;;)
		{
			char c1;

			if (fread(&hd, sizeof(hd), 1, in) == 0)
				break;
			if (hd.h_sync != ROFSYNC)
			{
				printf("'%s' is not a relocatable module\n", fname);
				break;
			}
			showhead();
			showglobs();
			/* skip code and initialized data */
			fseek(in, o9_int(hd.h_ocode +
					 o9_int(hd.h_ddata) +
					 o9_int(hd.h_data)), 1);
			showrefs();
			showlcls();

			/* Some ROFs have two extra zero bytes at the end.  We check if that is the case -- BGP */
			c1 = fgetc(in);
			if (c1 == 0)
			{
				c1 = fgetc(in);
			}
			else
			{
				/* not a zero, put it back. */

				ungetc(c1, in);
			}
		}
		fclose(in);
	}
}

/* Switch a Little-Endian number byte order to make it Big-Endian */

unsigned int
                o9_int(nbr)
	u16             nbr;
{
#ifndef __BIG_ENDIAN__
	return (((nbr & 0xff00) >> 8) + ((nbr & 0xff) << 8));
#else
	return nbr;
#endif
}

showhead()
{
	int             c;

	puts("\nModule name: ");
	while (c = getc(in))
		putchar(c);
	if (ferror(in))
		ferr(fname);
	printf("\nTyLa/RvAt:   %02x/%02x\n", mc(hd.h_tylan >> 8), mc(hd.h_tylan));
	printf("Asm valid:   %s\n", hd.h_valid ? "No" : "Yes");
	printf("Create date: %.3s %2d, %4d %02d:%02d\n",
	       &("JanFebMarAprMayJunJulAugSepOctNovDec"[(mc(hd.h_date[1]) - 1) * 3]),
	       mc(hd.h_date[2]), 1900 + mc(hd.h_date[0]),
	       mc(hd.h_date[3]), mc(hd.h_date[4]));
	printf("Edition:     %2d\n", hd.h_edit);
	puts("  Section    Init Uninit\n");
	printf("   Code:     %04x\n", o9_int(hd.h_ocode));
	printf("     DP:       %02x   %02x\n", o9_int(hd.h_ddata),
	       o9_int(hd.h_dglbl));
	printf("   Data:     %04x %04x\n", o9_int(hd.h_data),
	       o9_int(hd.h_glbl));
	printf("  Stack:     %04x\n", o9_int(hd.h_stack));
	printf("Entry point: %04x\n", o9_int(hd.h_entry));
}

showglobs()
{
	register unsigned count,
	                offset;
	char            sym[SYMLEN + 1],
	                flag;

	count = getwrd(in);	/* global def count */
	if (gflag)
		printf("\n%u global symbols defined:\n", count);
	while (count--)
	{
		getname(sym);
		flag = getc(in);
		offset = getwrd(in);
		if (gflag)
		{
			printf(" %9s %04x ", sym, offset);
			ftext(flag, DEF);
		}
	}
}

getname(s)
	register char  *s;
{
	while (*s++ = getc(in));
	*s = '\0';
	if (ferror(in))
		ferr(fname);
}

ftext(c, ref)
	char            c;
	int             ref;
{
	printf("(%02x) ", mc(c));
	if (ref & REF)
	{
		if (c & CODLOC)
			puts("in code");
		else
			puts(c & DIRLOC ? "in dp data" : "in non-dp data");
		puts(c & LOC1BYT ? "/byte" : "/word");
		if (c & NEGMASK)
			puts("/neg");
		if (c & RELATIVE)
			puts("/pcr");
	}
	if (ref & DEF)
	{
		if (ref & REF)
			puts(" - ");
		if (c & CODENT)
			puts("to code");
		else
		{
			puts(c & DIRENT ? "to dp" : "to non-dp");
			puts(c & INIENT ? " data" : " bss");
		}
	}
	putchar('\n');
}

showrefs()
{
	register unsigned count,
	                rcount;
	def_ref         ref;
	char            sym[SYMLEN + 1];
	int             fflag;

	count = getwrd(in);
	if (rflag)
		printf("\n%u external references:\n", count);
	while (count--)
	{
		getname(sym);
		rcount = getwrd(in);
		if (rflag)
			printf(" %9s ", sym);
		fflag = 0;
		while (rcount--)
		{
			fread(&ref.r_flag, sizeof(ref.r_flag), 1, in);
			fread(&ref.r_offset, sizeof(ref.r_offset), 1, in);
			if (ferror(in))
				ferr(fname);
			if (rflag && oflag)
			{
				if (fflag)
					puts("           ");
				else
					fflag = 1;
				printf("%04x ", o9_int(ref.r_offset));
				ftext(ref.r_flag, REF);
			}
		}
		if (rflag && !oflag)
			putchar('\n');
	}
}

showlcls()
{
	register unsigned count;
	def_ref         ref;

	count = getwrd(in);
	if (oflag)
		printf("\n%u local references\n", count);
	while (count--)
	{
		fread(&ref.r_flag, sizeof(ref.r_flag), 1, in);
		fread(&ref.r_offset, sizeof(ref.r_offset), 1, in);
		if (ferror(in))
			ferr(fname);
		if (oflag)
		{
			printf("   %04x ", o9_int(ref.r_offset));
			ftext(ref.r_flag, DEF | REF);
		}
	}
}

unsigned int
                getwrd(FILE * fp)
{
	unsigned        Msb,
	                Lsb;
	unsigned int    nbr;

	Msb = getc(fp);
	Lsb = getc(fp);
	nbr = Msb << 8 | Lsb;
	return (o9_int(nbr));
}

error(s1, s2, s3, s4)
	int             s1,
	                s2,
	                s3,
	                s4;
{
	fprintf(stderr, "rdump: ");
	fprintf(stderr, s1, s2, s3, s4);
	putc('\n', stderr);
	exit(1);
}

ferr(s)
{
	error("error reading '%s'", s);
}
