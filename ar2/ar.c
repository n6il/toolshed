
/*
 *------------------------------------------------------------------
 *
 * $Source$
 * $RCSfile$
 * $Revision$
 * $Date$
 * $State$
 * $Author$
 * $Locker$
 *
 *------------------------------------------------------------------
 *
 * Carl Kreider
 * 22305 CR 28
 * Goshen, IN  46526
 * (219) 875-7019
 * (71076.76@compuserve.com, ckreider@skyenet.net, carlk@syscon-intl.com)
 *
 *------------------------------------------------------------------
 * $Log$
 * Revision 1.15  2010/05/28 00:23:07  aaronwolfe
 * use SEEK_* throughout for better portability and readability, from Christian Lesage
 *
 * Revision 1.14  2010/05/28 00:15:06  aaronwolfe
 *  corrects a bug introduced in rev 1.10 that would make the program crash, from Christian Lesage
 *
 * Revision 1.13  2010/05/28 00:08:15  aaronwolfe
 * case-insensitive comparison of the archive suffix (.ar) from Christian Lesage
 *
 * Revision 1.12  2010/05/27 03:49:11  aaronwolfe
 * updated table to print a four digit year - kelly anderson
 *
 * Revision 1.11  2010/05/26 23:20:00  aaronwolfe
 * fix for notdot call at line 534, from Kelly Anderson
 *
 * Revision 1.10  2010/04/04 13:08:56  robertgault
 * Corrects pointer to integer without cast problems. RG
 *
 * Revision 1.9  2008/11/03 15:20:19  robertgault
 * added WIN32 compensation for SYSV
 *
 * Revision 1.8  2008/10/30 15:52:24  boisy
 * Clenaed up warnings in ar2
 *
 * Revision 1.7  2008/10/30 03:08:48  boisy
 * Additional updates
 *
 * Revision 1.6  2007/10/06 06:27:24  tlindner
 * Updated for CYGWIN, Should not have broken anything. :)
 *
 * ----------------------------------------------------------------------
 *
 * Revision 1.5  2006/09/09 01:59:03  boisy
 * Changes to accomodate compiling under Turbo C++
 *
 * Revision 1.4  2006/04/11 01:32:45  boisy
 * Fixed warnings under Linux
 *
 * Revision 1.3  2005/09/16 23:46:35  boisy
 * ar2 now makes under OS X
 *
 * Revision 1.2  1996/07/20 22:15:35  cc
 * Merged in pwz's unixification (Sunos).
 *
 * Revision 1.1  96/07/20  17:10:34  cc
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

/*
 * file archive utility loosely modeled after Kernigan & Plauger
 *
 *  written sometime in late 1986 by Carl Kreider
 *
 *  02/15/87 Richard Pettit
 *       Modified to compile and run under Unix SysV on 680x0 boxes
 *       Has byte order problems on a Vax under SysV
 *
 *  05/28/87 Carl Kreider
 *       Re-organize a bit.  Move system dependent code to one
 *        module.  A few conditionals left in main body.
 *       Deal with byte order problems with special routines to
 *        read and write the header struct in pieces instead of
 *        in one shot.  Non-character (shorts and longs) are
 *        read and written a byte at a time using shifts and
 *        masks to let the machine get the order correct.
 *       Re-write my directory handler to be compatible with
 *        OSK and Richard's.
 *       Put better error checking on I/O in LZ routines.
 *       LZ stuff should be changed to use char array for 'buf'
 *        instead of short int array to make life easier on I/O.
 *
 * 93-07-01 CrK
 *	Lots of work.  Intervening mods not recorded, but this is the
 *	 desired (nee, demanded) rewrite.
 *	o - -o forces old compression and compatibility and overrides
 *		other -b or -s options.
 *  o - -m command moves (adds and deletes) files to the archive
 *  o - -d command deletes files from the archive
 *
 * 95-09-04 CRK
 *		small bug in lz1 found by Steve Bliss
 *		(in de_LZ_1, (char) finchar => (u_char) finchar)
 *
 * 96-07-20 Paul W. Zibaila <pwz@ittc.pgh.wec.com>
 *		changes to run on Sun Sparc and i386
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#if defined(OSK)
# include <sys/dir.h>
#else
# include <sys/types.h>
# include <unistd.h>
#if defined(WIN32)
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#endif
#include "ar.h"
#include "lz1.h"

#if defined(BRAINDEAD)
# undef tolower
# define tolower ck_tolower
#endif

void copy_from(FILE *ifp, FILE *ofp, HEADER *hp);
void proc_opt(char *p);
void proc_cmd(char command, FILE *afp);
void delete(FILE *afp);
void extract(FILE *afp, int flag);
void table(FILE *fp);
void fatal(int code, char *msg, char *arg1, int arg2);
void help(void);
void update(FILE *afp);
int puthdr(FILE *fp, HEADER *hp);

FN		*fnhead = (FN *)NULL;
char	*hid = HID;
char	*suf = SUF;
char	*mod;					/* pointer to module name			*/
char	*archfile;				/* name of archive file				*/
int		all = FALSE;			/* true to access old versions		*/
int		oldmode = FALSE;		/* true forces old compression		*/
int		supflag = 0;			/* 1=no binary 2=none at all		*/
int		debug = FALSE;			/* sets debug level					*/
int		compt;					/* default to new compression		*/
int		rmflag = 0;				/* don't rm file after save			*/
int		zflag = FALSE;			/* true if names come from stdin	*/
/*page*/

