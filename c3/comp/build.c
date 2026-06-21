/*
     Modification history for build.c:
          22-May-83      Fixed sizeof problem with undefined name.
          25-May-83      Made better use of register variables.
          19-Sep-83      Fixed assops so their associativity is the
                         same as that of assignment itself.
          26-Nov-83      Look for undefined struct tags in casts.
          10-Apr-84      Conversion for UNIX.
*/

#include "cj.h"

static elsize();

expnode *parsexp(int priority)
{
    register expnode *rhs,*lhs,*temp;
    register int op,priop,lno,rprec;
    char *errpnt;

    if ((lhs = primary()) == NULL) return NULL;

    while (isop() && priority <= symval) {
        op = sym;
        rprec = priop = symval;
        lno = symline;
        errpnt = symptr;
        getsym();

        switch(op) {
            default:
                if (op < ASSPLUS || op > ASSXOR) rprec = priop+1;
            case QUERY :
            case ASSIGN :
                ;
        }

        if (rhs = parsexp(rprec)) {
            if (op == QUERY) {
                if (need(COLON)) {
                    reltree(rhs);
                    goto exit;
                } else {
                    errpnt = symptr;
                    lno = symline;
                    if (temp = parsexp(3)) {
                        rhs = newnode(COLON,rhs,temp,3,lno,errpnt);
                    } else {
                        error("third expression missing");
                        reltree(rhs);
                        goto exit;
                    }
                }
            }
            lhs = newnode(op,lhs,rhs,priop,lno,errpnt);
        } else error("operand expected");
    }

exit:
    return lhs;
}

expnode *primary()
{
    register expnode *nodep,*prim;
    expnode *temp;
    register int op,lno,size;
    register char *errpnt;

    nodep = NULL;

    switch (sym) {
        case STRING:
        case NAME:
        case LCONST:
#ifdef  DOFLOATS
        case FCONST:
#endif
        case CONST:
            nodep = newnode(sym,0,0,symval,symline,symptr);
            getsym();
            break;
        case LPAREN:
            getsym();
            if (istype()) {
                nodep = getcast();
                need(RPAREN);
                if (nodep->left = primary()) ;
                else {
                    release(nodep);
                    nodep=NULL;
                }
                break;
            } else if ((nodep = parsexp(0)) == NULL) {
noexp:
                experr();
                nodep = newnode(CONST,0,0,0,symline,symptr);
            }
            need(RPAREN);
            break;
        case NOT:
        case NEG:
        case COMPL:
        case STAR:
        case DECBEF:
        case INCBEF:
        case AMPER:
            op=sym;
            lno=symline;
            errpnt=symptr;
            getsym();
            if (prim = primary())
                nodep = newnode(op,prim,0,14,lno,errpnt);
            else error("primary expected");
            break;
        case SIZEOF:
            lno=symline;
            errpnt=symptr;
            getsym();

            if(sym==LPAREN){
                getsym();
                if (istype()) prim = getcast();
                else if ((prim = parsexp(0)) == NULL) goto noexp;
                need(RPAREN);
            } else prim = primary();
            temp = prim;
            nodep = newnode(CONST,0,0,elsize(&temp),lno,errpnt);
            reltree(temp);
            break;
    }
    if (nodep == NULL) return NULL;

    for ( ; ; ) {
        switch(sym) {
            case LPAREN:
                errpnt=symptr;
                lno=symline;
                getsym();
                nodep=newnode(CALL,nodep,explist(),15,lno,errpnt);
                need(RPAREN);
                continue;

            case LBRACK:
                getsym();
                if ((prim = parsexp(2)) == NULL) {
                    experr();
                    prim = newnode(CONST,0,0,0,symline,symptr);
                }
                nodep=newnode(PLUS,nodep,prim,12,symline,symptr);
                nodep=newnode(STAR,nodep,0,15,symline,symptr);
                need(RBRACK);
                continue;

            case DOT:
            case ARROW:
                op=sym;
                lno=symline;
                errpnt=symptr;
                ++mosflg;
                getsym();
                --mosflg;
                if (sym!=NAME) {
                    identerr();
                    break;
                }
                prim = newnode(sym,0,0,symval,symline,symptr);
                nodep = newnode(op,nodep,prim,15,lno,errpnt);
                getsym();
                continue;
        }
        break;
    }

    switch (sym){
        case INCBEF:
            sym=INCAFT;
            goto dblop;
        case DECBEF:
            sym=DECAFT;
dblop:      nodep = newnode(sym,nodep,0,14,symline,symptr);
            getsym();
    }

    return nodep;
}

