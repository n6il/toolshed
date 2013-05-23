/* COpt2 Version 1.0 - A generic peephole optimizer */
/* Copyright (C) 1994 by Vaughn Cato */
/* Copying is permitted for non-profit use only. */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>
#include "variable.h"
#include "copt2.h"
#include "expr.h"


int             infile = 0;
char            inbuf[INBUF_SIZE * 2 + 1];
char           *inbufp;
char           *inbufend;
char           *bufmark = 0;
int             matchstart = -1;
char           *maxreplptr = inbuf;
int             ateof = 0;

char            patbuf[10000];
char           *curpat = 0;

int             debug = 0;

Variable        variables[MAX_VAR];
int             varcount = 0;

PatTree         patheap[PATHEAPSIZE];
PatTree        *pattree = 0;
PatTree        *patheapptr = patheap;
char           *matchpat = 0;



#if 0
char           *
                memmove(char *os1, const char *s2, int len)
{
	register char  *s1 = os1;

	if (s1 < s2)
	{
		char           *endp = s1 + len;

		while (s1 < endp)
		{
			*s1++ = *s2++;
		}
	}
	else if (s1 > s2)
	{
		s1 += len;
		s2 += len;
		while (s1 > os1)
		{
			*--s1 = *--s2;
		}
	}
	return s1;
}

#endif				/* 0 */

Variable       *
                FindVar(const char *varname, int namelen)
{
	int             varnum = 0;

	while (varnum < varcount)
	{
		if (NameCmp(variables[varnum].name, varname, namelen) == 0)
			return &variables[varnum];
		++varnum;
	}
	return (Variable *) 0;
}

int
                NameCmp(const char *name, const char *against, int alen)
{
	while (alen-- > 0)
	{
		if (*name != *against)
			return *name - *against;
		++name;
		++against;
	}
	return *name;
}

void
                PrintVar(Variable * varp)
{
	printf("%s = ", varp->name);
	switch (varp->type)
	{
	case VT_INT:
		printf("%d\n", varp->value.intval);
		break;
	case VT_IDENT:
		printf("'%s'\n", varp->value.identval);
		break;
	default:
		printf("Unknown var type\n");
		exit(1);
	}
}

void
                Error(const char *errmsg)
{
	printf("Error: %s\n", errmsg);
}

void
                ShiftBuf(void)
{
	if (bufmark != (char *) 0)
	{
		int             moveamount = bufmark - inbuf;

		memmove(inbuf, bufmark, inbufend - bufmark);
		inbufend -= moveamount;
		inbufp -= moveamount;
		maxreplptr -= moveamount;
		bufmark = inbuf;
	}
	else
	{
		inbufend = inbuf;
		inbufp = inbuf;
	}
}

void
                FillInbuf(void)
{
	char           *endptr = inbuf + INBUF_SIZE;

	if (inbufend < endptr)
	{
		int             readamount;
		int             readcount;

		if ((readamount = endptr - inbufend) <= 0)
		{
			PatError("Match too large");
			exit(1);
		}
		inbufend += (readcount = read(infile, inbufend, readamount));
		if (readcount <= 0)
			ateof = 1;
	}
}

void
                ReadMore(void)
{
	ShiftBuf();
	FillInbuf();
}

#define AdvChr() {if (++inbufp>=inbufend) ReadMore();}

int
                ReadInt(void)
{
	char           *start = inbufp;

	if (*inbufp == '+' || *inbufp == '-')
		AdvChr();
	while (isdigit(*inbufp))
		AdvChr();
	return atoi(start);
}

void
                ReadIdent(char *buffer)
{
	register char  *ibp = inbufp;

	while (isalpha(*ibp) || *ibp == '_' || *ibp == '$' || isdigit(*ibp))
	{
		*buffer = *ibp;
		++buffer;
		AdvChr();
		ibp = inbufp;
	}
	*buffer = '\0';
}

void
                PatError(const char *errmsg)
{
	printf("Error in pattern: %s\n", errmsg);
	if (curpat != 0)
	{
		printf("Original:\n");
		{
			char           *p = bufmark + matchstart;

			while (p < inbufp)
			{
				putc(*p, stdout);
				++p;
			}
		}
		printf("Expression:\n");
		printf("%s\n", curpat + 2);
		printf("Replacement:\n");
		printf("%s\n", ((char **) curpat)[0]);
	}
	{
		int             vn = 0;

		while (vn < varcount)
		{
			PrintVar(&variables[vn]);
			++vn;
		}
	}
	exit(1);
}