char	*emalloc(size_t);

int main(argc, argv)
int		argc;
char	**argv;
	{
	char	command, *p, lc_suf[SUFSIZ + 1];
	int		n, i;
	FILE	*afp;

#if defined(SYSV)
# if !defined(WIN32)
 	setuid(0);
# endif
#else
# ifndef OSK
	pflinit();
# endif
#endif
	setbuf(stdout, 0);
	mod = *argv++;						/* save program name			*/

	if ((argc < 3) || (*(p = *argv++) != '-'))
		help();

	if ('m' == (command = tolower(*++p)))
		command = 'u', ++rmflag;

	lz1_config(13);						/* set up compression defaults	*/
	compt = (13 << 4) | COMP3;
	proc_opt(++p);						/* process command modifiers	*/

	archfile = *argv++;
	n = strlen(archfile);
	
	/* case-insensitive comparison of the archive suffix (.ar) 5/27/10 Christian Lesage */
	strcpy(lc_suf, archfile + n - SUFSIZ);

	for(i=0; i < SUFSIZ; i++)
		lc_suf[i] = tolower(lc_suf[i]);

	if ((strcmp(lc_suf, suf)) != 0)
		archfile = strcat(strcpy(emalloc(n + SUFSIZ + 1), archfile), suf);

	if (get_names(argc -= 3, argv, command == 'u') == 0)
		if (command == 'u')
			fatal(0, "none of the targets found\n", 0, 0);

	if (command == 'u')
		{
		if ((afp = fopen(archfile, F_RP)) == NULL)	/* try old first	*/
			if ((afp = fopen(archfile, F_WP)) == NULL)	/* create it	*/
				fatal(errno, "can't create %s\n", archfile, 0);
		}
	else
		if (command == 'd')
			{
			if ((afp = fopen(archfile, F_RP)) == NULL)
				fatal(errno, "can't find %s\n", archfile, 0);
			}
		else
			{
			if ((afp = fopen(archfile, F_R)) == NULL)
				fatal(errno, "can't find %s\n", archfile, 0);
			}

	proc_cmd(command, afp);				/* process a command			*/

	return 0;
	}
/*page*/
/*
 * process command modifiers
 */

