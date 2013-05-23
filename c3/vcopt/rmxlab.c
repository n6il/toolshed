#include <stdio.h>
#include <ctype.h>
#include <lowio.h>

#define DIRECT direct;

#define INBUF_SIZE 4096

static DIRECT int infile = 0;
static char     inbuf[INBUF_SIZE];
static DIRECT int ateof = 0;
static DIRECT char *inbufp = inbuf;
static DIRECT char *inbufend = inbuf;
static DIRECT char *bufmark = 0;

typedef struct PatTree_s PatTree;

struct PatTree_s
{
	char            chr;
	PatTree        *match;
	PatTree        *notmatch;
};

#define PATHEAPSIZE 4000

static PatTree  patheap[PATHEAPSIZE];
static PatTree *pattree = 0;
static PatTree *patheapptr = patheap;

#define AdvChr() {if (++inbufp>=inbufend) ReadMore();}

static void
                ShiftBuf(void)
{
	if (bufmark != (char *) 0)
	{
		int             moveamount = bufmark - inbuf;

		memmove(inbuf, bufmark, inbufend - bufmark);
		inbufend -= moveamount;
		inbufp -= moveamount;
		bufmark = inbuf;
	}
	else
	{
		inbufend = inbuf;
		inbufp = inbuf;
	}
}

static void
                FillInbuf(void)
{
	char           *endptr = inbuf + INBUF_SIZE;

	if (inbufend < endptr)
	{
		int             readamount = endptr - inbufend;
		int             readcount;

		inbufend += (readcount = read(infile, inbufend, readamount));
		if (readcount <= 0)
			ateof = 1;
	}
}

static void
                ReadMore(void)
{
	ShiftBuf();
	FillInbuf();
}

static int
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

static void
                Error(char *msg)
{
	printf("rmxlab: %s\n", msg);
	exit(1);
}

static PatTree *
                PatTreeAlloc(void)
{
	if (patheapptr >= patheap + PATHEAPSIZE)
	{
		Error("Pattern tree overflow");
	}
	return patheapptr++;
}

static void
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
			printf(":%d\n", *(char *) ptp->match);
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

static void
                PrintPatTree(void)
{
	PrintPatBranch(pattree, 0);
}

static PatTree **
                AddPatTree(PatTree ** ptp, int c)
{
	for (;;)
	{
		if (*ptp == 0)
		{
#if 0
			*ptp = PatTreeAlloc();
			(*ptp)->chr = c;
			(*ptp)->match = 0;
			(*ptp)->notmatch = 0;
			ptp = &(*ptp)->match;
			break;
#else
			register PatTree *pt = *ptp = PatTreeAlloc();

			pt->chr = c;
			pt->match = 0;
			pt->notmatch = 0;
			ptp = &pt->match;
			break;
#endif				/* 0 */
		}
		else
		{
			if ((*ptp)->chr == c)
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

static char    *
                FindLabel(char *label, int length)
{
	PatTree       **ptpp = &pattree;
	char           *p = label;

	while (p - label < length)
	{
		ptpp = AddPatTree(ptpp, *p++);
	}
	ptpp = AddPatTree(ptpp, '\0');
	return (char *) ptpp;
}

static void
                AddLabel(char *label, int length, int used)
{
	{
		char           *p = FindLabel(label, length);

		if (used > *p)
			*p = used;
		return;
	}
}

static void
                ReadStmt(void)
{
	/* skip space before opcode */
	while (*inbufp == ' ')
		AdvChr();
	if (*inbufp == '\n')
		return;
	/* skip opcode */
	bufmark = inbufp;
	while (*inbufp != ' ' && *inbufp != '\n')
		AdvChr();
	if (NameCmp("nam", bufmark, inbufp - bufmark) == 0)
		return;
	if (NameCmp("psect", bufmark, inbufp - bufmark) == 0)
		return;
	if (NameCmp("fcc", bufmark, inbufp - bufmark) == 0)
		return;
	/* skip space after opcode */
	while (*inbufp == ' ')
		AdvChr();
	/* search for label in operand */
	while (*inbufp != '\n' && *inbufp != ',')
	{
		if (isalpha(*inbufp) || *inbufp == '_')
		{
			bufmark = inbufp;
			while (isalpha(*inbufp) || isdigit(*inbufp) || *inbufp == '_' || *inbufp == '$')
				AdvChr();
			/*
			 * All labels are more than 1 char
			 * long
			 */
			if (inbufp - bufmark > 1)
				AddLabel(bufmark, inbufp - bufmark, 1);
			bufmark = 0;
		}
		else
			AdvChr();
	}
}

static char    *infilename = 0;

static void     OpenFile(void)
{
	infile = open(infilename, READ);
	inbufp = inbufend = inbuf;
	ateof = 0;
	bufmark = (char *) 0;
	FillInbuf();
}

static void     BuildTree(void)
{
	OpenFile();
	while (!ateof)
	{
		if (*inbufp == '*')
		{
			while (*inbufp != '\n')
				AdvChr();
			AdvChr();
		}
		else
		{
			if (*inbufp != ' ' && *inbufp != '\n')
			{
				bufmark = inbufp;
				while (isalpha(*inbufp) || isdigit(*inbufp) || *inbufp == '_' ||
				       *inbufp == '$')
					AdvChr();
				if (*inbufp != ':')
				{
					AddLabel(bufmark, inbufp - bufmark, 0);
					bufmark = 0;
				}
				else
				{
					bufmark = 0;
					AdvChr();
				}
			}
			ReadStmt();
			while (*inbufp != '\n')
				AdvChr();
			AdvChr();
		}
	}
	close(infile);
}

static void     PrintName(char *name, int namelen)
{
	char           *p = name;

	while (p - name < namelen)
	{
		putc(*p++, stdout);
	}
}

static void     OutRest(void)
{
	while (*inbufp != '\n')
	{
		putc(*inbufp, stdout);
		AdvChr();
	}
	putc(*inbufp, stdout);
	AdvChr();
}

static void     FilterFile(void)
{
	OpenFile();
	while (!ateof)
	{
		if (*inbufp == '*')
		{
			OutRest();
		}
		else if (*inbufp == '\n')
		{
			AdvChr();
		}
		else
		{
			int             bol = 1;

			if (*inbufp != ' ')
			{
				int             removeit = 0;

				bufmark = inbufp;
				while (*inbufp != ' ' && *inbufp != '\n' && *inbufp != ':')
				{
					AdvChr();
				}
				if (*inbufp != ':')
				{
					char           *p = FindLabel(bufmark, inbufp - bufmark);

					if (*p == 0)
						removeit = 1;
				}
				if (!removeit)
				{
					PrintName(bufmark, inbufp - bufmark);
					bol = 0;
				}
				bufmark = (char *) 0;
			}
			if (bol)
			{
				while (*inbufp == ' ')
					AdvChr();
				if (*inbufp != '\n')
				{
					putc(' ', stdout);
					OutRest();
				}
				else
					AdvChr();
			}
			else
				OutRest();
		}
	}
	close(infile);
}

int             main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage: rmxlab <filename>");
		exit(1);
	}
	infilename = argv[1];
	BuildTree();
	FilterFile();
	return 0;
}