const char     *
                PatMatchLine(const char *pattern)
{
	const char     *patp = pattern;
	register int    patchr = *patp;

	for (;;)
	{
		if (patchr == ' ')
		{
			if (*inbufp != ' ')
				return 0;
			while (*inbufp == ' ')
				AdvChr();
		}
		else if (patchr == '\n')
		{
			while (*inbufp == ' ')
				AdvChr();
			if (*inbufp != '\n')
				return 0;
			AdvChr();
			++patp;
			break;
		}
		else if (patchr == '{')
		{
			++patp;
			if (!isalpha(*patp) && *patp != '_')
			{
				PatError("invalid identifier");
			}
			{
				const char     *start = patp;

				++patp;
				while (isalpha(*patp) || *patp == '_' || isdigit(*patp))
					++patp;
				if (varcount >= MAX_VAR)
				{
					PatError("too many variables");
				}
				{
					char            readtoend = 0;
					Variable       *variable = &variables[varcount];

					{
						char           *varname = variable->name;
						int             namelen = patp - start;

						if (namelen >= sizeof(variables[0].name))
							PatError("identifier too long");
						memcpy(varname, start, namelen);
						varname[namelen] = '\0';
					}
					if (*patp == '*')
					{
						++patp;
						readtoend = 1;
					}
					if (readtoend)
					{
						char           *ip = variable->value.identval;

						variable->type = VT_IDENT;
						while (*inbufp != '\n')
						{
							if (ip - variable->value.identval >=
							    sizeof(variable->value.identval))
							{
								PatError("Rest of line too long");
								exit(1);
							}
							*ip++ = *inbufp;
							AdvChr();
						}
						*ip = '\0';
					}
					else if (*inbufp == '+' || *inbufp == '-' || isdigit(*inbufp))
					{
						variable->type = VT_INT;
						variable->value.intval = ReadInt();
					}
					else if (isalpha(*inbufp) || *inbufp == '_' || *inbufp == ' ')
					{
						variable->type = VT_IDENT;
						ReadIdent(variable->value.identval);
					}
					++varcount;
					if (*patp == ':')
					{
						char            expr[80];
						char           *exprptr = expr;

						++patp;
						while (*patp != '}' && *patp != '\n')
						{
							*exprptr++ = *patp++;
						}
						*exprptr = '\0';
						if (!ExprTrue(expr))
							return 0;
					}
					if (*patp != '}')
					{
						PatError("'}' expected");
					}
				}
			}
		}
		else
		{
			if (patchr == '\0')
				break;
			if (patchr != *inbufp)
				return 0;
			AdvChr();
		}
		patchr = *++patp;
	}
	return patp;
}

const char     *
                AnyMatch(const char *pattern)
{
	int             oldinbufp = inbufp - bufmark;
	int             oldvarcount = varcount;

	while (*inbufp == '*')
	{
		const char     *patp;

		++inbufp;
		patp = PatMatchLine(pattern);
		if (patp)
		{
			inbufp = bufmark + oldinbufp;
			return patp;
		}
		varcount = oldvarcount;
		while (*inbufp != '\n')
			AdvChr();
		AdvChr();
	}
	inbufp = bufmark + oldinbufp;
	return 0;
}

int             PatMatch3(PatTree * patp);

char            exprstack[10][80];
int             exprstacktop = 0;

int
                MatchVar5(PatTree * patp, char *exprptr)
{
	while (patp)
	{
		if (patp->chr == '}')
		{
			*exprptr = '\0';
			if (ExprTrue(exprstack[exprstacktop]))
			{
				++exprstacktop;
				if (PatMatch3(patp->match))
					return 1;
				--exprstacktop;
			}
		}
		else if (patp->chr == '\n')
			PatError("no '}' on line");
		else
		{
			*exprptr = patp->chr;
			if (MatchVar5(patp->match, exprptr + 1))
				return 1;
		}
		patp = patp->notmatch;
	}
	return 0;
}