void proc_opt(char *p)
	{
	int		n;

	while (*p)
		switch (tolower(*p++))
			{
			case 'a' :					/* all - even old ones			*/
				all = TRUE;
				break;

			case 'b' :					/* set maxbits to nn			*/
				n = atoi(p);
				lz1_config(n);
				compt = (n << 4) | COMP3;
				while (*p && isdigit(*p))
					++p;				/* eat number					*/
				break;

			case 'd' :
				debug++;				/* increase debug level			*/
				break;

			case 'o' :					/* use old compression method	*/
				oldmode = TRUE;
				break;

			case 's' :					/* suppress compression			*/
				supflag++;
				break;

			case 'z' :					/* get names from stdin			*/
				zflag = TRUE;
				break;

			default  :
				help();
			}

	if (oldmode)
		{
		lz1_config(11);					/* force old bits/code			*/
		compt = COMP1;					/* force old compression		*/
		if (supflag == 0)
			supflag = 1;
		}
	}
/*page*/
/*
 * process the command
 */

void proc_cmd(char command, FILE *afp)
	{
	switch (command)
		{
		case 'd' :						/* delete file(s)				*/
			delete(afp);
			break;

		case 'p' :						/* print member(s)				*/
			extract(afp, 0);
			break;

		case 't' :						/* table of contents			*/
			table(afp);
			break;

		case 'u' :						/* update or add				*/
			update(afp);
			break;

		case 'x' :						/* extract member(s)			*/
			extract(afp, 1);
			break;

		default  :
			help();
		}
	}
/*page*/
/*
 * delete file(s) from the archive
 */

void delete(FILE *afp)
	{
	long	archive_size, ftell(), get_fsize();
	FN		*fnp;
	HEADER	header;
	int		found;

	if (fnhead == NULL)
		fatal(0, "No file(s) specified to delete\n", 0, 0);

	archive_size = get_fsize(fileno(afp));	/* current size of archive	*/

	while ((gethdr(afp, &header)) != EOF)
		{
		found = FALSE;
		/* see if this member of archive is one we want */
		for (fnp = fnhead; fnp && !found; fnp = fnp->fn_link)
			if (TRUE == (found = patmatch(fnp->fn_name, header.a_name, TRUE)))
				{
				int		n;
				long	from_pos, to_pos, saved_pos;

				printf("deleting <%s>\n", header.a_name);

				/* Where in archive to move data */
				saved_pos = to_pos = ftell(afp) - SIZEOF_HEADER;

				/* This is the next file in the archive */
				from_pos = ftell(afp) + header.a_size;

				/* Decrease archive size */
				archive_size -= (SIZEOF_HEADER + header.a_size);

				do	{
					static char buf[BUFSIZ];

					fseek(afp, from_pos, SEEK_SET);	/* next chunk loc	*/
					if ((n = fread(buf, 1, BUFSIZ, afp)) <= 0)
						continue;		/* done with this member at EOF	*/

					from_pos += n;

					fseek(afp, to_pos, SEEK_SET);	/* put it back		*/
					fwrite(buf, 1, n, afp);

					to_pos += n;
					} while (n > 0);

				fseek(afp, saved_pos, SEEK_SET);	/* recover start pos */
				set_fsize(fileno(afp), archive_size);	/* resize archive */
				}

		if (!found)
			fseek(afp, header.a_size, SEEK_CUR);	/* skip if not del	*/
		}
	}
/*page*/
/*
 * extract file(s) from the archive
 *  copy a file from archive and restore it's origional attrs
 */

