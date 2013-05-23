/*
 * c3 - The C-Cubed Executive
 *
 * 2005 The C-Cubed Project
 */

#include <stdio.h>
#ifdef UNIX
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#define direct
#define vfork fork
#endif

#define version "0.9"
#define message "C-Cubed: The CoCo C Compiler"


#define MAXARGS        64
#define MAXLIBS        8

#define PREPNAME "c3prep"
#define COMPNAME "c3comp"
#define OPTZNAME "c3opt"
#define ASMBNAME "rma"
#define LINKNAME "rlink"

#define CSTART  "/lib/cstart.r"
#define CLIB    "/lib/clib.l"
#define SYSLIB  "/lib/sys.l"
#define CLIBN	"/lib/clibn.l"
#define MATH1	"/lib/math1.l"

direct short
                kflag, lflag, oflag, tflag, iflag, sflag, nflag, rflag,
                aflag, mflag, pflag, qflag, dflag, xflag = 0, libcount,
                term, pid, nocode, yflag = 0;
direct short    optflag = 1;
direct int      wstat;
char           *libs[MAXLIBS];
char           *snames[MAXARGS];
char            suffs[MAXARGS];
char           *defines[MAXARGS];
char            temp[20] = "ctmp.XXXXXX";
char            temp1[200];
char            temp2[200];
char            temp3[200];
direct char
               *srcptr, *dstptr, *devptr, *vptr, *memsize, *outmname, *edition;

char		*incdirs[32];
int		inccount = 0;
char            dest[128];
char            source[60];
char            outname[60];
char            alt_comp[10];
#ifdef UNIX
char            tdirname[60] = "/tmp/";
#else
char            tdirname[60];
#endif
char            odirname[60];	/* .r output directory */
char            arglst[1024];
direct int      argsize;
direct char    *argptr;
direct int      scount, defcount, rdel;
#ifndef UNIX
extern char    *sysdrive(), *attach(), *detach();
#else
static char    *sysdrive();
void uchain();
#endif
void dofork();
void error();
void setsuf();
int getsuf();
void addarg();
void endargs();
void opts();

void tidy()
{
	/* first kill any running child process */
	if (pid)
	{
		kill(pid, 2);
	}

	/* next deal with the terminal */
	if (term)
	{
		/*
		 * the fiddling for 'c.prep' has not been undone and we have
		 * to close the file before it can be deleted
		 */
		close(1);
		dup(term);
	}

	if (srcptr)
	{
		unlink(srcptr);
	}

	if (dstptr)
	{
		unlink(dstptr);
	}

	return;
}

void errexit(stat)
{
	tidy();
	exit(stat);
}