int
                MatchVar4(PatTree * patp)
{
	if (patp->chr == '}')
	{
		if (PatMatch3(patp->match))
			return 1;
	}
	else if (patp->chr == ':')
	{
		if (MatchVar5(patp->match, exprstack[exprstacktop]))
			return 1;
	}
	else
	{
		PatError("Expecting '}' or ':'");
	}
	return 0;
}

int
                MatchVar3(PatTree * patp, int readtoend)
{
	Variable       *variable = &variables[varcount];

	if (FindVar(variable->name, strlen(variable->name)))
	{
		printf("var '%s' already used\n", variables[varcount].name);
		exit(1);
	}
	if (readtoend)
	{
		char           *ip = variable->value.identval;

		variable->type = VT_IDENT;
		while (*inbufp != '\n')
		{
			if (ip - variable->value.identval >=
			    sizeof(variable->value.identval))
			{
				PatError("Rest of line too long");
				exit(1);
			}
			*ip++ = *inbufp;
			AdvChr();
		}
		*ip = '\0';
	}
	else if (*inbufp == '+' || *inbufp == '-' || isdigit(*inbufp))
	{
		variable->type = VT_INT;
		variable->value.intval = ReadInt();
	}
	else if (isalpha(*inbufp) || *inbufp == '_' || *inbufp == ' ')
	{
		variable->type = VT_IDENT;
		ReadIdent(variable->value.identval);
	}
	else
	{
		variable->type = VT_IDENT;
		variable->value.identval[0] = *inbufp;
		AdvChr();
		variable->value.identval[1] = '\0';
	}
	++varcount;
	if (MatchVar4(patp))
		return 1;
	--varcount;
	return 0;
}

int
                MatchVar2(PatTree * patp, char *varp)
{
	int             oldinbufp = inbufp - bufmark;

	while (patp)
	{
		if (isalpha(patp->chr) || patp->chr == '_' || isdigit(patp->chr))
		{
			*varp = patp->chr;
			if (MatchVar2(patp->match, varp + 1))
				return 1;
		}
		else
		{
			if (varcount >= MAX_VAR)
			{
				PatError("too many variables");
			}
			*varp = '\0';
			if (patp->chr == '*')
			{
				if (MatchVar3(patp->match, 1))
					return 1;
			}
			else if (MatchVar3(patp, 0))
				return 1;
		}
		inbufp = bufmark + oldinbufp;
		patp = patp->notmatch;
	}
	return 0;
}

int
                MatchVar(PatTree * patp)
{
	while (patp)
	{
		if (!isalpha(patp->chr) && patp->chr != '_')
		{
			PatError("invalid identifier");
		}
		else
		{
			if (MatchVar2(patp, variables[varcount].name))
				return 1;
		}
		patp = patp->notmatch;
	}
	return 0;
}


int
                CheckExpr(PatTree * patp)
{
	curpat = (char *) patp;
	if (ExprTrue((char *) patp + 2))
	{
		matchpat = (char *) patp;
		return 1;
	}
	return 0;
}

/* Normal matching */
int
                PatMatchChr(PatTree * patp)
{
	if (patp->chr == '\0')
	{
		if (CheckExpr(patp->match))
			return 1;
	}
	else if (patp->chr == ' ')
	{
		if (*inbufp == ' ')
		{
			int             oldinbufp = inbufp - bufmark;

			while (*inbufp == ' ')
				AdvChr();
			if (PatMatch3(patp->match))
				return 1;
			inbufp = bufmark + oldinbufp;
		}
	}
	else if (patp->chr == '\n')
	{
		int             oldinbufp = inbufp - bufmark;

		while (*inbufp == ' ')
			AdvChr();
		if (*inbufp == '\n')
		{
			AdvChr();
			if (PatMatch9(patp->match))
				return 1;
		}
		inbufp = bufmark + oldinbufp;
	}
	else if (patp->chr == '{')
	{
		int             oldinbufp = inbufp - bufmark;

		if (MatchVar(patp->match))
			return 1;
		inbufp = bufmark + oldinbufp;
	}
	else
	{
		if (*inbufp == patp->chr)
		{
			int             oldinbufp = inbufp - bufmark;

			AdvChr();
			if (PatMatch3(patp->match))
				return 1;
			inbufp = bufmark + oldinbufp;
		}
	}
	return 0;
}

