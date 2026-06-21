/*
 * c3 - The C-Cubed Executive
 *
 * 2005 The C-Cubed Project
 */

#include <stdio.h>

/* `direct` is a no-op on every target: the OS-9 direct page aliases the stdio
   iob and comes up as garbage, corrupting the driver's globals (flags, scount,
   the arg pointers); keep them in normal, crt0-zeroed data (same decision as
   comp/cj.h and prep). */
#define direct

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
#define vfork fork
#endif

#define version "0.9"
#define message "C-Cubed: The CoCo C Compiler"


#define MAXARGS        64
#define MAXLIBS        8

#define PREPNAME "c3prep"
#define COMPNAME "c3comp"
#define COMP1NAME "c3comp1"
#define COMP2NAME "c3comp2"
/* The working optimiser is "c3optx" (built from optexp/); the original
   "c3opt" (opt/) is incomplete and not built. */
#define OPTZNAME "c3optx"
#ifdef UNIX
/* On a modern host there is no rma/rlink; default to the lwasm/lwlink
   back-end wrappers (lwbackend/c3-as, c3-ld).  Override any of these with
   the C3_OPT / C3_ASM / C3_LINK environment variables. */
#define ASMBNAME "c3-as"
#define LINKNAME "c3-ld"
#else
#define ASMBNAME "rma"
#define LINKNAME "rlink"
#endif

#define CSTART  "/lib/cstart.r"
#define CLIB    "/lib/clib.l"
#define SYSLIB  "/lib/sys.l"
#define CLIBN	"/lib/clibn.l"
#define MATH1	"/lib/math1.l"

/* Where the host driver looks for defs/ (and lib/) when $C3BASE is unset.
   The install builds this to match its PREFIX; the fallback is the standard
   /usr/local prefix.  (The OS-9 build probes drives instead - see sysdrive.) */
#ifndef C3BASE_DEFAULT
#define C3BASE_DEFAULT "/usr/local/share/toolshed"
#endif

direct short
                kflag, lflag, oflag, tflag, iflag, sflag, nflag, rflag,
                aflag, mflag, pflag, qflag, gflag, xflag = 0, libcount,
                term, pid, nocode, twoflag, yflag = 0;
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
/* All file-local functions have internal linkage and abstract ANSI prototypes
   (parameter TYPES only, no names: c3 rejects a parameter named twice).
   No-argument functions use ANSI (void); error() keeps K&R () because it is
   variadic in use and c3 has no <stdarg.h>.  main is the only non-static
   function and needs no prototype. */
#ifndef UNIX
extern char    *sysdrive(void), *attach(), *detach();
#else
static char    *sysdrive(void);
static void uchain(char *, char *);
#endif
static void tidy(void);
static void errexit(int);
static void dofork(char *, char *, int);
static void error();		/* K&R: variadic-in-use, but c3/OS-9 has no varargs */
static void rmtemp(char *);
static void setsuf(char *, char);
static int getsuf(char *);
static void addarg(char *);
static void endargs(void);
static void opts(void);

static void tidy(void)
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
		rmtemp(srcptr);
	}

	if (dstptr)
	{
		rmtemp(dstptr);
	}

	return;
}

static void errexit(int stat)
{
	tidy();
	exit(stat);
}