int main(argc, argv)
	int             argc;
	char          **argv;
{
	register char  *p, **pp;
	int             c, count, j;
	int             assdel;

	if (argc == 1)
	{
		fprintf(stderr, "%s\nVersion %s\n", message, version);
	}

	count = 0;


	while (--argc > 0 && ++count < MAXARGS)
	{
		if (*(p = *++argv) == '-')
		{
			while (*++p)
				switch (*p)
				{
				case 'y':
					yflag = 1;
					break;
				case 'd':
				case 'u':
					if (p[1])
						defines[defcount++] = p - 1;
					goto done;
				case 'n':
					*--p = '-';
					outmname = p;
					goto done;
				case 's':
					sflag = 1;
					break;
				case 'c':
					lflag = 1;
					break;
				case 'o':
					optflag = 0;
					break;
				case 'r':
					rflag = 1;
					if (*(p + 1) == '=')
					{
						strcpy(odirname, p + 2);
						strcat(odirname, "/");
						goto done;
					} else
						break;
				case 'a':
					aflag = 1;
					break;
				case 'p':
					pflag = 1;
					break;
				case 'q':
					qflag = 1;
					break;
				case 'x':
					xflag = 0;
					break;
				case 'k':
					kflag = 1;
					if (*++p == '=')
					{
						p++;
					}
					strcpy(alt_comp, p);
					goto done;
				case 'e':
					pp = &edition;
					goto garg;
				case 'm':
					pp = &memsize;
					*p &= 0x5f;	/* for linker */
			garg:		if (p[1])
					{
						*--p = '-';
						*pp = p;
					}
					goto done;
				case 'f':
					if (*++p == '=')
					{
						strcpy(outname, p + 1);
						if (outname[0] == '\0')
							goto done;
						if ((c = getsuf(outname)) == 'c' || c == 'r')
							error("Suffix '.%c' not allowed for output",
							      c);
					}
					goto done;
				case 'l':
					if (p[1] == '=')
					{
						if (libcount == MAXLIBS)
						{
							error("Too many libraries");
						}
						*--p = '-';
						libs[libcount++] = p;
					}
					goto done;
				case 't':
					if (*++p == '=')
					{
						strcpy(tdirname, p + 1);
						strcat(tdirname, "/");
						if (tdirname[0] == '\0')
							goto done;
						++tflag;
					}
					goto done;
				case 'v':
					*p |= 0x60;
					if (p[1])
					{
						*--p = '-';
						incdirs[inccount++] = p;
						vptr = p;
					}
					goto done;
				case 'i':
					iflag++;
					break;
				default:
					opts();
					error("unknown flag : -%c\n", *p);
				}
	done:		;
		} else
			switch (c = getsuf(*argv))
			{
			case 'r':
				rdel = 1;
			case 'a':
			case 'c':
				suffs[scount] = c;
				snames[scount] = *argv;
				++scount;
				break;
			default:
				error("%s : no recognised suffix", *argv);
			}
	}

	/* cross compiler target defines */
	defines[defcount++] = "-DOS9";

	if (scount == 0)
	{
		fprintf(stderr, "no files!\n");
		exit(0);
	}
	if ((devptr = sysdrive()) == 0)
	{
		error("Cannot find default system drive");
	}

	if (aflag && rflag)
	{
		error("incompatible flags", NULL);
	}

	if (outname[0] && (scount > 1) && (aflag || rflag))
	{
		error("%s : output name not applicable", outname);
	}

	if (outname[0] == '\0')
	{
		if (scount == 1)
			strcpy(outname, snames[0]);
		else
			strcpy(outname, "output");
	}
	mkstemp(temp);
	strcpy(temp2, temp);
	strcat(temp2, ".m");
	strcpy(temp3, temp);
	strcat(temp3, ".r");

#ifdef UNIX
	signal(2, errexit);
#else
	intercept(errexit);
#endif
/*	nice(); */
	for (count = 0; count < scount; ++count)
	{
		if (!qflag)
		{
			fprintf(stderr, "\n'%s'\n", snames[count]);
		}

		if (suffs[count] == 'c')
		{
			int i;


			assdel = 1;

			argptr = arglst;

			for (i = 0; i < inccount; i++)
			{
				addarg(incdirs[i]);
			}

			strcpy(dest, "-v=");
			strcat(dest, devptr);
			strcat(dest, "/defs");
			addarg(dest);

			strcpy(dest, tdirname);	/* tempfile directory */
			strcat(dest, temp2);
			setsuf(dest, 'm');
			if (lflag)
				addarg("-l");
			if (edition)
				addarg(edition);

			for (j = 0; j < defcount;)
				addarg(defines[j++]);
			addarg(snames[count]);

			term = dup(1);
			close(1);
			if (creat(dest, 3) != 1)
			{
				error("can't create temporary file: '%s'", dest);
			}
#ifdef UNIX
			chmod(dest, 0644);
#endif
			endargs();

			srcptr = NULL;
			dstptr = dest;
			dofork(PREPNAME, "pre-processor", 1);
			close(1);
			dup(term);
			close(term);
			term = 0;

			strcpy(source, dest);
			argptr = arglst;

			srcptr = source;
			addarg(source);
			if (sflag)
			{
				addarg("-s");
			}

			if (xflag)
			{
				addarg("-t");
			}

			if (nocode)
			{
				addarg("-n");
				strcpy(dest, "/nil");
				assdel = 0;
			} else if (aflag)
			{
				strcpy(dest, snames[count]);
				setsuf(dest, 'a');
			} else
				setsuf(dest, 'a');

			strcpy(temp1, "-o=");
			strcat(temp1, dest);
			addarg(temp1);
			if (pflag)
				addarg("-p");

			endargs();

			dstptr = dest;

			dofork(kflag ? alt_comp :
		      	COMPNAME, "compiler mainline", 0);

			unlink(source);
		} else
			assdel = 0;

		if (aflag || nocode || suffs[count] == 'r')
		{
			dstptr = NULL;
			continue;
		}
		if (suffs[count] == 'a')
		{
			strcpy(source, snames[count]);
			srcptr = NULL;
		} else
		{
			strcpy(source, dest);
			srcptr = source;
		}
		if (optflag)
		{
			argptr = arglst;

			addarg(source);
			strcpy(dest, temp2);
			setsuf(dest, 'o');
			addarg(dest);

			endargs();

			dstptr = dest;
			dofork(OPTZNAME, "optimizer", 0);

			if (assdel)
				unlink(source);
			strcpy(source, dest);
			srcptr = source;
		}
		argptr = arglst;

		addarg(source);

		if (scount == 1 && rflag == 0)
		{
			strcpy(dest, tdirname);
			strcat(dest, temp3);
		} else
		{
			setsuf(snames[count], 'r');
			strcpy(dest, snames[count]);
		}
		strcpy(temp1, "-q -o=");
		strcat(temp1, odirname);
		strcat(temp1, dest);
		addarg(temp1);

		endargs();
		dstptr = dest;
		dofork(ASMBNAME, "assembler", 0);

		if (assdel)
			unlink(source);
	}

	if (nocode || aflag || rflag)
		exit(0);

	argptr = arglst;

	if (yflag == 0)
	{
		strcpy(temp1, devptr);
		strcat(temp1, CSTART);
		addarg(temp1);
	}

	if (scount == 1 && suffs[0] != 'r')
	{
		srcptr = dest;
		addarg(dest);
	} else
	{
		srcptr = NULL;
		for (count = 0; count < scount; ++count)
			addarg(snames[count]);
	}

	strcpy(temp1, "-o=");
	setsuf(outname, 0);

	/*
	        fprintf(stderr,"\n'%s'\n",outname);
	*/
	strcat(temp1, outname);

	addarg(temp1);

	for (j = 0; j < libcount;)
		addarg(libs[j++]);

	strcpy(temp1, "-l=");
	strcat(temp1, devptr);

	if (yflag == 0)
	{
		if (xflag)
		{
			strcat(temp1, CLIBN);	/* no-traps c library */
			strcat(temp1, " -l=");
			strcat(temp1, devptr);
			strcat(temp1, MATH1);	/* and math1 library */
		} else
		{
			strcat(temp1, CLIB);	/* c library */
		}
	}
	else
	{
		addarg("-y");
	}

	strcat(temp1, " -l=");
	strcat(temp1, devptr);
	strcat(temp1, SYSLIB);	/* system library */
	addarg(temp1);

	if (outmname)
	{
		addarg(outmname);
	}

	if (memsize)
	{
		addarg(memsize);
	}

	if (edition)
	{
		addarg(edition);
	}

	endargs();
	dstptr = NULL;
	dofork(LINKNAME, "linker", 0);
	/*
	        chmod(outname,15);
	*/

	tidy();

	return 0;
}