/* Normal matching */
int
                PatMatch3(PatTree * patp)
{
	while (patp)
	{
		if (PatMatchChr(patp))
			return 1;
		patp = patp->notmatch;
	}
	return 0;
}


/* Beginning of line. Normal matching */
int
                PatMatch9(PatTree * patp)
{
	while (patp)
	{
		if (patp->chr == '&' || patp->chr == '!')
		{
			if (*inbufp == '*')
			{
				if (PatMatch17(patp))
					return 1;
			}
			else
			{
				if (patp->chr == '!')
				{
					if (PatMatch7(patp->match))
						return 1;
				}
			}
		}
		else if (patp->chr == '\0')
		{
			if (CheckExpr(patp->match))
				return 1;
		}
		else
		{
			int             oldinbufp = inbufp - bufmark;

			while (*inbufp == '*')
				AdvLine();
			if (PatMatchChr(patp))
				return 1;
			inbufp = bufmark + oldinbufp;
		}
		patp = patp->notmatch;
	}
	return 0;
}

char            linestack[10][32];
int             linestacktop = 0;

/* Beginning of line. Middle of tag patterns. Tag on line. */
int
                PatMatch5(PatTree * patp)
{
	while (patp)
	{
		if (patp->chr == '&' || patp->chr == '!')
		{
			if (PatMatch17(patp))
				return 1;
		}
		else
		{
			if (matchstart >= 0 && patp->chr == '\0')
			{
				if (CheckExpr(patp->match))
					return 1;
			}
			else
			{
				int             oldinbufp = inbufp - bufmark;

				while (*inbufp == '*')
				{
					AdvLine();
				}
				if (PatMatch13(patp))
					return 1;
				inbufp = bufmark + oldinbufp;
			}
		}
		patp = patp->notmatch;
	}
	return 0;
}

/* Building tag patterns. Tag on line. */
int
                PatMatch6(PatTree * patp, char *linep)
{
	while (patp)
	{
		*linep = patp->chr;
		if (*linep == '\n')
		{
			char           *line;

			*(linep + 1) = '\0';
			line = linestack[linestacktop];
			if (line[0] == '&')
			{
				int             oldvarcount = varcount;

				if (AnyMatch(line + 1))
				{
					++linestacktop;
					if (PatMatch5(patp->match))
						return 1;
					--linestacktop;
				}
				varcount = oldvarcount;
			}
			else if (line[0] == '!')
			{
				int             oldvarcount = varcount;

				if (!AnyMatch(line + 1))
				{
					++linestacktop;
					if (PatMatch5(patp->match))
						return 1;
					--linestacktop;
				}
				varcount = oldvarcount;
			}
			else
			{
				printf("bad line start");
				exit(1);
			}
		}
		else
		{
			if (PatMatch6(patp->match, linep + 1))
				return 1;
		}
		patp = patp->notmatch;
	}
	return 0;
}

/* Matching '&' or '!' pattern. Tag on line */
int
                PatMatch17(PatTree * patp)
{
	char           *linep = linestack[linestacktop];

	*linep = patp->chr;
	return PatMatch6(patp->match, linep + 1);
}

int             PatMatch16(PatTree * patp);

/* skipping '!' pattern. Pre tag. No tag on line */
int
                PatMatch7(PatTree * patp)
{
	while (patp)
	{
		if (patp->chr == '\n')
		{
			if (PatMatch16(patp->match))
				return 1;
		}
		else
		{
			if (PatMatch7(patp->match))
				return 1;
		}
		patp = patp->notmatch;
	}
	return 0;
}

/* Beginning of line. First line of non-tag patterns. */
int
                PatMatch13(PatTree * patp)
{
	matchstart = inbufp - bufmark;
	if (PatMatchChr(patp))
		return 1;
	matchstart = -1;
	return 0;
}