void extract(FILE *afp, int flag)
	{
	FILE	*ofp;						/* assume just listing			*/
	HEADER	header;
	FN		*fnp;
	FILE	*spl_open();

	if (fnhead == (FN *) NULL)
		stash_name("*");				/* fake for special case		*/

	while ((gethdr(afp, &header)) != EOF)
		{
		for (fnp = fnhead; fnp; fnp = fnp->fn_link)
			if ((patmatch(fnp->fn_name, header.a_name, TRUE) == TRUE)
					|| (header.a_stat != 0 && all == TRUE))
				break;

		if (fnp == 0)
			fseek(afp, header.a_size, SEEK_CUR);	/* file not found			*/
		else
			{
			if (!flag)
				copy_from(afp, stdout, &header);
			else
				{
				printf("extracting <%s>\n", header.a_name);
				ofp = spl_open(&header);
				copy_from(afp, ofp, &header);
#if defined(SYSV) || defined(WIN32)
				fclose(ofp);
				set_fstat(header.a_name, &header.a_attr);
#else
				set_fstat(fileno(ofp), &header.a_attr);
				fclose(ofp);
#endif
				}
			}
		}
	}
/*page*/
/*
 * list a table of contents for the archive file
 *  only show files matching the search mask which are current
 *  unless the all flag is set, whereupon we will show old ones too
 *
 * 5/26/10 updated to print a four digit year - kelly anderson
 */

void table(FILE *fp)
	{
	HEADER	header;
	FN		*fnp;
	long c4tol();
	static char	*attrs[8] =
		{"---", "--r", "-w-", "-wr", "e--", "e-r", "ew-", "ewr"};

	if (fnhead == (FN *) NULL)
		stash_name("*");				/* fake for special case		*/

	printf("                                                    file    file   stored\n");
	printf("  file name                   ver     file date     attr    size    size\n");
	printf("----------------------------- --- ---------------- ------   -----   -----\n");
	while ((gethdr(fp, &header)) != EOF)
		{
		for (fnp = fnhead; fnp; fnp = fnp->fn_link)
			{
			if ((patmatch(fnp->fn_name, header.a_name, TRUE) == TRUE)
					&& (header.a_stat == 0 || all == TRUE))
				printf("%-29s %2d  %04d/%02d/%02d %02d:%02d %s%s %7ld %7ld\n",	/*+ek+*/
					header.a_name, header.a_stat, 
					1900 + header.a_attr.fd_date[0], header.a_attr.fd_date[1],
					header.a_attr.fd_date[2], header.a_attr.fd_date[3], 
					header.a_attr.fd_date[4],
					attrs[(header.a_attr.fd_attr >> 3) & 7],
					attrs[header.a_attr.fd_attr & 7],
					c4tol(header.a_attr.fd_fsize), header.a_size);
			}

		fseek(fp, header.a_size, SEEK_CUR);
		}
	}
/*page*/
/*
 * add new files or replace existing files
 */

