#include "op.h"
#include <string.h>

DIRECT int      inscount;
int             insmax = MAXINS;
DIRECT chain    ilist;
DIRECT int      lbf,
                lbdone;
static DIRECT instruction *freeins;
static DIRECT int insused;

/* branch mnemonics */
char           *bratab[] = {
	"ra", "sr",
	"eq", "ne",
	"lt", "ge",
	"le", "gt",
	"lo", "hs",
	"ls", "hi",
	"pl", "mi",
	"cc", "cs",
	"vc", "vs",
};

#define NUMBRA ((sizeof bratab) / (sizeof (char *)))

char          **srchmnem(mnemp, tblbeg, tblend)
	char           *mnemp;
	char          **tblbeg,
	              **tblend;
{
	register char **nxtmnem;

	for (nxtmnem = tblbeg; nxtmnem < tblend; ++nxtmnem)
		if (strcmp(mnemp, *nxtmnem) == 0)
		{
			return nxtmnem;
		}
	return NULL;
}

void insinit()
{
	ilist.succ = ilist.pred = &ilist;
	inscount = 0;
	insused = 0;
	freeins = NULL;
}

instruction    *insins(ins, mnem, args, l)
	instruction    *ins;
	char           *mnem,
	               *args;
	label         **l;
{
	register instruction *i;
	register label *lp;
	char           *mnemp,
	               *argp,
	              **mnemtp;
	int             argl;

	i = newins();

#ifdef DEBUG
	debug("insins: ");
	for (lp = (l ? *l : NULL); lp; lp = lp->nextl)
		debug("%s%s", lp->lname, lp->nextl ? "/" : "");
	debug(" %s %s\n", mnem, args);
#endif

	if ((i->llist = (l ? *l : NULL)) != 0)
	{
		for (lp = *l; lp; lp = lp->nextl)
			lp->dest = i;
		*l = NULL;
	}
	strcpy(i->mnem, mnem);
	if (*(argp = args))
		while (*++argp);
	argl = argp - args;

	i->succ = ins;
	i->pred = ins->pred;
#if 0
	((instruction *) ins->pred)->succ = i;
#else
	ins->pred->succ = i;
#endif
	ins->pred = i;
	++inscount;

	switch (*(mnemp = i->mnem))
	{
	case 'l':
		if (*++mnemp != 'b')
			break;
	case 'b':
		if ((mnemtp = srchmnem(mnemp + 1, bratab, bratab + NUMBRA)) != 0)
		{
			i->itype |= (BRANCH | LONG | (mnemtp - bratab));
			strcpy(i->mnem, *mnemtp);
			parse(LABEL, args, args, &argl);
			lp = findlabel(args);
			if (mnemtp == bratab)
			{
				if (lp && lp->dest)
					movlab(i, lp->dest);
				if (i->llist == NULL
				    && i->pred != &ilist
#if 0
				    && (((instruction *) i->pred)->itype & CODEBRK))
#else
				    && (i->pred->itype & CODEBRK))
#endif
				{
					remins(i);
					++opsdone;
					return NULL;
				}
				i->itype |= CODEBRK;
			}
			++lbf;
			if (lp == NULL)
				lp = inslabel(args);
			i->itype |= DESTPTR;
			insref(i, i->alt.lab = lp);
		}
		break;
	case 'p':
		if (strcmp(mnemp, "puls") == 0
		    && strcmp(args + argl - 3, ",pc") == 0)
			i->itype |= CODEBRK;
		break;
	case 'r':
		if (strcmp(mnemp, "rts") == 0)
			i->itype |= CODEBRK;
		break;
	}
	if (!(i->itype & BRANCH))
		strcpy(i->alt.args = newarg(argl), args);
#ifdef DEBUG
	debug("inscount = %d, inserted: ", inscount);
	prtins(i);
#endif
	return i;
}

instruction    *newins()
{
	register instruction *i;

	if ((i = freeins) == NULL)
	{
		if (++insused > insmax + 3)
			error("run out of instructions");
		i = (instruction *)grab(sizeof(*i));
	}
	else
	{
		freeins = i->succ;
		i->itype = 0;
		i->alt.args = 0;
	}
	return i;
}

void insref(ip, lp)
	instruction    *ip;
	label          *lp;
{
	register chain *rlp;

	rlp = newchain();
	rlp->pred = ip;
	rlp->succ = lp->rlist;
	lp->rlist = rlp;
}

void movlab(i1, i2)
	instruction    *i1,
	               *i2;
{
	register label *l,
	               *lastl;

	if (i1 != i2 && (l = i1->llist))
	{
		do
		{
			lastl = l;
			l->dest = i2;
		} while ((l = l->nextl) != 0);
		lastl->nextl = i2->llist;
		i2->llist = i1->llist;
		i1->llist = NULL;
	}
}