expnode *explist()
{
    register expnode *list = NULL, *ptr;

    for ( ; ; ) {
        if (sym == RPAREN) break;
        if (ptr = parsexp(2)){
            ptr = newnode(ARG,ptr,list,0,ptr->lno,ptr->pnt);
            list = ptr;
        }
        if (sym != COMMA) break;
        getsym();
    }
    return list;
}

constexp(int p)
{
    register expnode *ptr;
    register int v;

    if ((ptr = optim(parsexp(p))) && ptr->op == CONST) v = ptr->val.num;
    else {
        v = 0;
        error("constant required");
    }
    if (ptr) reltree(ptr);
    return v;
}

isop()
{
    switch (sym) {
        case AMPER:
            sym=AND;
            symval=8;
            return 1;
        case STAR:
            sym=TIMES;
            symval=13;
            return 1;
        case NEG:
            sym=MINUS;
            symval=12;
        case COMMA:
        case ASSIGN:
        case QUERY:
            return 1;
        case COLON:
            return 0;
        default:
            return ((sym >= DBLAND && sym <= GT)
                            || (sym >= ASSPLUS && sym <= ASSXOR));
    }
}


expnode *getcast()
{
    expnode *ptr;
    dimnode *dptr;          /* settype writes a dimnode * here, not an expnode * */
    elem *dummy;            /* settype writes an elem * here; ignored by a cast */
    long size;              /* settype writes a long here: an int would be
                               overrun by 2 bytes on the 6809 (4 on an LP64 host),
                               smashing this frame on every cast that is parsed */
    int lno,type;
    register char *errpnt;

    lno = symline;
    errpnt = symptr;
    type = settype(&size,&dptr,&dummy);
    if (type == USTRUCT) {
        error("undefined struct");
        ptr = NULL;
    } else {
        type = declarator(&ptr,&dptr,type);
        clear(&arglist);
        if (ptr) error("name in a cast");
        ptr = newnode(CAST,0,0,0,lno,errpnt);
        ptr->type = type;
        sizeup(ptr,dptr,size);
    }
    return ptr;
}


experr()
{
    error("expression missing");
}

static elsize(expnode **prim)
{
    register expnode *p;

    switch((p = *prim)->op) {
        default:
            p = *prim = optim(p);
            goto usual;
        case NAME:
            p->type = p->val.sp->type;
            chkdecl(p);
            p = (expnode *) p->val.sp;
        case CAST:
usual:      return getsize(p->type,(int)p->size,p->dimptr);
            /* p->size is `long` but getsize's size parameter is a K&R int (as
               sizeup's call already assumes); without the cast the long is read
               from its HIGH half on the big-endian target, so sizeof(struct)
               folded to 0 there (24 on the little-endian host) - the same width
               trap as val.lval / argword.  Truncating to int first gives the
               correct low 16 bits on both. */
        case DOT:
        case ARROW:
            return elsize(&p->right);
    }
}


/* parser error-recovery helpers (moved from misc.c at the split) */
need(int key)
{
    static char ptr[] = "x expected";
    register int i;

    if(sym==key) {
        getsym();
        return 0;
    }
    for (i = 0; i < 128; ++i) if (chartab[i] == key) break;
    *ptr=i;
    error(ptr);
/*
    for ( ; ; ) {
        if(sym==key)
            getsym();
        else switch(sym) {
            case SEMICOL: case RBRACE: case EOF:
                break;
            default:
                getsym();
                continue;
        }
        break;
    }
*/
    return 1;
}

junk()
{
    while (sym != SEMICOL && sym != RBRACE && sym != EOF) getsym();
}