void update(FILE *afp)
	{
	FILE	*ifp;
	HEADER	header;
	FN		*fnp;
	int		saved = 0;
	long	bytes, head_pos, tail_pos, copy_to(), c4tol();

	while ((gethdr(afp, &header)) != EOF)
		{
		for (fnp = fnhead; fnp; fnp = fnp->fn_link)
			if (patmatch(fnp->fn_name, header.a_name, TRUE) == TRUE)
				{
				++header.a_stat;		/* mark it older				*/
				fseek(afp, -SIZEOF_HEADER, SEEK_CUR);
				if ((puthdr(afp, &header)) == EOF)
					fatal(errno, "write failure on delete\n", 0, 0);
				}

		fseek(afp, header.a_size, SEEK_CUR);
		}

	for (fnp = fnhead; fnp; fnp = fnp->fn_link)
		{
		if ((ifp = fopen(fnp->fn_name, F_R)) == NULL)
			{
			if (errno == 214)
				continue;				/* a directory, we presume		*/
			else
				fatal(errno, "can't find %s\n", fnp->fn_name, 0);
			}

#if defined(SYSV) || defined(WIN32)
		if (is_dir(fileno(ifp)))	/*It saves the header block otherwise*/
			{ 
			printf("\t<%s> is a directory and IS NOT being archived\n", fnp->fn_name);
			continue;
			}
#endif
		printf("archiving <%s>\n", fnp->fn_name);
		++saved;						/* count the files added		*/
		if ((supflag == 2) || ((supflag == 1) && isobject(ifp)))
			header.a_type = PLAIN;
		else
			header.a_type = compt;

		strcpy(header.a_hid, hid);
		memset(header.a_name, ' ', FNSIZ + 1);
		strcpy(header.a_name, fnp->fn_name);
		get_fstat(fileno(ifp), &header.a_attr);
		header.a_stat = '\0';
		rewind(ifp);
		head_pos = ftell(afp);			/* save for update				*/
		if (puthdr(afp, &header) == EOF)	/* skip ahead				*/
			fatal(errno, "write error on header for %s\n", fnp->fn_name, 0);

		bytes = head_pos + c4tol(header.a_attr.fd_fsize) + SIZEOF_HEADER;
		set_fsize(fileno(afp), bytes);	/* make it big enough for all	*/
		header.a_size = copy_to(afp, ifp, &header);	/* now copy it		*/
		fclose(ifp);
		tail_pos = ftell(afp);			/* save our final position		*/
		fseek(afp, head_pos, SEEK_SET);			/* back up to header pos		*/
/*		if ((fwrite(&header, SIZEOF_HEADER, 1, afp)) == NULL) */
		if (puthdr(afp, &header) == EOF)
			fatal(errno, "write error on header for %s\n", fnp->fn_name, 0);

		fseek(afp, tail_pos, SEEK_SET);			/* go to end of file			*/
		if (rmflag)
			unlink(fnp->fn_name);
		}

	if (saved > 0)
		set_fsize(fileno(afp), tail_pos);	/* now set real file size	*/
	}
/*page*/
/*
 * gather file names from command line or std in
 *  use linked list to avoid finite limit on number of names
 */

int get_names(int ac, char **av, int updating)
	{
	char			*p, *q, *r, buf[80];
	DIR				*dirp;
#if defined(WIN32)
	struct dirent	*dp;
#else
	struct direct	*dp;
#endif
	int				found = 0;

	while (ac--)
		if (!updating || !iswild(*av))
			if (notdot(*av))  /* dereference av - Kelly Anderson - 5/26/10 */
				found += stash_name(*av++);
			else
				printf("Cannot archive %s\n", *av++);
		else
			{
			*(p = buf) = '\0';
			if ((q = strrchr((r = *av++), '/')))
				{
				strcpy(p, r);			/* copy all to buffer			*/
				*(r = strrchr(p, '/')) = '\0';	/* break in two			*/
				++q;					/* pointer to pattern			*/
				}
			else
				{
				q = r;
				r = p;					/* swap pointers				*/
				}

			dirp = opendir(*p ? p : ".");
			if (*p)
				*r++ = '/';				/* set up for append			*/

			while ((dp = readdir(dirp)))
				if (patmatch(q, strcpy(r, dp->d_name), TRUE))
					if (strucmp(r, archfile) && notdot(r))
						found += stash_name(p);

			closedir(dirp);
			}

	if (zflag)
		while (fgets(buf, 80, stdin))
			if (buf[0] != '\0')
				found += stash_name(buf);

	return (found);
	}


/*page*/
/*
 * squirrel a name away in the linked list of targets
 */

int stash_name(char *p)
	{
	FN		*fnp;						/* where to insert new name		*/
	FN		*q;

	if (*p == '/')
		fatal(1, "absolute path illegal <%s>\n", p, 0);

	q = (FN *) emalloc(sizeof(FN) + strlen(p));
	q->fn_link = (FN *) 0;
	strcpy(q->fn_name, p);
	/* first check for empty list or insertion at head					*/
	if ((fnhead == (FN *) NULL) || strcmp(p, fnhead->fn_name) < 0)
		{
		q->fn_link = fnhead;			/* insert at head				*/
		fnhead = q;
		}
	else
		{								/* search for right spot		*/
		fnp = fnhead;
		while (fnp->fn_link != (FN *) NULL)	/* quit on end of list		*/
			if (strcmp(p, fnp->fn_link->fn_name) < 0)	/* here?		*/
				break;					/* found right spot				*/
			else
				fnp = fnp->fn_link;		/* follow chain					*/

		q->fn_link = fnp->fn_link;		/* make insertion here			*/
		fnp->fn_link = q;
		}

	return (1);							/* we saved one name			*/
	}


