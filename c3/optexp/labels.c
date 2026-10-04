#include "op.h"
/*
#ifndef unix
#define NULL ((char*)0)
#endif
*/

chain           ltable[128];
DIRECT label   *lfree;

void labelinit(void)
{
	register int    i;
	register chain *p;

	for (p = ltable, i = 0; i < 128; i++)
	{
		p->succ = p->pred = p;
		p++;
	}
}

label          *inslabel(char *s)
{
	register chain *l;
	register label *p;

	p = newlabel();

	l = &ltable[hash(s)];
	strcpy(p->lname, s);

	/* put at back of list */
	p->succ = (int *)l;
	p->pred = l->pred;
	((instruction *) (l->pred))->succ = (instruction *)p;
	l->pred = p;

	return p;
}

label          *findlabel(char *s)
{
	label          *p;
	chain          *l;

	l = &ltable[hash(s)];


	for (p = l->succ; (chain *)p != l;)
	{
		if (strcmp(s, p->lname) == 0)
			return p;
		p = (label *)p->succ;
	}
	return NULL;
}

int hash(register char *s)
{
	int             i = 0;

	while (*s)
		i += *s++;

	return i & 127;
}

label          *newlabel(void)
{
	register label *p;

	if ((p = lfree) != NULL)
	{
		lfree = (label *)p->succ;
		p->dest = NULL;
		p->rlist = NULL;
		p->nextl = NULL;
		p->lflags = 0;
	}
	else
		p = (label *)grab(sizeof(*p));

	return p;
}

void freelabel(register label *p)
{
	if (p->dest == NULL && p->rlist == NULL)
	{
		/* remove from ltable */
		((instruction *) (p->pred))->succ = (instruction *)p->succ;
		((instruction *) (p->succ))->pred = (instruction *)p->pred;
		/* add to free list */
		p->succ = (int *)lfree;
		lfree = p;
	}
}

char           *newbra(char *lblp)
{
	static int      labnum = 0;

	/* sprintf(lblp,"_$%d",++labnum); */
	sprintf(lblp, "_O%d", ++labnum);
	return lblp;
}