/* Beginning of line. Middle of tag patterns. No tag on line. */
int
                PatMatch16(PatTree * patp)
{
	while (patp)
	{
		if (patp->chr == '!')
		{
			if (PatMatch7(patp->match))
				return 1;
		}
		else if (patp->chr == '&')
		{
		}
		else
		{
			if (matchstart >= 0 && patp->chr == '\0')
			{
				if (CheckExpr(patp->match))
					return 1;
			}
			{
				int             oldinbufp = inbufp - bufmark;

				AdvLine();
				if (PatMatch13(patp))
					return 1;
				inbufp = bufmark + oldinbufp;
			}
		}
		patp = patp->notmatch;
	}
	return 0;
}

/* Beginning of line. Beginning of tag patterns. */
int
                PatMatch4(PatTree * patp)
{
	if (*inbufp == '*')
	{
		while (patp)
		{
			if (patp->chr == '&' || patp->chr == '!')
			{
				if (PatMatch17(patp))
					return 1;
			}
			else
			{
				if (PatMatch13(patp))
					return 1;
			}
			patp = patp->notmatch;
		}
	}
	else
	{
		while (patp)
		{
			if (patp->chr == '&')
			{
			}
			else if (patp->chr == '!')
			{
				if (PatMatch7(patp->match))
					return 1;
			}
			else
			{
				if (PatMatch13(patp))
					return 1;
			}
			patp = patp->notmatch;
		}
	}
	return 0;
}

void
                Replace(char *bufptr, const char *replstr)
{
	int             repstrlen = strlen(replstr);
	int             markedlen = inbufp - bufptr;
	int             movedist = repstrlen - markedlen;

	if (debug)
	{
		printf("Replacing:\n");
		{
			char           *p = bufptr;

			while (p < inbufp)
			{
				putc(*p, stdout);
				++p;
			}
		}
		printf("with:\n");
		printf("%s", replstr);
	}
	if (inbufend + movedist > inbuf + sizeof inbuf)
	{
		ShiftBuf();
		if (inbufend + movedist > inbuf + sizeof inbuf)
		{
			PatError("replacement overflow");
		}
	}
	memmove(bufptr + repstrlen, inbufp, inbufend - inbufp + 1);
	memcpy(bufptr, replstr, repstrlen);
	inbufend += movedist;
	inbufp += movedist;
	if (maxreplptr > bufptr)
	{
		maxreplptr += movedist;
	}
	if (inbufp > maxreplptr)
		maxreplptr = inbufp;
}

void
                EvalExpr(const char **exprptr, char **strptr)
{
	const char     *expr = *exprptr;
	char           *str = *strptr;
	Variable        value;

	++expr;
	expr = ReadExpr(expr, &value);
	switch (value.type)
	{
	case VT_INT:
		sprintf(str, "%d", value.value.intval);
		str += strlen(str);
		break;
	case VT_IDENT:
		strcpy(str, value.value.identval);
		str += strlen(str);
		break;
	default:
		PatError("Unknown type");
		exit(1);
	}
	if (*(expr - 1) != '}')
		PatError("Mismatched {}'s");
	*exprptr = expr;
	*strptr = str;
}

int
                ExprTrue(const char *expr)
{
	Variable        value;

	ReadExpr(expr, &value);
	if (value.type != VT_INT)
	{
		PatError("Invalid type for expression");
		exit(1);
	}
	return value.value.intval;
}

int
                MakRepl(const char *replpat, char *replstr)
{
	char            cleartags = 0;
	const char     *pp = replpat;
	char           *sp = replstr;

	while (*pp != '\0')
	{
		char           *start = sp;

		/* loop until the line contains characters */
		for (;;)
		{
			if (*pp == '{')
			{
				EvalExpr(&pp, &sp);
				if (sp > start)
					break;
			}
			else if (*pp == '#')
			{
				cleartags = 1;
				while (*pp++ != '\n')
				{
				}
			}
			/* skip empty lines */
			else if (*pp == '\n')
				++pp;
			else
				break;
		}
		/* just in case nothing was produced */
		if (*pp == '\0')
			break;
		/* write out the rest of the line */
		for (;;)
		{
			if (*pp == '{')
				EvalExpr(&pp, &sp);
			else
			{
				if ((*sp++ = *pp++) == '\n')
					break;
			}
		}
	}
	*sp = '\0';
	return cleartags;
}

