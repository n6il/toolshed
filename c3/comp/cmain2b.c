/*
 * cmain2b.c - c3comp2 IL-read / symbol-synthesis layer (split from cmain2.c).
 *
 * The back end's expression-tree rebuilder (rdexpr) and the per-function
 * synthesised-symbol machinery (mksym/relsyms), plus the string, local-static
 * and goto-label number maps.  Split out of cmain2.c purely so each translation
 * unit is small enough for c3comp1 (pass 1) to compile within OS-9 Level 2's
 * ~21 KB per-process heap when the toolchain self-hosts - cmain2.c as one file
 * was the only compiler source that overran it.  The driver and statement loop
 * stay in cmain2.c; output is byte-identical (pure code motion).
 *
 * The per-function symbol cache and its free lists are SHARED with cmain2.c:
 * they are defined there and allocated in c2init, so here they are extern.
 */

#include "cj.h"
#include "il.h"

extern char *grab(int);
extern int arity(int);          /* op arity 0/1/2, shared in treeutil.c */

#define ESTACK 64               /* operand-stack depth while rebuilding a tree */
#define MAXDIMS 8               /* deepest array/pointer dimension chain */
#define MAXSYMS 256             /* max distinct symbols interned per function */
#define MAXSTR 192              /* string temp-id -> label map */
#define MAXSTAT 8               /* local statics per function */
#define MAXLABS 12              /* goto labels per function */

/* Shared per-function symbol cache + free lists (defined in cmain2.c, allocated
 * in c2init).  mksym/relsyms here populate and reclaim them; cmain2.c's
 * flushsyms does the block-scoped reclaim, so both halves touch this state. */
extern symnode **symcache;      /* [MAXSYMS] */
extern int nsyms;
extern symnode *symfree;        /* reclaimed synthesised symnodes (link: snext) */
extern dimnode *dimfree;        /* reclaimed dim-chain nodes      (link: dptr)  */
extern int *strmap;             /* [MAXSTR] string temp-id -> pass-2 label */
extern char strname[];          /* strings temp-file name (mktemp'd in main) */
extern char *labnames;          /* [MAXLABS][IL_NAMEMAX] flat, heap (c2init) */
extern int nstat, nlabs;        /* reset per function by cmain2.c's IL_FUNC */

static char emptypnt[] = "";    /* lno/pnt placeholder for synthesised nodes */

/* per-function map of a local static's pass-1 temp-id to the pass-2 label */
static int stattmp[MAXSTAT];
static int statlab[MAXSTAT];

/* per-function goto-label numbers (labnames, shared, holds the names) */
static int labnums[MAXLABS];

int statlabel(int tid)     /* look up a ref's label (def always precedes it) */
{
    register int i;

    for (i = 0; i < nstat; i++) if (stattmp[i] == tid) return statlab[i];
    return tid;             /* IL_LBSS/IL_LBL precede the refs; fall back if not */
}

int statdef(int tid)       /* allocate (once) the pass-2 label for a local static */
{
    register int i, N;

    for (i = 0; i < nstat; i++) if (stattmp[i] == tid) return statlab[i];
    N = getlabel(0);
    if (nstat >= MAXSTAT) fatal("IL: too many local statics in one function");
    stattmp[nstat] = tid; statlab[nstat] = N; nstat++;
    return N;
}

/* handle an IL_STR record: assign a real label, map the temp-id, and write the
 * table entry (label + body lines) to strfile for dumpstrings to emit.  A
 * string is defined where it was lexed, which lands inside the expression
 * record stream, so this is called from both rdexpr and the statement loop. */
void dostr()
{
    int tid = ilrint();
    int N = getlabel(0);
    register char *ln;

    if (tid < 0 || tid >= MAXSTR) fatal("IL: too many string constants");
    strmap[tid] = N;
    if (strfile == NULL && (strfile = fopen(strname,"w+")) == (FILE *) 0)
        fatal("can't open strings file");
    fprintf(strfile, "%c%d", UNIQUE, N);    /* the table label */
    while ((ln = ilrline()) != (char *) 0) {
        if (*ln >= '0' && *ln <= '9') break;   /* IL_ENDRAW marker */
        fputs(ln, strfile);
        putc('\n', strfile);
    }
}

/* synthesise (or reuse) a symnode for a NAME leaf from its carried fields.
 * tagtype/tagsize are the struct tag's type+size for a USTRUCT name (chtype
 * expects size to hold a tag-symnode pointer); ignored otherwise. */