void dofork(s, s1, stat)
	char           *s, *s1;
	int            stat;
{
	if (!qflag)
	{
		fprintf(stderr, "%s:\n", s);
	}

	if (iflag)
	{
		fprintf(stderr, arglst);
		putc('\n', stderr);
	}
#ifdef UNIX
	if ((pid = vfork()) == -1)
		error("cannot execute the %s", s1);
	else if (pid)
	{
		wait(&wstat);
		pid = 0;
#if 0
		if ((((unsigned) wstat) >> 8) & 0xff > stat)
			errexit(wstat);
#else
		if (WIFEXITED(wstat) == 1 && WEXITSTATUS(wstat) != 0)
		{
			errexit(wstat);
		}
#endif

	} else
		uchain(s, arglst);
#else
	if ((pid = os9fork(s, argsize, arglst, 1, 1, 0, 0)) < 0)
	{
		error("cannot execute the %s", s1);
	}

	wait(&wstat);
	pid = 0;
	if (wstat > stat)
	{
		errexit(wstat);
	}
#endif
}

#ifdef UNIX
void uchain(prog, argstr)
	char           *prog, *argstr;
{
	char           *args[50], **argv = args;

	*argv++ = prog;
	for (*argv = argstr; *argstr; ++argstr)
	{
		if (*argstr == ' ')
		{
			*argv++ = argstr + 1;
			*argstr = '\0';
		}
	}
	*argv = NULL;
	if (execvp(prog, args) == -1)
	{
		fprintf(stderr, "chain failed. err=%d\n", errno);
		exit(errno);
	}
}
#endif

