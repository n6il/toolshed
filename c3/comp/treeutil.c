/*
 * treeutil.c - shared tree / type helpers, compiled into BOTH passes.
 *
 * These primitives are needed by the front end (parser, c3comp1) and the back
 * end (optim type-check + codegen, c3comp2) alike, so they live in their own
 * module rather than in a pass-specific file.  Moved here verbatim when c3comp
 * was split into two passes:
 *      grab, chartab        from lex.c
 *      newnode, constop     from build.c
 *      sizeup, getsize      from declare.c
 */

#include "cj.h"

extern char *sbrk();

/* character-classification table, indexed by character code */
char chartab[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  109,64,105,0,0,84,65,104,45,46,66,80,48,67,69,83,
  107,107,107,107,107,107,107,107,107,107,47,40,93,120,95,100,
  0,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,
  106,106,106,106,106,106,106,106,106,106,106,43,102,44,89,106,
  0,106,106,106,106,106,106,106,106,106,106,106,106,106,106,106,
  106,106,106,106,106,106,106,106,106,106,106,41,88,42,68,0,
};

/* bump allocator over sbrk - all symbols and expression trees live here */
char *grab(int size)
{
    char *oldptr;

    if ((oldptr = sbrk(size)) == (char *) -1) fatal("out of memory");

    if (!lobrk) lobrk = oldptr;
    hibrk = oldptr+size;

    return oldptr;
}

/* allocate and initialise an expression-tree node */
/* value is the node's value: a symnode * (NAME), an int (CONST), or a precedence
   (operators).  MUST be argword, not long: callers pass an int precedence or
   symval, which on the 6809 stack ABI are pushed pointer-width - a `long` here
   would read 2 bytes too many and misalign lno/errpnt.  argword == long on the
   LP64 host (no-op). */
expnode *newnode(int op, expnode *left, expnode *right, argword value, int lno, char *errpnt)
{
    register expnode *node;

    if (freenode) {
        node = freenode;
        freenode = node->left;
    } else node = (expnode *) grab(NODESIZE);
    node->op = op;
    node->left = left;
    node->right = right;
    node->val.sp = (symnode *) value;	/* store at the union's offset 0 (where
					   val.num / val.sp / val.lp are read).  NOT
					   val.lval: on a big-endian target its low
					   half holds the value but num/sp read its
					   HIGH half, so CONST read 0 and NAME read a
					   wild pointer.  As a pointer cast this keeps
					   full width on the LP64 host too, so output
					   is unchanged there. */
    node->lno = lno;
    node->pnt = errpnt;
    node->modifier = 0;

    return node;
}

/* fold a constant operator: evaluates "r OP l" (unary ops use r) */
constop(int op, int r, int l)
{
    switch(op) {
        case PLUS:      return r + l;
        case MINUS:     return r - l;
        case TIMES:     return r * l;
        case DIV:       if (l) return r / l;
                        diverr();
                        return 0;
        case MOD:       if(l) return r % l;
                        diverr();
                        return 0;
        case AND:       return r & l;
        case OR:        return r | l;
        case XOR:       return r ^ l;
        case SHL:       return r << l;
        case SHR:       return r >> l;
        case EQ:        return r == l;
        case NEQ:       return r != l;
        case GT:        return r > l;
        case LT:        return r < l;
        case GEQ:       return r >= l;
        case LEQ:       return r <= l;
        case NEG:       return -r;
        case NOT:       return !r;
        case COMPL:     return ~r;
        case DBLAND:    return r && l;
        case DBLOR:     return r || l;
        case ULEQ:
        case ULT:
        case UGEQ:
        case UGT:
            {
                unsigned lp,rp;
                lp = (unsigned) l;  rp = (unsigned) r;
                switch (op) {
                    case ULEQ:      return rp <= lp;
                    case ULT:       return rp < lp;
                    case UGEQ:      return rp >= lp;
                    default:        return rp > lp;
                }
            }
        default:        error("constant operator");
                        return 0;
    }
}

/* compute and record the size of a symbol's declared type */
sizeup(register symnode *ptr, register dimnode *dimptr, long size)
{
    register int n,temp;

    switch (btype(temp = ptr->type)) {
        case CHAR:      n = 1; break;
        case INT:
        case UNSIGN:    n = INTSIZE; break;
        case LONG:      n = LONGSIZE; break;
#ifdef  DOFLOATS
        case FLOAT:     n = FLOATSIZE; break;
        case DOUBLE:    n = DOUBLESIZE; break;
#endif
        case UNION:
        case STRUCT:    n = size; break;
        case USTRUCT:   n = 0; break;
    }
    ptr->size = n ? n : size;

    return getsize(temp,n,ptr->dimptr = dimptr);
}

getsize(int t, int size, register dimnode *dptr)
{
    int n;

    if (isptr(t) || isftn(t)) return 2;

    if (isary(t)) {
        n = 1;
        do {
            n *= dptr->dim;
            dptr = dptr->dptr;
        } while (isary(t = decref(t)));

        return n * (isptr(t) ? POINTSIZE : size);
    }
    return size;
}

/* arity of an expression-tree op: 0 leaf, 1 unary, 2 binary.  Shared by the
 * IL writer (ilemit, pass 1) and reader (cmain2, pass 2) so both agree on tree
 * shape - NULL children of binary nodes are written as an IL_NULL placeholder. */
arity(int op)
{
    switch (op) {
        case NAME: case CONST: case LCONST: case FCONST: case STRING:
        case STACK: case DREG: case XREG: case YREG: case UREG: case FREG:
            return 0;
        case NEG: case COMPL: case NOT: case AMPER: case STAR:
        case INCBEF: case DECBEF: case INCAFT: case DECAFT:
        case CTOI: case LTOI: case FTOD: case DTOF:
            return 1;
        default:
            return 2;
    }
}