symnode *mksym(int type, int size, int storage, int offset, char *name, int *dims, int ndims, int tagtype, int tagsize)
{
    register int i;
    register symnode *s;
    register dimnode *d;

    for (i = 0; i < nsyms; i++) {
        s = symcache[i];
        if (s->storage == storage && s->offset == offset && s->type == type
                && strcmp(s->sname,name) == 0)
            return s;          /* same variable -> share the symnode + dimptr */
        /* type is part of the key: sibling blocks reuse a stack slot, so two
           differently-typed locals can share name+offset+storage (e.g. fold's
           long *p and double *p) and must NOT be interned together */
    }
    if (s = symfree) symfree = s->snext;        /* reuse a reclaimed symnode */
    else s = (symnode *) grab(SYMSIZE);
    s->type = type;
    if (btype(type) == USTRUCT) {       /* rebuild the tag symnode chtype reads */
        register symnode *tag;
        if (tag = symfree) symfree = tag->snext;
        else tag = (symnode *) grab(SYMSIZE);
        tag->type = tagtype;
        tag->size = tagsize;
        s->size = (long) tag;
    } else
        s->size = size;
    s->storage = storage;
    s->offset = offset;
    s->dimptr = NULL;
    for (i = ndims - 1; i >= 0; i--) {      /* rebuild the dimension chain */
        if (d = dimfree) dimfree = d->dptr;     /* reuse a reclaimed dimnode */
        else d = (dimnode *) grab(sizeof(dimnode));
        d->dim = dims[i];
        d->dptr = s->dimptr;
        s->dimptr = d;
    }
    s->blklev = 0;
    s->x.labflg = 0;
    s->args = NULL;     /* pass 2 has no prototype info from the IL yet, so a
			   reconstructed function has no arglist -> chtype's call
			   coercion stays inert here (must be NULL, not garbage) */
    strcpy(s->sname,name);
    if (nsyms >= MAXSYMS) fatal("IL: too many symbols in one function");
    symcache[nsyms++] = s;      /* MUST cache to intern repeat refs: a silent
                                   drop here defeats the register tracker and
                                   miscompiles (don't revert to a capped store) */
    return s;
}

/* Reclaim a finished function's synthesised symnodes (and their dim chains and
 * any USTRUCT tag symnodes) onto free lists for the next function to reuse.
 * Called at each IL_FUNC; by then the previous function is fully generated, so
 * its symnodes are dead.  (The block-scoped companion, flushsyms, lives in
 * cmain2.c with the control-flow frame stacks it walks.) */
void relsyms()
{
    register int i;
    register symnode *s;
    register dimnode *d, *dn;

    for (i = 0; i < nsyms; i++) {
        s = symcache[i];
        for (d = s->dimptr; d; d = dn) {        /* its dimension chain */
            dn = d->dptr;
            d->dptr = dimfree;
            dimfree = d;
        }
        if (btype(s->type) == USTRUCT && s->size) {     /* tag symnode off ->size */
            register symnode *tag = (symnode *) s->size;
            tag->snext = symfree;
            symfree = tag;
        }
        s->snext = symfree;
        symfree = s;
    }
    nsyms = 0;
}

int labelnum(char *name)
{
    register int i;

    for (i = 0; i < nlabs; i++)
        if (strcmp(&labnames[i*IL_NAMEMAX], name) == 0) return labnums[i];
    if (nlabs >= MAXLABS) fatal("IL: too many labels");
    strcpy(&labnames[nlabs*IL_NAMEMAX], name);
    labnums[nlabs] = getlabel(0);
    return labnums[nlabs++];
}

