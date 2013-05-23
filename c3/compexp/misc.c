/*
	 Modification history for misc.c:
		  25-May-83      Make better use of register variables.
		  01-Sep-83      Write errors to stderr.
		  17-Apr-84 LAC  Conversion for UNIX.
*/

/* miscellaneous routines for the c compiler */

#include "cj.h"

/* to push down an outer block declaration
 */
void pushdown(symnode *sptr)
{
	register symnode *nptr;
	register char *sp, *ep;

	if (nptr = freedown)
		freedown = nptr->snext;
	else
		nptr = (symnode *) grab(DOWNSIZE);

	move(sptr, nptr, DOWNSIZE);
	sptr->downptr = nptr;
	sptr->type = sptr->size = sptr->dimptr =
	sptr->offset = sptr->storage = sptr->x.labflg =
	sptr->blklev = sptr->snext = 0;
}

/* to recover outer block declaration
 */
void pullup(symnode *sptr)
{
	register symnode	*nptr;

	nptr = sptr->downptr;
	move(nptr, sptr, DOWNSIZE);

	nptr->snext = freedown;
	freedown = nptr;
}

/* byte move by count */
void move(char *p1, char *p2, int count)
{
	while (--count >= 0)
		*p2++ = *p1++;
}

void pfile(void)
{
	eprintf("%s : ", filename, NULL);
}

void fatal(char *errstr)
{
	error(errstr);
	fflush(stderr);	/* because 'tidy()' uses '_exit()' i.e. no flush */
	tidy();			/* get rid of temp files and exit */
}

void multidef(void)
{
	error("multiple definition");
}

void error(char *s)
{
	doerr(symptr - line, s, symline);
}

void comperr(expnode *node, char *errstr)
{
	char	newstr[50];

	strcpy(newstr, "compiler error - ");
	strcat(newstr, errstr);
	terror(node, newstr);
}

void terror(expnode *node, char *errstr)
{
	doerr(node->pnt - line, errstr, node->lno);
}

void doerr(int n, char *errstr, int lno)
{
	char	*lscan;

	pfile();
	eprintf("line %d  ", lno, NULL);
	eprintf("****  %s  ****\n", errstr, NULL);
	if (lno == lineno) {
		eputs(lscan = line);
		goto dopoint;
	} else if (lno == lineno - 1) {
		eputs(lscan = lastline);
dopoint:
		for ( ; n > 0; --n) {
			eputchar(*lscan == '\t' ? '\t' : ' ');
			lscan++;
		}
		eputs("^");
	}
	if (++errcount > 30) {
		fflush(stderr);
		eputs("too many errors - ABORT") ;
		tidy();
	}
}

void eprintf(char *s1, char *s2, char *s3)
{
	fprintf(stderr, s1, s2, s3);
}

void eputs(char *s)
{
	fputs(s, stderr);
	eputchar('\n');
}

void eputchar(char c)
{
	putc(c, stderr);
}

void reltree(expnode *tree)
{
	if (tree) {
#ifdef DEBUG
		printf("reltree: %04x\n", &tree);
		pnode(tree);
		fflush(stdout);
#endif
		reltree(tree->left);
		reltree(tree->right);
		release(tree);
	}
}

void release(expnode *node)
{
	if (node) {
		node->left = freenode;
		freenode = node;
	}
}

void nodecopy(char *n1, char *n2)
{
	move(n1, n2, NODESIZE);
}

int istype(void)
{
	if (sym == KEYWORD) {
		switch (symval) {
		case INT:
		case CHAR:
		case UNSIGN:
		case SHORT:
		case LONG:
		case STRUCT:
		case UNION:
		case DOUBLE:
		case FLOAT:
		    return 1;
		}
	} else if (sym == NAME && ((symnode *)symval)->storage == TYPEDEF)
		return 1;
	return 0;
}

int issclass(void)
{
	if (sym == KEYWORD) {
		switch (symval) {
		case EXTERN:
		case AUTO:
		case TYPEDEF:
		case REG:
		case STATIC:
		case DIRECT:
			return 1;
		}
	}
	return 0;
}

int decref(int t)
{
	return ((t >> 2) & (~BASICT)) + btype(t);
}

int incref(int t)
{
	return ((t & (~BASICT)) << 2 ) + POINTER + btype(t);
}

int isbin(int op)
{
	return (op >= UMOD && op <= UGT);
}

dimnode *dimwalk(dimnode *dptr)
{
	return dptr ? dptr->dptr : 0;
}

int need(int key)
{
	static char		ptr[] = "x expected";
	register int	i;

	if (sym == key) {
	    getsym();
	    return 0;
	}
	for (i = 0; i < 128; ++i) {
		if (chartab[i] == key)
			break;
	}
	*ptr = i;
	error(ptr);
	return 1;
}

void junk(void)
{
	while (sym != SEMICOL && sym != RBRACE && sym != EOF)
		getsym();
}

#ifdef PTREE
void prtree(expnode *node)
{
	puts("\naddress op        value     type        size sux left   right");
	ptree(node);
}

void ptree(expnode *node)
{
	if (node) {
	    pnode(node);
	    ptree(node->left);
	    ptree(node->right);
	}
}

void pnode(expnode *node)
{
	int	op, val, i;

	op = node->op;
	printf("%04x    ",node);
	if (op == NAME)
	    printf("%-10.8s %04x  ", node->val.sp->sname, node->modifier);
	else
		printf("%-10s%5d  ", kw[op], node->val.num);
	val = node->type;
	for (i = 14; i >= 4 ; i -= 2) {
	    switch ((val >> i)  & 3) {
	    case 1:
	    	putchar('P');
	    	break;
	    case 2:
	    	putchar('A');
	    	break;
	    case 3:
	    	putchar('F');
	    	break;
	    case 0:
	    	putchar(' ');
	    }
	}
	printf(" %-8s%4d %2d  %04x   %04x\n",
		kw[val & BASICT], node->size, node->sux, node->left, node->right);
}
#endif
