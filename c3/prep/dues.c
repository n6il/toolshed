/*
	$Id: dues.c,v 1.3 2006/09/28 02:34:33 boisy Exp $
*/
#include "prep.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>


#define MAXIFS 32

/* extern char *defdev; */

static short nextncnt;


/* Return the next candidate path for an #include of fname, or NULL when there
   are no more.  Every candidate fits in FNAMSIZE (so it can be copied into an
   include entry); an include directory too long to join with fname is skipped. */
char *nextname(char *fname, char delimiter)
{
	static char nambuf[FNAMSIZE];

	if (delimiter == '"' && nextncnt == -1)
	{
		nextncnt = 0;
		return strcpy(nambuf, fname);	/* doinclude bounds fname */
	}

	while (nextncnt < inclcount)
	{
		if (strlen(incl[nextncnt]) + 1 + strlen(fname) < FNAMSIZE)
		{
			strcpy(nambuf, incl[nextncnt++]);
			strcat(nambuf, "/");
			return strcat(nambuf, fname);
		}
		nextncnt++;
	}
	return NULL;
}


void doinclude(void)
{
	register char c, *nameptr;
	static char fname[FNAMSIZE], msg[FNAMSIZE + 32];
	char *last;
	register FILE *fp;
	int lasterr;

	if (process)
	{										/* ok to do this? */
		skipsp(1);							/* skip spaces */
		symptr = lptr;
		gch(KEEPSP);
		c = cch;
		nameptr = fname;
		if (c == '"' || c == '<')
		{
			gch(SKIPSP);
			if (c == '"')
			{
				nextncnt = -1;
			}
			else
			{
				c = '>';
				nextncnt = 0;
			}
			while (cch && cch != c)
			{
				if (nameptr < fname + FNAMSIZE - 1)
					*nameptr++ = cch;	/* longer names are cut short */
				gch(SKIPSP);
			}
			*nameptr = '\0';
			skipsp(1);	/* get rid of (possible) comment */
			for (last = fname; nameptr = nextname(fname, c); last = nameptr)
			{
				if (fp = fopen(nameptr, "r"))
				{
					register include *nptr;
				
					nptr = (include*)grab(sizeof(include));
					nptr->next = inclptr;
					nptr->lno = lineno;
					nptr->fp = in;
					strcpy(nptr->fname, filename);
					strcpy(nptr->modname, modname);
					strcpy(setfile(filename), nameptr);
					strcpy(modname, makename(nameptr));
					putesc(NEWFNAME, filename, modname);
					inclptr = nptr;
				
					putesc(NEWLINO, 0, 0);
					setline(lineno = 0);
					in = fp;
					return;
				}
				else
				{
					lasterr = errno;
				}
			}
			strcpy(msg, "can't open ");
			strcat(msg, last);
			sprintf(msg+strlen(msg), " (err=%d)", lasterr);
			fatal(msg);
		}
		else
		{
			fatal("incorrect include file syntax");
		}
	}
}


void dodefine(int func)
{
	char name[NAMESIZE];
	
	if (process)
	{
		skipsp(1);
		gch(KEEPSP);
		if (isalpha(cch) || cch == '_')
		{
			getword(name, NAMESIZE);
			if (func == DEFINE)
			{
				/*
				 * ANSI allows "redefinition" if it's not really different,
				 * which will take some hacking...for now, we'll leave it
				 * alone.
				 */
				if (findmac(name))
				{
					lerror("redefined macro");
				}
				else if (strcmp(name, "defined") == 0)
				{
					lerror("can't #define defined");
				}
				else
				{
					addmac(name, 0);
				}
			}
			else
			{
				delmac(name);
			}
		}
		else
		{
			lerror("illegal macro name");
		}
	}
}


int	iftop, elseflag;
static struct
{
	int		oldproc, oldelse, hitaltern;
} ifstack[MAXIFS];


void docond(void)
{
	if (iftop < MAXIFS)
	{
		/*
		 * skipsp(1);
		 * gch(SKIPSP);
		 */
		symptr = lptr;
		ifstack[iftop].oldelse = elseflag;
		if (ifstack[iftop].oldproc = process)
		{
			ifstack[iftop].hitaltern = process = eval();
		}
		else
		{
			ifstack[iftop].hitaltern = FALSE;
		}
		iftop++;
		elseflag = TRUE;
	}
	else
	{
		lerror("#if nesting too deep");
	}
}


void doifdef(int _bool)
{
	char name[NAMESIZE];
	
	if (iftop < MAXIFS)
	{
		skipsp(1);
		gch(SKIPSP);
		symptr = lptr;
		if (isalpha(cch) || cch == '_')
		{
			getword(name, NAMESIZE);
			ifstack[iftop].oldelse = elseflag;
			if (ifstack[iftop].oldproc = process)
				ifstack[iftop].hitaltern = process = _bool ^ !findmac(name);
			else
				ifstack[iftop].hitaltern = FALSE;
			iftop++;
			elseflag = TRUE;
		}
		else
		{
			lerror("illegal #if macro name");
		}
	}
	else
	{
		lerror("#if nesting too deep");
	}
}


void doelif(void)
{
	if (elseflag)
	{
		/*
		 * skipsp(1);
		 * gch(SKIPSP);
		 */
		symptr = lptr;
		if (ifstack[iftop - 1].oldproc && !ifstack[iftop - 1].hitaltern)
			ifstack[iftop - 1].hitaltern = process = eval();
		else
			process = FALSE;
	}
	else
	{
		lerror("no #if for #elif");
	}
}


void doelse(void)
{
	if (elseflag)
	{
		process = ifstack[iftop - 1].oldproc && !ifstack[iftop - 1].hitaltern;
		elseflag = FALSE;
	}
	else
	{
		lerror("no #if for #else");
	}
}


void unstack(void)
{
	if (iftop)
	{
		process = ifstack[--iftop].oldproc;
		elseflag = ifstack[iftop].oldelse;
	}
	else
	{
		lerror("too many #endifs");
	}
}


void doliner(void)
{
	register int i;
	char name[NAMESIZE];

	if (process)
	{
		skipsp(1);
		gch(1);
		getword(name, NAMESIZE);
		if (i = atoi(name))
		{
			setline(lineno = i);
		}
		skipsp(1);
		gch(1);
		getword(name, NAMESIZE);
		if (*name)
		{
			strcpy(setfile(filename), name);
			strcpy(modname, makename(filename));
		}
		putesc(NEWFNAME, filename, modname);
	}
}