/* rebuild one postfix expression tree, up to IL_ENDE; NULL if empty */
expnode *rdexpr()
{
    expnode *stk[ESTACK];
    register expnode *n;
    int op, depth, ty, sz, sto, off, mod;
    char *nm;

    depth = 0;
    while ((op = ilrrec()) != IL_ENDE) {
        if (op == ILEOF) fatal("IL: EOF inside expression");
        if (op == IL_NULL) {            /* a NULL child (e.g. ARG-chain end) */
            if (depth >= ESTACK) fatal("IL: expression too deep");
            stk[depth++] = NULL;
            continue;
        }
        if (op == IL_STR) {             /* string def lexed mid-expression */
            dostr();
            continue;
        }
        if (freenode) {                 /* reuse a released expnode instead of always */
            register char *q;           /* grab'ing a fresh one: rdexpr never reused  */
            register int z;             /* the pool reltree fills, leaking the heap.  */
            n = freenode; freenode = n->left;
            for (q = (char *)n, z = NODESIZE; z--; ) *q++ = 0;   /* sbrk zeroes a fresh
                                           node; match that so fields rdexpr leaves
                                           unset (val on operators, ...) read as 0. */
        } else n = (expnode *) grab(NODESIZE);
        n->op = op;
        n->left = n->right = NULL;
        n->dimptr = NULL;
        n->modifier = 0;
        n->lno = 0;
        n->pnt = emptypnt;
        n->sux = 0;
        switch (op) {
            case NAME: {
                int dims[MAXDIMS], nd, k, tagty, tagsz;
                int ptypes[MAXCARGS], np;

                ty = ilrint(); sz = ilrint(); sto = ilrint();
                off = ilrint(); mod = ilrint(); nm = ilrname();
                nd = ilrint();
                if (nd > MAXDIMS) fatal("IL: too many dimensions");
                for (k = 0; k < nd; k++) dims[k] = ilrint();
                tagty = tagsz = 0;
                if (btype(ty) == USTRUCT) {     /* struct tag's type + byte size */
                    tagty = ilrint();
                    tagsz = ilrint();
                }
                np = ilrint();                  /* prototype param-type count */
                if (np > MAXCARGS) fatal("IL: too many prototype params");
                for (k = 0; k < np; k++) ptypes[k] = ilrint();
                /* a local static is referenced by its label (offset); remap the
                   pass-1 temp-id to the label pass 2 assigned at its IL_LBSS */
                if (sto == STATIC || sto == STATICD) off = statlabel(off);
                n->type = ty;
                n->size = sz;
                n->modifier = mod;
                n->val.sp = mksym(ty,sz,sto,off,nm,dims,nd,tagty,tagsz);
                n->dimptr = n->val.sp->dimptr;
                /* rebuild the prototype arg-type list (forward snext order) so
                   chtype coerces this call exactly as single-pass does */
                if (np && n->val.sp->args == NULL) {
                    register symnode *prev = NULL, *a;
                    for (k = np - 1; k >= 0; k--) {
                        a = (symnode *) grab(SYMSIZE);
                        a->type = ptypes[k];
                        a->snext = prev;
                        prev = a;
                    }
                    n->val.sp->args = prev;
                }
                break;
            }
            case CONST:
                n->type = ilrint();
                n->val.num = ilrint();
                n->size = INTSIZE;
                break;
            case STRING: {
                int tid = ilrint();

                n->type = POINTER | CHAR;
                n->size = 1;
                n->val.num = (tid >= 0 && tid < MAXSTR) ? strmap[tid] : 0;
                break;
            }
            case LCONST: {              /* 32-bit value, hi then lo */
                int hi, lo;
                register long *lp;

                n->type = ilrint();
                hi = ilrint();
                lo = ilrint();
                lp = (long *) grab(sizeof(long));
                *lp = ((long) hi << 16) | (lo & 0xffff);
                n->val.lp = lp;
                n->size = LONGSIZE;
                break;
            }
#ifdef DOFLOATS
            case FCONST: {              /* 8 raw bytes of the host double */
                register char *p;
                register int i;

                n->type = ilrint();
                p = grab(DOUBLESIZE);
                for (i = 0; i < DOUBLESIZE; i++) p[i] = ilrint();
                n->val.dp = (DBLETYPE *) p;
                n->size = DOUBLESIZE;
                break;
            }
#endif
            case CAST: {                /* target type/size/dims, then pop operand */
                int dims[MAXDIMS], nd, k;
                register dimnode *d;

                n->type = ilrint();
                n->size = ilrint();
                nd = ilrint();
                if (nd > MAXDIMS) fatal("IL: too many dimensions");
                for (k = 0; k < nd; k++) dims[k] = ilrint();
                n->dimptr = NULL;
                for (k = nd - 1; k >= 0; k--) {
                    d = (dimnode *) grab(sizeof(dimnode));
                    d->dim = dims[k];
                    d->dptr = n->dimptr;
                    n->dimptr = d;
                }
                if (depth < 2) fatal("IL: operator stack underflow");
                n->right = stk[--depth];    /* NULL placeholder */
                n->left = stk[--depth];     /* the operand */
                break;
            }
            default:
                switch (arity(op)) {
                    case 2:
                        if (depth < 2) fatal("IL: operator stack underflow");
                        n->right = stk[--depth];
                        n->left = stk[--depth];
                        break;
                    case 1:
                        if (depth < 1) fatal("IL: operator stack underflow");
                        n->left = stk[--depth];
                        break;
                    default:
                        fatal("IL: unknown leaf op in expression");
                }
                n->type = INT;          /* optim recomputes operator types */
                n->size = INTSIZE;
        }
        if (depth >= ESTACK) fatal("IL: expression too deep");
        stk[depth++] = n;
    }
    if (depth == 0) return NULL;
    if (depth != 1) fatal("IL: malformed expression (stack not 1)");
    return stk[0];
}