/*
 * trivial function to check for the illegal names "." and ".."
 *  returns true if name is valid.
 */

int notdot(char *p)
	{
	return (strcmp(p, ".") && strcmp(p, ".."));
	}


/*
 * trivial function to check a file spec for meta characters
 *  returns true if any metachars in file spec
 */

int iswild(char *p)
	{
	return (strchr(p, '?') || strchr(p, '*'));
	}
/*page*/
/*
 * get the next header from the file
 */

int gethdr(FILE *fp, HEADER *hp)
	{
	long	pos;

	if ((fread(hp->a_hid, HIDSIZ + 1, 1, fp) == 0)
			|| (fread(hp->a_name, FNSIZ + 1, 1, fp) == 0)
			|| (readlong(fp, &hp->a_size) == EOF)
			|| ((hp->a_type = getc(fp)) == EOF)
			|| ((hp->a_stat = getc(fp)) == EOF)
			|| (fread(&hp->a_attr, sizeof(FILDES), 1, fp) == 0))
		return (EOF);

	if (strncmp(hp->a_hid, hid, HIDSIZ) != 0)
		{
		if (0 == (pos = (ftell(fp) - SIZEOF_HEADER)))
			fatal(1, "file not archive\n", 0, 0);

		if ((hp->a_hid[0] == 0) || (hp->a_hid[0] == 0x1a))
			fatal(1, "probable XModem padding at $%lX\n", (char *) pos, 0);

		fatal(1, "file damaged - no header at $%lX\n", (char *) pos, 0);
		}

	return (0);
	}


/*
 * put a header on the file
 */

int puthdr(FILE *fp, HEADER *hp)
	{
	if ((fwrite(hp->a_hid, HIDSIZ + 1, 1, fp) == 0)
			|| (fwrite(hp->a_name, FNSIZ + 1, 1, fp) == 0)
			|| (writelong(fp, hp->a_size) == EOF)
			|| (putc(hp->a_type, fp) == EOF)
			|| (putc(hp->a_stat, fp) == EOF)
			|| (fwrite(&hp->a_attr, sizeof(FILDES), 1, fp) == 0))
		return (EOF);

	return (0);
	}
/*page*/
/*
 * here we will recreate a tree that was collapsed into the archive file
 */

FILE	*spl_open(hp)
HEADER	*hp;
	{
	char	buf[FNSIZ + 3];
	FILE	*ofp;
	char	*p;
	long	c4tol();

	p = hp->a_name;
	while ((p = strchr(p, '/')))
		{
		*p = '\0';						/* truncate temporarily			*/
		if (assureDir(hp->a_name))		/* create it if not there		*/
			fatal(errno, "can't make <%s>\n", hp->a_name, 0);

		*p++ = '/';						/* put back the delim			*/
		}

	strcpy(buf, hp->a_name);
	if (hp->a_stat)
		sprintf(&buf[strlen(buf)], ".%d", hp->a_stat);	/* make unique	*/

	if ((ofp = fopen(buf, F_W)) == NULL)
		fatal(errno, "create failure on %s\n", buf, 0);

	set_fsize(fileno(ofp), c4tol(hp->a_attr.fd_fsize));
	return (ofp);
	}
/*page*/
/*
 * copy an archived file from an archive
 */

