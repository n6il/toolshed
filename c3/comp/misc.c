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
void pushdown(register symnode *sptr)
{
    register symnode *nptr;
    register char *sp, *ep;

    if (nptr = freedown) freedown = nptr->snext;
    else nptr = (symnode *) grab(DOWNSIZE);

    move(sptr,nptr,DOWNSIZE);
    sptr->downptr = nptr;
	sptr->type = 0;
	sptr->size = 0;
	sptr->dimptr = NULL;
	sptr->offset = 0;
	sptr->storage = 0;
	sptr->x.labflg = 0;
	sptr->blklev = 0;
	sptr->snext = NULL;
}

/* to recover outer block declaration
 */
void pullup(register symnode *sptr)
{
    register symnode *nptr;

    nptr = sptr->downptr;
    move(nptr,sptr,DOWNSIZE);

    nptr->snext = freedown;
    freedown = nptr;
}

/* byte move by count
 */
void move(register char *p1, register char *p2, int count)
{
    while (count--) *p2++ = *p1++;
}

void pfile()
{
    eprintf("%s : ",filename);
}

void fatal(char *errstr)
{
    error(errstr);
    fflush(stderr); /* because 'tidy()' uses '_exit()' i.e. no flush */
    tidy();         /* get rid of temp files and exit */
}

void multidef()
{
    error("multiple definition");
}

void error(char s[])
{
    doerr(symptr-line,s,symline);
}

void comperr(register expnode *node, char *errstr)
{
    char newstr[50];

    strcpy(newstr,"compiler error - ");
    strcat(newstr,errstr);
    terror(node,newstr);
}

void terror(register expnode *node, char *errstr)
{
    doerr(node->pnt - line,errstr,node->lno);
}

void doerr(register int n, char errstr[], int lno)
{
    pfile();
    eprintf("line %d  ",lno);
    eprintf("****  %s  ****\n",errstr);
    if (lno == lineno) {
        eputs(line);
        goto dopoint;
    } else if (lno == lineno - 1) {
        eputs(lastline);
dopoint:
        for ( ; n > 0; --n) eputchar(' ');
        eputs("^");
    }
    if(++errcount>30) {
        fflush(stderr);
        eputs("too many errors - ABORT") ;
        tidy();
    }
}

/* s1 is the format string; s2/s3 are forwarded args: a string (%s) or an int
   (%d).  They MUST be argword, not long: callers pass int/char* (2 bytes on the
   6809) and fprintf's %d/%s read 2 bytes, so a `long` (4 bytes) here misaligns
   the forwarded args on the 16-bit stack ABI (invisible on the LP64 host's
   register ABI). */
void eprintf(char *s1, argword s2, argword s3)
{
    fprintf(stderr,s1,s2,s3);
}

void eputs(char *s)
{
    fputs(s,stderr);
    eputchar('\n');
}

void eputchar(int c)
{
    putc(c,stderr);
}

void reltree(register expnode *tree)
{
    if (tree) {
#ifdef DEBUG
        printf("reltree: %04x\n",&tree);
        pnode(tree);
        fflush(stdout);
#endif
        reltree(tree->left);
        reltree(tree->right);
        release(tree);
    }
}

void release(register expnode *node)
{
    if(node) {
        node->left=freenode;
        freenode=node;
    }
}

void nodecopy(char *n1, char *n2)
{
    move(n1,n2,NODESIZE);
}

int istype()
{
    if (sym == KEYWORD)
        switch (symval) {
            case INT:
            case CHAR:
            case UNSIGN:
            case SHORT:
            case LONG:
            case STRUCT:
            case UNION:
#ifdef  DOFLOATS
            case DOUBLE:
            case FLOAT:
#endif
                return 1;
        }
    else if (sym == NAME) {
        if (((symnode *)symval)->storage == TYPEDEF) return 1;
    }
    return 0;
}

int issclass()
{
    if(sym==KEYWORD) switch(symval) {
        case EXTERN:
        case AUTO:
        case TYPEDEF:
        case REG:
        case STATIC:
        case DIRECT:
            return 1;
    }
    return 0;
}

int decref(int t)
{
    return ((t >> 2) & (~BASICT)) + btype(t) ;
}

int incref(int t)
{
    return ((t & (~BASICT)) << 2 ) + POINTER + btype(t) ;
}

int isbin(register int op)
{
    return (op>=UMOD && op<=UGT);
}

dimnode *dimwalk(register dimnode *dptr)
{
    return dptr ? dptr->dptr : 0;
}


#ifdef PTREE
void prtree(expnode *node)
{
    puts("\naddress op        value     type        size sux left   right");
    ptree(node);
}

void ptree(register expnode *node)
{
    if (node) {
        pnode(node);
        ptree(node->left);
        ptree(node->right);
    }
}

void pnode(register expnode *node)
{
    int op,val,i;

    op=node->op;
    printf("%04x    ",node);
    if (op == NAME)
        printf("%-10.8s %04x  ",node->val.sp->sname,node->modifier);
    else printf("%-10s%5d  ",kw[op],node->val.num);
    val=node->type;
    for (i=14; i>=4 ; i-=2) {
        switch ((val>>i)  & 3) {
            case 1: putchar('P');break;
            case 2: putchar('A'); break;
            case 3: putchar('F'); break;
            case 0: putchar(' ');
        }
    }
    printf(" %-8s%4d %2d  %04x   %04x\n",kw[val & BASICT],node->size,
                                      node->sux, node->left,node->right);
}
#endif