void
                RemoveTags(void)
{
	char           *bufp = bufmark;

	while (bufp < maxreplptr)
	{
		if (*bufp == '*')
			break;
		while (*bufp++ != '\n')
		{
		}
	}
	{
		char           *writep = bufp;

		while (bufp < maxreplptr)
		{
			if (*bufp == '*')
			{
				while (*bufp++ != '\n')
				{
				}
			}
			else
			{
				while ((*writep++ = *bufp++) != '\n')
				{
				}
			}
		}
		{
			int             size = inbufend - maxreplptr;
			int             movedist = maxreplptr - writep;

			memmove(writep, maxreplptr, size);
			inbufend -= movedist;
			inbufp -= movedist;
			maxreplptr -= movedist;
		}
	}
}

char            cleartags;

void
                ReplPat(char *bufptr, const char *pat)
{
	static char     replbuf[512];

	cleartags = MakRepl(pat, replbuf);
	Replace(bufptr, replbuf);
	if (cleartags)
		RemoveTags();
}

PatTree        *
                PatTreeAlloc(void)
{
	if (patheapptr >= patheap + PATHEAPSIZE)
	{
		Error("Pattern tree overflow");
	}
	return patheapptr++;
}

#if 0
void
                PrintPatBranch(PatTree * ptp, int depth)
{
	int             indent = 0;

	if (ptp == (PatTree *) 0)
	{
		printf("\n");
		return;
	}
	for (;;)
	{
		if (ptp == (PatTree *) 0)
			break;
		if (indent)
		{
			int             n = depth;

			while (n > 0)
			{
				putc(' ', stdout);
				--n;
			}
		}
		if (ptp->chr == '\0')
		{
			putc('$', stdout);
			putc('\n', stdout);
		}
		else
		{
			if (ptp->chr == '\n')
				putc('@', stdout);
			else
			{
				putc(ptp->chr, stdout);
			}
			PrintPatBranch(ptp->match, depth + 1);
		}
		indent = 1;
		ptp = ptp->notmatch;
	}
}

void
                PrintPatTree(void)
{
	PrintPatBranch(pattree, 0);
}

#endif				/* 0 */

PatTree       **
                AddPatTree(PatTree ** ptp, int c)
{
	for (;;)
	{
		if (*ptp == 0)
		{
			*ptp = PatTreeAlloc();
			(*ptp)->chr = c;
			(*ptp)->match = 0;
			(*ptp)->notmatch = 0;
			ptp = &(*ptp)->match;
			break;
		}
		else
		{
			if ((*ptp)->chr == c && c != '\0')
			{
				ptp = &(*ptp)->match;
				break;
			}
			else
				ptp = &(*ptp)->notmatch;
		}
	}
#if 0
	printf("--------\n");
	PrintPatTree();
#endif				/* 0 */
	return ptp;
}

void
                ReadPatFile(const char *filename)
{
	char           *patbufp = patbuf;

	inbufend = inbufp = inbuf;
	infile = open(filename, O_RDONLY);
	if (infile == -1)
	{
		Error("Can't open patterns file");
		exit(1);
	}
	FillInbuf();
	while (!ateof)
	{
		PatTree       **ptp = &pattree;
		char           *start = patbufp;
		char            lastchar = 0;

		/* Skip comments */
		while (*inbufp == '*')
		{
			while (*inbufp != '\n')
				AdvChr();
			AdvChr();
		}
		while (!ateof && (*inbufp != '\n' || lastchar != '\n'))
		{
#if 0
			if (patbufp >= patbuf + sizeof(patbuf))
			{
				Error("Pattern buffer overflow");
				exit(1);
			}
			*patbufp++ = lastchar = *inbufp;
#else
			lastchar = *inbufp;
			ptp = AddPatTree(ptp, lastchar);
#endif
			AdvChr();
		}
		AdvChr();
		ptp = AddPatTree(ptp, '\0');
		*ptp = (PatTree *) start;
		patbufp = start + 2;
		if (patbufp >= patbuf + sizeof(patbuf))
		{
			Error("Pattern buffer overflow");
			exit(1);
		}
		while (!ateof && *inbufp != '\n')
		{
			if (patbufp >= patbuf + sizeof(patbuf))
			{
				Error("Pattern buffer overflow");
				exit(1);
			}
			*patbufp++ = *inbufp;
			AdvChr();
		}
		AdvChr();
		if (patbufp >= patbuf + sizeof(patbuf))
		{
			Error("Pattern buffer overflow");
			exit(1);
		}
		*patbufp++ = '\0';
		((char **) start)[0] = patbufp;
		lastchar = '\n';
		while (!ateof && (*inbufp != '\n' || lastchar != '\n'))
		{
			if (patbufp >= patbuf + sizeof(patbuf))
			{
				Error("Pattern buffer overflow");
				exit(1);
			}
			*patbufp++ = lastchar = *inbufp;
			AdvChr();
		}
		if (patbufp >= patbuf + sizeof(patbuf))
		{
			Error("Pattern buffer overflow");
			exit(1);
		}
		*patbufp++ = '\0';
		AdvChr();
	}
	close(infile);
}