void copy_from(FILE *ifp, FILE *ofp, HEADER *hp)
	{
	long	bytes = hp->a_size;
	int		byt;

	switch (hp->a_type & 0x0f)
		{
		case PLAIN :
			while (bytes--)
				{
				if ((byt = getc(ifp)) == ERROR)
					fatal(errno, "read error while copying\n", 0, 0);

				if (putc(byt, ofp) == ERROR)
					fatal(errno, "write error while copying\n", 0, 0);
				}
			break;

		case COMP1 :
		case COMP3 :
			byt = (hp->a_type >> 4) & 0x0f;
			lz1_config(byt ? byt : 11);
			byt = de_LZ_1(ifp, ofp, bytes);
#if defined(DEBUG)
			if (debug > 1)
				dump_otbl();
#endif
			switch (byt)
				{
				case NOT_AR :
					fatal(1, "not an archive or archive damaged\n", 0, 0);

				case RERR :
					fatal(1, "read error on archive\n", 0, 0);

				case WERR :
					fatal(1, "write error on output\n", 0, 0);
				}
			break;

		default  :
			fatal(1, "unknown compression method\n", 0, 0);
		}
	}
/*page*/
/*
 * copy an file to an archive
 */

long	copy_to(ofp, ifp, hp)
FILE	*ofp, *ifp;
HEADER	*hp;
	{
	long	bytes = 0;
	int		byt;

	switch (hp->a_type & 0x0f)
		{
		case PLAIN :
			while ((byt = getc(ifp)) != ERROR)
				if (putc(byt, ofp) == ERROR)
					fatal(errno, "write error while copying\n", 0, 0);
				else
					++bytes;

			if (ferror(ifp))
				fatal(errno, "read error while copying\n", 0, 0);
			break;

		case COMP3 :
		case COMP1 :
			byt = LZ_1(ifp, ofp, &bytes);
#if defined(DEBUG)
			if (debug > 1)
				dump_itbl();
#endif
			switch (byt)
				{
				case RERR :
					fatal(1, "read error on input file\n", 0, 0);

				case WERR :
					fatal(1, "write error on archive\n", 0, 0);

				case TBLOVF :
					fatal(1, "string table overflow on compression\n", 0, 0);
				}
			break;

		default  :
			fatal(1, "unknown compression method\n", 0, 0);
		}

	return (bytes);
	}
/*page*/
/*
 * get memory from the system or die trying
 */

char	*emalloc(n)
size_t		n;
	{
	char	*p;

	if ((p = malloc(n)) == NULL)
		fatal(errno, "Can't get memory\n", 0, 0);

	return (p);
	}


/*
 * print a fatal error message and exit
 */

void fatal(int code, char *msg, char *arg1, int arg2)
	{
	fprintf(stderr, "%s: ", mod);
	fprintf(stderr, msg, arg1, arg2);
	exit(code);
	}

/*page*/
/*
 * provide usage info for this command
 */

static char	*hlpmsg[] = {
	"ar2 from Toolshed " TOOLSHED_VERSION "\n",
	"Ar V2.02 - archive file manager\n",
	"Usage:  ar2 -<cmd>[<modifier>] archive [file .. ]\n",
	"      <cmd> is one of the following:\n",
	"         d   delete file(s) from the archive\n",
	"         m   move file(s) to archive (add and delete)\n",
	"         p   print file(s) from the archive\n",
	"         t   show table of contents for archive\n",
	"         u   update/add file(s) to the archive\n",
	"         x   extract file(s) from the archive\n",
	"      <modifier> is one of the following:\n",
	"         a   all versions (for extract)\n",
	"         bnn set max bits to 'nn' (12 default)\n",
	"         d   incrment debug level\n",
	"         o   default to 'old' archives\n",
	"         s   once, suppress binary; twice, suppress all file compression\n",
	"         z   read names for <cmd> from std in\n",
	"\n      File names can include the meta chars ",
	"* and ?, or path lists.\n",
	0};

void help(void)
	{
	register char	**p;

	for (p = hlpmsg; *p; ++p)
		fputs(*p, stderr);

	exit (1);
	}
