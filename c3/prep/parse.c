/*
	$Id: parse.c,v 1.3 2006/09/28 03:27:09 boisy Exp $
*/
#include "prep.h"
#include <string.h>

extern int	nxtlno;

void preinit(void)
{
	lbase = lptr = line;
	*line = '\0';
	cch = ' ';
	process=1;
	setline(lineno);
	setfile(filename);
}

int getline1(void)
{
	register char *name = holdbuf;
	char pline[LINESIZE];               /* finished output line */
		
	strcpy(lastline,line);              /* save last input line */
	
restart:
	if(newline()) {
		gch(0);
		while(cch == '#') {
			gch(0);
			while(isspace(cch)) gch(0);
			symptr = lptr;
			if(isalpha(cch)) {
				getword(name,NAMESIZE);
				switch(findcmd(name)) {
					case DEFINE:    dodefine(DEFINE); break;
					case INCLUDE:   doinclude(); break;
					case IFDEF:     doifdef(1); break;
					case IFNDEF:    doifdef(0); break;
					case ENDIF:     unstack(); break;
					case IF:		docond(); break;
					case ELIF:		doelif(); break;
					case ELSE:      doelse(); break;
					case UNDEF:     dodefine(UNDEF); break;
					case ASM:       if(process)
										asmflag = 1;
									break;
					case ENDASM:    if(process)
										asmflag = 0;
									break;
					case LINE:      doliner(); break;
					default:        goto badhash;
				}
			} else {
badhash:        error("illegal '#'");
			}
			if(newline() == 0) return EOF;
			else gch(0);
		}
		
		if(cch == '@') {		/* special asmline indicator for OS9 */
			if(process) {
				putesc(ASSLINE,line+1,0);
				goto restart;
			}
		}

		if(process) {
			if(asmflag) {
				putesc(ASSLINE,line,0);
				goto restart;
			} else {
				pptr = pline;
				scan(lptr = line, FALSE, supergch);
				quoexp(line,pline);
				return ' ';
			}
		} else goto restart;
	}
	return EOF;
}

void quoexp(register char *s, register char *t)
{
	while(*s++ = *t++)
		if(s[-1] == HIDER)      /* if the last char was a hidden quote */
			s[-1] = *t++ + 16;  /* then replace with the real quote */
}

char *makename(register char *s)
{
	static char buf[15];
	register char *p=s;
	register int c,i=sizeof buf;
	
	while(c= *s++)
		if(c == '/')
			p = s;

	for(s=buf; (c= *p++) && (i-- > 0); ) {
		if(!isalnum(c))
			c = '_';
		*s++ = c;
	}
	*s = '\0';
	return buf;
}

void getword(register char *word, register int count)
{
	while(--count > 0 && (isalnum(cch) || cch == '_')) {
		*word++ = cch;
		gch(KEEPSP);
	}
	*word = '\0';

/* eat any excess chars from word */
	while(isalnum(cch) || cch == '_')
		gch(KEEPSP);
}

int newline(void)
{
	for(;;) {
		if(_inline()) {
			if(lflag && (asmflag == 0 && line[0] != '@'))
				putesc(SOURCE,line,0);
			return 1;
		}
		fclose(in);
		/* The nxtlno/lineno check can get faked out, so... */
		nxtlno = -1;
		if(inclptr) {
			in = inclptr->fp;
			setline(lineno = inclptr->lno);
			strcpy(setfile(filename),inclptr->fname);
			strcpy(modname,inclptr->modname);
			inclptr = inclptr->next;
			putesc(NEWFNAME,filename,modname);
			putesc(NEWLINO,(char *) lineno,0);
		} else return 0;
	}
}

int gch(register int skflag)
{
	if((cch = *lptr) == 0)
		return 0;
	else if(skflag && skipsp(0))
		cch = ' ';
	else ++lptr;
	return 0;
}

int supergch(register int skflag)
{
	for (;;) {
		gch(skflag);
		if (cch != 0 || !newline())
			break;
		if (skflag == KEEPSP) {
			cch = ' ';
			break;
		}
	}
	return 0;
}

int skipsp(int skflag)
{
	register char c;
	register int flag = 0;
	
	for (;;)
	{
		switch (c = *lptr)
		{
			case '\f':
			case '\t':
			case ' ':
				if (skflag)
				{
					++lptr;
					flag = 1;
					continue;
				}
				return flag;

			case '/':
				if (lptr[1] == '*')
				{
					/* is this what this is? (sic) */
					flag = 1;
					lptr += 2;
					while (((c = *lptr) != '*') || lptr[1] != '/')
					{
						if (c == 0)
						{
							if (_inline() == 0)
							{
								return flag;
							}
						}
						else
						{
							++lptr;
						}
					}
					lptr += 2;
					continue;
				}
				else
				if (lptr[1] == '/')
				{
					/* C++ style comment (like this) */
					flag = 1;
					while (*lptr != 0)
					{
						lptr++;
					}
				}

			default:
				return flag;
		}
	}
}


int _inline(void)
{
	register char *p;
	register int c,count = LINESIZE;

	p = lptr = line;
	if ((c = getc(in)) == EOF)
	{
		if (ferror(in))
			fatal("source file read error");
		else return *p=0;
	}
	
	while (c != '\n' && c != EOF && count-- > 0)
	{
		if (c == '\\')
		{
			if((c = getc(in)) == '\n')
			{
				count++;            /* ..skipped a character */
				setline(++lineno);  /* ..to a new line */
			}
			else
			{
				ungetc(c,in);       /* some other escape sequence */
				*p++ = '\\';
			}
		}
		else *p++ = c;
		c = getc(in);
	}
	
	if (count < 0)
		fatal("source line too long");

	*p = '\0';
	setline(++lineno);

	return 1;
}
