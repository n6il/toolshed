#include "op.h"
/*
#ifndef unix
#define NULL ((char*)0)
#endif
*/

chain           ltable[128];
DIRECT label   *lfree;

void labelinit()
{
	register int    i;
	register chain *p;

	for (p = ltable, i = 0; i < 128; i++)
	{
		p->succ = p->pred = p;
		p++;
	}
}

label          *inslabel(s)
	char           *s;
{
	register chain *l;
	register label *p;

	p = newlabel();

	l = &ltable[hash(s)];
	strcpy(p->lname, s);

	/* put at back of list */
	p->succ = l;
	p->pred = l->pred;
	((instruction *) (l->pred))->succ = p;
	l->pred = p;

	return p;
}

label          *findlabel(s)
	char           *s;
{
	label          *p;
	chain          *l;

	l = &ltable[hash(s)];


	for (p = l->succ; p != l;)
	{
		if (strcmp(s, p->lname) == 0)
			return p;
		p = p->succ;
	}
	return NULL;
}

int hash(s)
	register char  *s;
{
	int             i = 0;

	while (*s)
		i += *s++;

	return i & 127;
}

label          *newlabel()
{
	register label *p;

	if ((p = lfree) != NULL)
	{
		lfree = p->succ;
		p->dest = p->rlist = p->nextl = p->lflags = NULL;
	}
	else
		p = grab(sizeof(*p));

	return p;
}

void freelabel(p)
	register label *p;
{
	if (p->dest == NULL && p->rlist == NULL)
	{
		/* remove from ltable */
		((instruction *) (p->pred))->succ = p->succ;
		((instruction *) (p->succ))->pred = p->pred;
		/* add to free list */
		p->succ = lfree;
		lfree = p;
	}
}

char           *newbra(lblp)
	char           *lblp;
{
	static int      labnum = 0;

	/* sprintf(lblp,"_$%d",++labnum); */
	sprintf(lblp, "_O%d", ++labnum);
	return lblp;
}