void error(s1, s2)
	char           *s1, *s2;
{
	int             err = errno;

	fprintf(stderr, s1, s2);
	putc('\n', stderr);
	errexit(err ? err : 1);
}

void setsuf(s, c)
	register char  *s;
	char            c;
{
	while (*s++);
	if (s[-3] != '.')
		return;
	if (c == '\0')
	{
		s[-3] = '\0';
	}
	else
	{
		s[-2] = c;
	}
}

int getsuf(s)
	register char  *s;
{
	register int    count;
	char            c;

	count = 0;
	while ((c = *s++) != 0)
	{
		if (c == '/')
			count = 0;
		else
			++count;
	}

	if (count <= 29 && count > 2 && s[-3] == '.')
		return s[-2] | 0x40;
	else
		return 0;
}

void addarg(s)
	register char  *s;
{
	register char  *p = argptr;

	*p++ = ' ';
	while ((*p++ = *s++) != 0);
	argptr = p - 1;
}

void endargs()
{
#ifndef UNIX
	*argptr++ = '\n';
#endif
	*argptr = '\0';
	argsize = argptr - arglst;
}

static char    *cmds[] = {
	"\nc3 [opts] [files] [opts]\n",
	"Options:\n",
	"    -a         Compile to assembler files\n",
	"    -c         Leave comments in assembler source\n",
	"    -d<name>   Define a name for preprocessor\n",
	"    -u<name>   Undefine a predefined name\n",
	"    -r[=<dir>] Compile/assemble to relocatable files\n",
	"               (Optional target directory may appear)\n",
	"    -o         Don't run object code improver\n",
	"    -s         No runtime stack checking\n",
	"    -e=<n>     Edition number for output module\n",
	"    -m=<n>[K]  Additional memory for stack in output module\n",
	"    -f=<name>  Output file name\n",
	"    -n=<name>  Output module name\n",
	"    -l=<name>  Additional library to search\n",
	"    -v=<dir>   Search directory for preprocessor < > includes\n",
	"    -t=<dir>   Directory for temporary files\n",
	"    -x         Cause compiler to use traps for math1 calls\n",
	"    -y         Don't add cstart.r and standard libs\n",
	"    -q         Quiet mode. Don't output command names\n",
	"    -i         Verbose mode. Output exact command executed\n",
};

char          **cmdsend = cmds + (sizeof cmds) / (sizeof(char **));

void opts()
{
	register char **p = cmds;

	while (p < cmdsend)
	{
		fputs(*p++, stderr);
	}
}

#if 0
void nice()
{
}
#endif

#ifndef UNIX
char           *devices[] = {
	"/dd",
	"/h0",
	"/d0",
	0,
};

static char    *
sysdrive()
{
	static char     buf[20];
	register char **p = devices;
	int             devptr;

	while (*p)
	{
		if ((devptr = attach(*p + 1, 1)) != -1)
		{
			detach(devptr);
			break;
		} else
			++p;
	}
#ifdef OSK
	return *p;
#else
	strhcpy(buf, *p);
	return buf;
#endif
}
#else
static char    *
sysdrive()
{
	static char *c3base;
	static int firsttime = 1;

	if (firsttime == 1)
	{
		firsttime = 0;
		c3base = getenv("C3BASE");

		if (c3base == NULL)
		{
			c3base = "/opt/c3";
		}
	}

	return (c3base);
}
#endif