int
                ReplMatch2(void)
{
	int             bufptr = inbufp - bufmark;

	varcount = 0;
	linestacktop = 0;
	exprstacktop = 0;
	matchstart = -1;
	if (PatMatch4(pattree))
	{
		ReplPat(bufmark + matchstart, *((char **) matchpat));
		inbufp = bufmark + bufptr;
		ateof = 0;
		return 1;
	}
	return 0;
}

void
                OutputLine(void)
{
	for (;;)
	{
		char            c = *bufmark++;

		putc(c, stdout);
		if (c == '\n')
			break;
	}
}

void
                Usage(void)
{
	fprintf(stderr, "Usage: copt2 [-d] <pattern file>\n");
	exit(1);
}

void
                AdvLine(void)
{
	while (*inbufp != '\n')
		AdvChr();
	AdvChr();
}

void
                BackLine(void)
{
	--inbufp;
	if (inbufp < bufmark)
		Error("BackLine: at bof");
	for (;;)
	{
		--inbufp;
		if (inbufp < bufmark || *inbufp == '\n')
			break;
	}
	++inbufp;
}

/* Back up to the previous search point */
void
                Backup(void)
{
	if (inbufp > bufmark)
	{
		BackLine();
		if (inbufp > bufmark && *inbufp == '*')
		{
			for (;;)
			{
				BackLine();
				if (*inbufp != '*')
				{
					AdvLine();
					break;
				}
				if (inbufp <= bufmark)
					break;
			}
		}
	}
}

int
                main(int argc, char **argv)
{
	int             argnum = 1;

	if (argnum >= argc)
		Usage();
	if (strcmp(argv[argnum], "-d") == 0)
	{
		debug = 1;
		++argnum;
	}
	if (argc - argnum < 1)
		Usage();
	ReadPatFile(argv[argnum]);
	/* PrintPatTree(); */
	inbufend = inbufp = inbuf;
	infile = 0;
	ateof = 0;
	FillInbuf();
	bufmark = inbufp;
	while (!ateof)
	{
		int             lineoff = inbufp - bufmark;

		if (!ReplMatch2())
		{
			/*
			 * if no matches
			 * occur, advance one
			 * line
			 */
			/*
			 * A tag group is
			 * considered to be
			 * one line
			 */
			if (*inbufp == '*')
			{
				while (*inbufp == '*')
					AdvLine();
			}
			else
			{
				AdvLine();
			}
			/*
			 * If we are over our
			 * limit in back
			 * characters, output
			 * one line worth
			 */
			if (inbufp - bufmark > MAXBACK)
			{
				while (inbufp - bufmark > MAXBACK)
				{
					if (*bufmark == '*')
					{
						while (*bufmark == '*')
							OutputLine();
					}
					else
					{
						OutputLine();
					}
				}
			}
		}
		else
		{
			if (debug)
			{
				printf("-- Buffer --\n");
				{
					char           *p = bufmark;

					while (p < maxreplptr)
					{
						putc(*p, stdout);
						++p;
					}
				}
				printf("------------\n");
			}
			if (cleartags)
			{
				inbufp = bufmark;
				ateof = 0;
			}
			else
			{
				inbufp = bufmark + lineoff;
				ateof = 0;
				if (inbufp > bufmark)
				{
					Backup();
					if (inbufp > bufmark)
						Backup();
				}
			}
		}
	}
	/* Output anything remaining */
	while (bufmark < inbufp)
		putc(*bufmark++, stdout);
	return 0;
}