void rminst()
{
	register instruction *i;
	label          *l;

	if ((i = ilist.succ) == &ilist)
		error("removing too many instructions");

	if ((i->itype & LONGBRA) == LONGBRA)
		fix(i, FORWARD);
	if ((l = i->llist) != 0)
		fix(i, BACK);
	while (l)
	{
		fprintf(out, "%s", l->lname);
		if (l->lflags & GLOBAL)
			putc(':', out);
		if ((l = l->nextl) != 0)
			putc('\n', out);
	}

	putc(' ', out);
	if (i->itype & BRANCH)
	{
		fprintf(out, "%sb%s %s", (i->itype & LONG ? "l" : ""), i->mnem,
		    (i->itype & DESTPTR ? i->alt.lab->lname : i->alt.args));
	}
	else
	{
		fprintf(out, "%s", i->mnem);
		if (i->alt.args[0])
			fprintf(out, " %s", i->alt.args);
	}
	putc('\n', out);
	remins(i);
}

void fix(i, direction)
	instruction    *i;
{
	register instruction *ip = i;
	instruction    *dp;
	label          *lp;
	int             c;

	if (direction == FORWARD)
	{
		lp = (ip->itype & DESTPTR ? ip->alt.lab : findlabel(ip->alt.args));
		if (lp && (dp = lp->dest))
		{
			for (c = BRALIM;
			   c && (ip = (instruction *) (ip->succ)) != (instruction *)&ilist;
			     --c)
			{
				if (dp == ip)
				{
#ifdef DEBUG
					debug("fix: ");
					prtins(i);
#endif
					ip->itype |= BLOCKDUP;
					i->itype &= ~LONG;
					++lbdone;
					return;
				}
			}
		}
	}
	else
	{
		for (c = BRALIM;
		     c && ((ip = ip->succ) != NULL) != &ilist;
		     --c)
		{
			if ((ip->itype & LONGBRA) == LONGBRA)
			{
				lp = (ip->itype & DESTPTR) ? ip->alt.lab
					: findlabel(ip->alt.args);
				if (lp && i == lp->dest)
				{
#ifdef DEBUG
					debug("fix: ");
					prtins(ip);
#endif
					ip->itype &= ~LONG;
					++lbdone;
					return;
				}
			}
		}
	}
}

void remins(i)
	register instruction *i;
{
	register label *l;

#ifdef DEBUG
	debug("remins: ");
	prtins(i);
#endif
	if (i->itype & BRANCH)
	{
		remref(i, i->alt.lab);
	}
	else
		freearg(i->alt.args);
	for (l = i->llist; l; l = l->nextl)
	{			/* return labels */
		l->dest = NULL;
		freelabel(l);
	}
	/* remove instruction from the list */
	((instruction *) (i->pred))->succ = i->succ;
	((instruction *) (i->succ))->pred = i->pred;

	i->succ = freeins;
	freeins = i;

	--inscount;
}

void remref(ip, lp)
	instruction    *ip;
	label          *lp;
{
	chain          *rlp,
	               *lastrlp;

	if (lp)
	{
		lastrlp = (chain *) & lp->rlist;
		for (; (rlp = lastrlp->succ) != NULL; lastrlp = rlp)
			if (rlp->pred == ip)
			{
				lastrlp->succ = rlp->succ;
				freelabel(lp);
				freechain(rlp);
				break;
			}
	}
}

static DIRECT chain *_cfree = NULL;

chain          *newchain()
{
	register chain *cp;

	if ((cp = _cfree) != NULL)
		_cfree = cp->succ;
	else
		cp = (chain *)grab(sizeof(chain));
	cp->succ = cp->pred = NULL;
	return cp;
}

void freechain(p)
	register chain *p;
{
	p->succ = _cfree;
	_cfree = p;
}

static DIRECT chain *safree = NULL,
               *lafree = NULL;

char           *newarg(siz)
	int             siz;
{
	register chain *ap;

	if (siz < 16)
		if ((ap = safree) != NULL)
			safree = ap->succ;
		else
			ap = (chain *)grab(16);
	else if ((ap = lafree) != NULL)
		lafree = ap->succ;
	else
		ap = (chain *)grab(ARGSIZE + 1);
	ap->succ = NULL;
	return (char *)ap;
}

void freearg(p)
	chain          *p;
{
	register char  *sp;

	if ((sp = (char *)p) != NULL)
	{
		while (*sp)
			++sp;
		if (sp - (char *) p < 16)
		{
			p->succ = safree;
			safree = p;
		}
		else
		{
			p->succ = lafree;
			lafree = p;
		}
	}
}
