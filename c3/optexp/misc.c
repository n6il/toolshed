#include "op.h"

#define GRABSIZE 512

char           *grab(unsigned n)
{
	char *             ptr;

	if ((ptr = sbrk(n)) == (char *)-1)
		error("memory overflow");
	return ptr;
}

void error(char *s1, char *s2, char *s3, char *s4)
{
	fprintf(stderr, "C optimiser error: ");
	fprintf(stderr, s1, s2, s3, s4);
	putc('\n', stderr);
	exit(1);
}

#ifdef DEBUG
void debug(char *fmt, char *s1, char *s2, char *s3, char *s4)
{
	if (dbflag)
		fprintf(stderr, fmt, s1, s2, s3, s4);
}

void prtins(instruction *i)
{
	register label *l;

	if (dbflag)
	{
		fprintf(stderr, "@%04x ", (unsigned int)i);
		for (l = i->llist; l; l = l->nextl)
			fprintf(stderr, "%s%s", l->lname, l->nextl ? "/" : "");

		if (i->alt.lab != NULL && i->alt.args != NULL)
		{
			fprintf(stderr, " %s %s\n", i->mnem,
			     (i->itype & BRANCH ? i->alt.lab->lname : i->alt.args));
		}
	}
}

#endif