int main(int argc, char **argv)
{
	register char  *p, **pp;
	int             c, count, j;
	int             assdel;
	char           *optzname, *asmbname, *linkname, *comp1name, *comp2name;

	/* tool names, overridable from the environment (see the #defines) */
	if ((optzname = getenv("C3_OPT"))  == NULL) optzname = OPTZNAME;
	if ((asmbname = getenv("C3_ASM"))  == NULL) asmbname = ASMBNAME;
	if ((linkname = getenv("C3_LINK")) == NULL) linkname = LINKNAME;
	if ((comp1name = getenv("C3_COMP1")) == NULL) comp1name = COMP1NAME;
	if ((comp2name = getenv("C3_COMP2")) == NULL) comp2name = COMP2NAME;

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
				case 'g':	/* debug: echo every sub-command in
						   full and keep all intermediate
						   files (do not unlink them) */
					gflag = 1;
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
				case '2':
					twoflag = 1;
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
#ifndef UNIX
	/* Default the temp-file directory to the system drive, so the .r object
	   the driver hands to rlink is an ABSOLUTE path: rlink fails silently on
	   a relative object (or relative -o) on OS-9.  -t=<dir> still overrides. */
	if (tdirname[0] == '\0')
	{
		strcpy(tdirname, devptr);
		strcat(tdirname, "/");
	}
#endif
	if (gflag)
	{
		fprintf(stderr, "+ sysdrive=%s  tmpdir=\"%s\"  outdir=\"%s\"\n",
			devptr, tdirname, odirname);
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
#ifdef UNIX
	/* On the host mkstemp() both fills in a unique name AND creates the file,
	   returning an open fd.  We only want the NAME - the .m/.r intermediates
	   are derived from it below - so close the fd and remove the empty base
	   file, otherwise every run leaks a 0-byte ctmp.XXXXXX into the cwd. */
	{ int tfd = mkstemp(temp); if (tfd >= 0) close(tfd); unlink(temp); }
#else
	mkstemp(temp);		/* OS-9: mkstemp is mktemp - generates a name only */
#endif
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
			char ilfile[128];


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

			/* compute the compiler's final output (.a) name */
			if (nocode)
			{
				strcpy(dest, "/nil");
				assdel = 0;
			} else if (aflag)
			{
				strcpy(dest, snames[count]);
				setsuf(dest, 'a');
			} else
				setsuf(dest, 'a');

			if (twoflag && !nocode)
			{
				/*
				 * two-pass: c3comp1 emits the intermediate language
				 * to a temp file, c3comp2 reads it and generates the
				 * assembler.  The IL temp shares the .m basename with
				 * an 'i' suffix.  Same one-process-at-a-time,
				 * file-between-stages model as the other stages -
				 * which is what lets it run on OS-9 Level 2, where
				 * the single-pass compiler is too big to load.
				 */
				strcpy(ilfile, source);
				setsuf(ilfile, 'i');

				/* pass 1: preprocessed source -> IL */
				argptr = arglst;
				srcptr = source;
				addarg(source);
				strcpy(temp1, "-o=");
				strcat(temp1, ilfile);
				addarg(temp1);
				endargs();
				dstptr = ilfile;
				dofork(comp1name, "compiler pass 1", 0);
				rmtemp(source);

				/*
				 * pass 2: IL -> assembler.  -s (stack-check
				 * suppression) is a codegen flag, so it is handed
				 * to this pass, not pass 1.
				 */
				argptr = arglst;
				srcptr = ilfile;
				addarg(ilfile);
				if (sflag)
					addarg("-s");
				strcpy(temp1, "-o=");
				strcat(temp1, dest);
				addarg(temp1);
				endargs();
				dstptr = dest;
				dofork(comp2name, "compiler pass 2", 0);
				rmtemp(ilfile);
			} else
			{
				/* single-pass compiler */
				argptr = arglst;
				srcptr = source;
				addarg(source);
				if (sflag)
					addarg("-s");
				if (xflag)
					addarg("-t");
				if (nocode)
					addarg("-n");
				strcpy(temp1, "-o=");
				strcat(temp1, dest);
				addarg(temp1);
				if (pflag)
					addarg("-p");
				endargs();
				dstptr = dest;
				dofork(kflag ? alt_comp : COMPNAME,
				       "compiler mainline", 0);
				rmtemp(source);
			}
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
			dofork(optzname, "optimizer", 0);

			if (assdel)
				rmtemp(source);
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
		dofork(asmbname, "assembler", 0);

		if (assdel)
			rmtemp(source);
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

	setsuf(outname, 0);
#ifndef UNIX
	/* Make outname absolute (in place): rlink silently produces nothing for a
	   relative -o on OS-9, and the chmod after the link must name the same
	   file.  Skip if already absolute (a user-given -f=/dd/foo).  Build it in
	   temp2, NOT dest: dest is the object temp that srcptr points at and tidy()
	   deletes - clobbering it would make tidy() delete the output instead. */
	if (outname[0] != '/')
	{
		strcpy(temp2, devptr);
		strcat(temp2, "/");
		strcat(temp2, outname);
		strcpy(outname, temp2);
	}
#endif
	strcpy(temp1, "-o=");
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
	dofork(linkname, "linker", 0);
#ifndef UNIX
	/* mark the linked module executable (r/w/e + public read): rlink does not
	   set the OS-9 execute attribute itself, so the output was not runnable
	   without a manual `attr -e'.  outname was made absolute above. */
	chmod(outname, 15);
#endif

	tidy();

	return 0;
}

static void dofork(char *s, char *s1, int stat)
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
	if (gflag)	/* debug: log the exact command (tool + full arglist).
			   arglst is passed as data, not a format string, so a
			   '%' in a path can't misfire (unlike the iflag line). */
	{
		fprintf(stderr, "+ %s%s\n", s, arglst);
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
static void uchain(char *prog, char *argstr)
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

/*
 * error() is variadic in use (a printf format plus a single optional
 * argument), but the c3 6809 compiler that cross-builds this driver for OS-9
 * has no <stdarg.h> / varargs support.  So it stays in the K&R form that both
 * c3 and the host accept: a fixed (format, arg) definition reached through an
 * unprototyped declaration, letting callers pass the format alone.
 */
static void error(char *s1, char *s2)
{
	int             err = errno;

	fprintf(stderr, s1, s2);
	putc('\n', stderr);
	errexit(err ? err : 1);
}

/* remove an intermediate/temporary file - but under -g (debug) keep every one
   and say so, so a failing build can be taken apart stage by stage. */
static void rmtemp(char *f)
{
	if (gflag)
	{
		fprintf(stderr, "+ keep %s\n", f);
		return;
	}
	unlink(f);
}

static void setsuf(register char *s, char c)
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

static int getsuf(register char *s)
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

static void addarg(register char *s)
{
	register char  *p = argptr;

	*p++ = ' ';
	while ((*p++ = *s++) != 0);
	argptr = p - 1;
}

static void endargs(void)
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
	"    -g         Debug. Echo every sub-command and keep all temp files\n",
	"    -2         Two-pass (c3comp1+c3comp2; required on OS-9 L2)\n",
};

char          **cmdsend = cmds + (sizeof cmds) / (sizeof(char **));

static void opts(void)
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
	"/j0",
	0,
};

static char    *
sysdrive(void)
{
	int i;
	char probe[32];
	FILE *f;

	/* Return the drive that actually holds the c3 toolchain - the one whose
	   cmds/ has the assembler - NOT merely the first that attaches.  The boot
	   drive (/dd) always attaches but, during development, the toolchain may
	   live on a slave (/j0); a bare attach-probe wrongly picked the boot drive
	   and then looked for defs/lib that were not there.  Probe a stable file
	   present on every c3 disk (cmds/rma) so the right drive is found wherever
	   it is mounted; opening a path auto-attaches its device. */
	for (i = 0; devices[i] != 0; i++) {
		strcpy(probe, devices[i]);
		strcat(probe, "/cmds/rma");
		if ((f = fopen(probe, "r")) != NULL) {
			fclose(f);
			return devices[i];
		}
	}
	return 0;
}
#else
static char    *
sysdrive(void)
{
	static char *c3base;
	static int firsttime = 1;

	if (firsttime == 1)
	{
		firsttime = 0;
		c3base = getenv("C3BASE");

		if (c3base == NULL)
		{
			c3base = C3BASE_DEFAULT;
		}
	}

	return (c3base);
}
#endif
