/*
 * cmain2.c - c3comp2 (back end) driver.
 *
 * Reads the intermediate language emitted by c3comp1 (see il.h) and generates
 * 6809 assembly, reusing the existing back end (optim type-check, tranexp,
 * codgen, local).  This is the consumer half of the two-pass split; it owns no
 * parser and no symbol table - every NAME leaf in the IL carries the type/size/
 * storage/offset it needs, and a throwaway symnode is synthesised per leaf so
 * the unchanged codegen can dereference node->val.sp as before.
 *
 * Expression trees arrive in POSTFIX order, rebuilt here with an operand stack;
 * statements arrive as structured records so this pass owns all label/jump
 * generation.  WALKING SKELETON: currently functions whose bodies are just
 * return statements over int expressions.  Grows from here.
 */

#define MAIN
#include "cj.h"
#include "il.h"

extern char *grab();
extern expnode *treecopy();
extern direct expnode *dregcont, *xregcont;

char *dumname = sdummy.sname;
char strname[] = "cstr.XXXXX";

#define ESTACK 64               /* operand-stack depth while rebuilding a tree */
#define MAXDIMS 8               /* deepest array/pointer dimension chain */

static char emptypnt[] = "";    /* lno/pnt placeholders for synthesised nodes */

extern int arity();             /* op arity 0/1/2, shared in treeutil.c */

/* Per-function cache of synthesised symnodes.  References to the SAME variable
 * must share one symnode: the register-content tracker (cmptrees) compares the
 * symbol pointer (val.num), so distinct symnodes for the same var would defeat
 * it and force redundant reloads.  Reset at each IL_FUNC. */
#define MAXSYMS 256
static symnode *symcache[MAXSYMS];
static int nsyms;

#define MAXSTR 1024             /* string temp-id -> label map (pass 2 owns labels) */
static int strmap[MAXSTR];

/* Per-function map of a local static's pass-1 temp-id to the label pass 2 owns.
 * locstat emits IL_LBSS with the temp-id; the data and every code reference must
 * use the same pass-2 label, so STATIC/STATICD NAME refs are remapped through
 * this (like strings/goto, pass 2 owns the numbering to avoid label collisions). */
#define MAXSTAT 64
static int stattmp[MAXSTAT];
static int statlab[MAXSTAT];
static int nstat;

static statlabel(tid)       /* look up a ref's label (def always precedes it) */
int tid;
{
    register int i;

    for (i = 0; i < nstat; i++) if (stattmp[i] == tid) return statlab[i];
    return tid;             /* IL_LBSS/IL_LBL precede the refs; fall back if not */
}

static statdef(tid)         /* allocate (once) the pass-2 label for a local static */
int tid;
{
    register int i, N;

    for (i = 0; i < nstat; i++) if (stattmp[i] == tid) return statlab[i];
    N = getlabel(0);
    if (nstat < MAXSTAT) { stattmp[nstat] = tid; statlab[nstat] = N; nstat++; }
    return N;
}

/* handle an IL_STR record: assign a real label, map the temp-id, and write the
 * table entry (label + body lines) to strfile for dumpstrings to emit.  A
 * string is defined where it was lexed, which lands inside the expression
 * record stream, so this is called from both rdexpr and the statement loop. */
static dostr()
{
    int tid = ilrint();
    int N = getlabel(0);
    register char *ln;

    if (tid >= 0 && tid < MAXSTR) strmap[tid] = N;
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
static symnode *mksym(type,size,storage,offset,name,dims,ndims,tagtype,tagsize)
int type,size,storage,offset;
char *name;
int *dims;
int ndims,tagtype,tagsize;
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
    s = (symnode *) grab(SYMSIZE);
    s->type = type;
    if (btype(type) == USTRUCT) {       /* rebuild the tag symnode chtype reads */
        register symnode *tag = (symnode *) grab(SYMSIZE);
        tag->type = tagtype;
        tag->size = tagsize;
        s->size = (long) tag;
    } else
        s->size = size;
    s->storage = storage;
    s->offset = offset;
    s->dimptr = NULL;
    for (i = ndims - 1; i >= 0; i--) {      /* rebuild the dimension chain */
        d = (dimnode *) grab(sizeof(dimnode));
        d->dim = dims[i];
        d->dptr = s->dimptr;
        s->dimptr = d;
    }
    s->blklev = 0;
    s->x.labflg = 0;
    strcpy(s->sname,name);
    if (nsyms < MAXSYMS) symcache[nsyms++] = s;
    return s;
}

/* Per-function cache of user goto-label numbers, keyed by name.  Pass 2 owns
 * ALL label numbering, so user labels can never collide with the if/loop
 * labels it allocates; pass 1 emits the label name and we assign the number. */
#define MAXLABS 64
static char labnames[MAXLABS][IL_NAMEMAX];
static int labnums[MAXLABS];
static int nlabs;

static labelnum(name)
char *name;
{
    register int i;

    for (i = 0; i < nlabs; i++)
        if (strcmp(labnames[i], name) == 0) return labnums[i];
    if (nlabs >= MAXLABS) fatal("IL: too many labels");
    strcpy(labnames[nlabs], name);
    labnums[nlabs] = getlabel(0);
    return labnums[nlabs++];
}

/* rebuild one postfix expression tree, up to IL_ENDE; NULL if empty */
static expnode *rdexpr()
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
        n = (expnode *) grab(NODESIZE);
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
                /* a local static is referenced by its label (offset); remap the
                   pass-1 temp-id to the label pass 2 assigned at its IL_LBSS */
                if (sto == STATIC || sto == STATICD) off = statlabel(off);
                n->type = ty;
                n->size = sz;
                n->modifier = mod;
                n->val.sp = mksym(ty,sz,sto,off,nm,dims,nd,tagty,tagsz);
                n->dimptr = n->val.sp->dimptr;
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

/* conditional-jump codegen (stats.c dotest) */
static dotest(ptr,tl,fl,nj)
register expnode *ptr;
labstruc *tl,*fl;
{
    if (ptr) {
        tranbool(ptr,tl,fl,nj);
        reltree(ptr);
    }
}

/* nesting stack for structured if/else: holds each open if's true/false
 * labels (the locals stats.c doif keeps on the C stack).  The IL marker
 * stream (IL_IF/IL_ELSE/IL_ENDIF) drives push/pop; bodies are processed by
 * the normal record loop in between, so nesting falls out for free. */
#define MAXNEST 64
typedef struct { labstruc itl, ifl; int iet; } ifframe;
static ifframe ifstk[MAXNEST];
static int ifsp;

/* nesting stack for while loops; carries the break/continue labels and the
 * test tree (a while tests at the BOTTOM, after the body). */
typedef struct {
    labstruc brk, cnt;
    int tlab;
    expnode *cond;
    labstruc *whsavb, *whsavc;
} whileframe;
static whileframe whstk[MAXNEST];
static int whsp;
static labstruc *breakptr, *contptr;

/* break/continue jump (stats.c modnjmp) */
static modnjmp(labptr)
register labstruc *labptr;
{
    modstk(labptr->labsp);
    gen(JMP, setlabel(labptr), 0);
}

/* nesting stack for for loops: like while plus a separate test label and the
 * saved test/update trees (a for tests at the bottom, updates before the test) */
typedef struct {
    labstruc brk, cnt, tst;
    int slab;
    expnode *testp, *updp;
    labstruc *frsavb, *frsavc;
} forframe;
static forframe forstk[MAXNEST];
static int forsp;

/* switch state (pass 2's own copy; the parser's live in stats.c).  The case
 * list is built as IL_CASE/IL_DEFAULT records arrive in the body; the jump
 * table is emitted at IL_ENDSW. */
static casnode *caseptr, *lastcase, *freecase;
static int deflabel;
typedef struct {
    labstruc brk;
    int tests;
    casnode *savcase, *savlast;
    int savdef;
    labstruc *swsavb;
} switchframe;
static switchframe swstk[MAXNEST];
static int swsp;

/* expression-statement codegen (stats.c emit(): chkdecl, post->pre inc/dec,
 * translate, release).  The argument must already be optim'd. */
static emit2(ptr)
register expnode *ptr;
{
    if (ptr == NULL) return;
    chkdecl(ptr);
    switch (ptr->op) {
        case INCAFT: ptr->op = INCBEF; break;
        case DECAFT: ptr->op = DECBEF; break;
    }
    reltree(tranexp(ptr));
}

/* return-statement codegen (the post-parse half of stats.c doreturn) */
static doret(ptr)
register expnode *ptr;
{
    if (ptr) {
        ptr = optim(ptr);
        chkdecl(ptr);
        chkstrct(ptr);
        cvt(ptr, isptr(ftype) ? INT : ftype);
        switch (ftype) {
            case LONG:
                lload(ptr);
#ifdef DOFLOATS
                goto common;
            case FLOAT:
            case DOUBLE:
                dload(ptr);
common:
#endif
                if (ptr->op != FREG) {
                    gen(LOADIM,UREG,FREG);
                    gen(PUSH,UREG);
#ifdef DOFLOATS
                    switch (ftype) {
                        case FLOAT:
                        case DOUBLE:
                            gen(DBLOP,MOVE,ftype);
                            break;
                        default:
                            gen(LONGOP,MOVE);
                    }
#else
                    gen(LONGOP,MOVE);
#endif
                }
                break;
            default:
                lddexp(ptr);
        }
        reltree(ptr);
    }
    modstk(0);
    gen(RETURN,0,0);
    lastst = RETURN;
}

main()
{
    int op;
    char *nm;
    int flag, paramreg, dp;

    in = stdin;
    code = stdout;

    /* the codegen references sdummy as a placeholder symbol */
    sdummy.type = INT;
    sdummy.size = 2;
    sdummy.dimptr = NULL;
    sdummy.offset = 0;
    sdummy.storage = EXTDEF;
    strcpy(sdummy.sname,"_dummy_");
    mktemp(strname);

    ilrinit();
    while ((op = ilrrec()) != ILEOF) {
        switch (op) {
            case IL_PSECT:                      /* name, dp-size */
                nm = ilrname();
                dp = ilrint();
                fprintf(code," psect %s,0,0,%d,0,0\n nam %s\n",nm,dp,nm);
                break;
            case IL_BSS: {                      /* name, size, scope, area */
                int sz, scope, area;

                nm = ilrname();
                sz = ilrint();
                scope = ilrint();
                area = ilrint();
                vsect(area);
                fprintf(code,"%.8s%c rmb %d\n", nm, scope ? ':' : ' ', sz);
                endsect();
                break;
            }
            case IL_LBSS: {                     /* local static: temp-id, size, area */
                int tid, sz, area, N;

                tid = ilrint();
                sz = ilrint();
                area = ilrint();
                N = statdef(tid);               /* pass 2 owns the static's label */
                vsect(area);
                olbl(N);
                fprintf(code," rmb %d\n", sz);
                endsect();
                break;
            }
            case IL_RAW: {                      /* data asm until IL_ENDRAW, with
                                                   embedded IL_STR/IL_DWSTR records */
                register char *ln;
                int rop;

                while ((ln = ilrline()) != (char *) 0) {
                    if (ln[0] >= '0' && ln[0] <= '9') {  /* an embedded IL record */
                        rop = ilrint();        /* (ilp is at the line start) */
                        if (rop == IL_ENDRAW) break;
                        if (rop == IL_STR) { dostr(); continue; }
                        if (rop == IL_DWSTR) {  /* string pointer -> fdb _label */
                            fprintf(code," fdb %c%d\n", UNIQUE, strmap[ilrint()]);
                            continue;
                        }
                        if (rop == IL_LBL) {    /* local static's label site */
                            olbl(statdef(ilrint()));
                            nl();
                            continue;
                        }
                    } else {                    /* a plain data asm line (or blank) */
                        fputs(ln, code);
                        putc('\n', code);
                    }
                }
                break;
            }
            case IL_STR:                        /* string def at statement level */
                dostr();
                break;
            case IL_FUNC:                       /* name, flag, paramreg, ftype */
                nm = ilrname();
                flag = ilrint();
                paramreg = ilrint();
                ftype = ilrint();
                /* frame level after the prologue: a D-register first arg is
                 * pushed (pshs d), putting the frame base 2 below entry sp */
                callflag = 0;
                sp = stlev = (paramreg == DREG) ? -2 : 0;
                maxpush = (stlev < 0) ? stlev : 0;
                nsyms = 0;              /* fresh symnode cache per function */
                nlabs = 0;              /* fresh goto-label cache per function */
                nstat = 0;              /* fresh local-static label map per function */
                clrconts();
                startfunc(nm,flag,paramreg);
                lastst = 0;
                break;
            case IL_PARMOV: {                   /* register-param spill: reg, offset */
                int rg, of;

                rg = ilrint();
                of = ilrint();
                gen(rg, of);
                break;
            }
            case IL_RET:
                doret(rdexpr());
                break;
            case IL_EXPR:
                emit2(optim(rdexpr()));
                lastst = 0;
                break;
            case IL_IF: {                       /* et flag; then test+ENDE */
                register ifframe *fr;
                register expnode *cond;
                int et;

                et = ilrint();
                cond = optim(rdexpr());
                chkdecl(cond);
                if (ifsp >= MAXNEST) fatal("IL: if nesting too deep");
                /* fill the frame in place; c3 has no struct-value assignment */
                fr = &ifstk[ifsp++];
                fr->iet = et;
                fr->itl.labnum = fr->ifl.labnum = 0;
                fr->itl.labdreg = fr->itl.labxreg =
                    fr->ifl.labdreg = fr->ifl.labxreg = NULL;
                if (et) { dotest(cond,&fr->itl,&fr->ifl,FALSE); uselabel(&fr->ifl); }
                else    { dotest(cond,&fr->itl,&fr->ifl,TRUE);  uselabel(&fr->itl); }
                break;
            }
            case IL_ELSE: {
                register ifframe *fr = &ifstk[ifsp-1];

                if (fr->iet == 0) {
                    fr->iet = 1;
                    gen(JMP, fr->itl.labnum = getlabel(0), 0);
                    fr->itl.labdreg = dregcont;
                    fr->itl.labxreg = xregcont;
                    dregcont = treecopy(fr->ifl.labdreg);
                    xregcont = treecopy(fr->ifl.labxreg);
                    uselabel(&fr->ifl);
                }
                break;
            }
            case IL_ENDIF: {
                register ifframe *fr = &ifstk[--ifsp];

                uselabel(fr->iet ? &fr->itl : &fr->ifl);
                break;
            }
            case IL_WHILE: {                    /* empty-body flag; test+ENDE */
                int eb;
                register expnode *cond;
                register whileframe *w;

                eb = ilrint();
                cond = optim(rdexpr());
                chkdecl(cond);
                if (whsp >= MAXNEST) fatal("IL: while nesting too deep");
                w = &whstk[whsp++];
                w->whsavb = breakptr;
                w->whsavc = contptr;
                breakptr = &w->brk;
                contptr = &w->cnt;
                w->brk.labnum = 0;
                w->brk.labdreg = w->brk.labxreg = NULL;
                w->cnt.labnum = getlabel(0);
                w->cnt.labdreg = dregcont;
                w->cnt.labxreg = xregcont;
                dregcont = xregcont = NULL;
                w->brk.labsp = w->cnt.labsp = sp;
                w->cond = cond;
                if (eb) w->tlab = w->cnt.labnum;
                else { gen(JMP, w->cnt.labnum, 0); label(w->tlab = getlabel(0)); }
                break;
            }
            case IL_ENDWHILE: {
                register whileframe *w = &whstk[whsp-1];

                uselabel(&w->cnt);
                w->cnt.labnum = w->tlab;
                dotest(w->cond, &w->cnt, &w->brk, FALSE);
                uselabel(&w->brk);
                breakptr = w->whsavb;
                contptr = w->whsavc;
                whsp--;
                break;
            }
            case IL_BREAK:
                if (breakptr) modnjmp(breakptr);
                lastst = BREAK;
                break;
            case IL_CONT:
                if (contptr) modnjmp(contptr);
                lastst = CONTIN;
                break;
            case IL_FOR: {                      /* init, test, update (each +ENDE) */
                register expnode *initp;
                register forframe *fr;

                initp = rdexpr();
                if (forsp >= MAXNEST) fatal("IL: for nesting too deep");
                fr = &forstk[forsp++];
                fr->testp = rdexpr();
                fr->updp = rdexpr();
                emit2(optim(initp));            /* init runs once, here */
                fr->frsavb = breakptr;
                fr->frsavc = contptr;
                breakptr = &fr->brk;
                contptr = &fr->cnt;
                fr->brk.labsp = fr->cnt.labsp = sp;
                fr->brk.labnum = fr->cnt.labnum = fr->tst.labnum = 0;
                fr->brk.labdreg = fr->brk.labxreg = fr->cnt.labdreg =
                    fr->cnt.labxreg = fr->tst.labdreg = fr->tst.labxreg = NULL;
                fr->slab = getlabel(0);
                if (fr->testp) { fr->testp = optim(fr->testp); chkdecl(fr->testp); }
                if (fr->updp)  { fr->updp  = optim(fr->updp);  chkdecl(fr->updp); }
                if (fr->testp) gen(JMP, setlabel(&fr->tst), 0);
                clrconts();
                label(fr->slab);
                break;
            }
            case IL_ENDFOR: {
                register forframe *fr = &forstk[--forsp];

                uselabel(&fr->cnt);
                if (fr->updp) emit2(fr->updp);
                if (fr->testp) {
                    uselabel(&fr->tst);
                    fr->cnt.labnum = fr->slab;
                    dotest(fr->testp, &fr->cnt, &fr->brk, FALSE);
                } else gen(JMP, fr->slab, 0);
                uselabel(&fr->brk);
                breakptr = fr->frsavb;
                contptr = fr->frsavc;
                break;
            }
            case IL_DO: {                       /* body first, test at IL_DOWHILE */
                register whileframe *w;

                if (whsp >= MAXNEST) fatal("IL: loop nesting too deep");
                w = &whstk[whsp++];
                w->whsavb = breakptr;
                w->whsavc = contptr;
                breakptr = &w->brk;
                contptr = &w->cnt;
                w->brk.labsp = w->cnt.labsp = sp;
                w->brk.labnum = w->cnt.labnum = 0;
                w->brk.labdreg = w->brk.labxreg = w->cnt.labdreg =
                    w->cnt.labxreg = NULL;
                clrconts();
                label(w->tlab = getlabel(0));
                break;
            }
            case IL_DOWHILE: {                  /* trailing test + ENDE */
                register whileframe *w = &whstk[whsp-1];
                register expnode *cond;

                cond = optim(rdexpr());
                chkdecl(cond);
                uselabel(&w->cnt);
                w->cnt.labnum = w->tlab;
                dotest(cond, &w->cnt, &w->brk, FALSE);
                uselabel(&w->brk);
                breakptr = w->whsavb;
                contptr = w->whsavc;
                whsp--;
                break;
            }
            case IL_GOTO:                       /* label name */
                gen(GOTO, labelnum(ilrname()), 0);
                lastst = GOTO;
                break;
            case IL_LABEL:                      /* label name */
                gen(LABEL, labelnum(ilrname()), 0);
                clrconts();
                break;
            case IL_SWITCH: {                   /* control expr + ENDE */
                register expnode *ptr;
                register switchframe *s;

                if (swsp >= MAXNEST) fatal("IL: switch nesting too deep");
                s = &swstk[swsp++];
                s->savcase = caseptr;
                s->savlast = lastcase;
                s->savdef = deflabel;
                s->swsavb = breakptr;
                caseptr = NULL;
                breakptr = &s->brk;
                s->brk.labnum = deflabel = 0;
                s->brk.labdreg = s->brk.labxreg = NULL;
                s->brk.labsp = sp;
                if (ptr = optim(rdexpr())) {
                    chkdecl(ptr);
                    switch (ptr->type) {
                        case CHAR:
                        case LONG: cvt(ptr,INT);
                        case INT:
                        case UNSIGN: break;
                        default: integerr(ptr); makedummy(ptr);
                    }
                    ldxexp(ptr);
                    reltree(ptr);
                } else error("expression missing");
                gen(JMP, s->tests = getlabel(0), 0);
                break;
            }
            case IL_CASE: {                     /* case value */
                register casnode *ptr;
                int val = ilrint();

                clrconts();
                if (ptr = freecase) freecase = ptr->clink;
                else ptr = (casnode *) grab(CASESIZE);
                if (caseptr) lastcase->clink = ptr;
                else caseptr = ptr;
                lastcase = ptr;
                ptr->clink = NULL;
                ptr->cval = val;
                label(ptr->clab = getlabel(0));
                break;
            }
            case IL_DEFAULT:
                if (deflabel) error("multiple defaults");
                else label(deflabel = getlabel(0));
                clrconts();
                break;
            case IL_ENDSW: {
                register switchframe *s = &swstk[swsp-1];
                register casnode *cptr, *temp;

                if (lastst == 0) gen(JMP, setlabel(&s->brk), 0);
                label(s->tests);
                cptr = caseptr;
                while (cptr) {
                    temp = cptr->clink;
                    gen(JMPEQ, cptr->clab, cptr->cval);
                    cptr->clink = freecase;
                    freecase = cptr;
                    cptr = temp;
                }
                clrconts();
                if (deflabel) {
                    gen(JMP, deflabel, 0);
                    dregcont = treecopy(s->brk.labdreg);
                    xregcont = treecopy(s->brk.labxreg);
                }
                uselabel(&s->brk);
                caseptr = s->savcase;
                deflabel = s->savdef;
                lastcase = s->savlast;
                breakptr = s->swsavb;
                swsp--;
                break;
            }
            case IL_BLOCK: {                    /* frame level after local decls */
                int blev = ilrint();

                if (maxpush > blev) maxpush = blev;
                stlev = blev;
                sp = modstk(blev);
                break;
            }
            case IL_ENDBLOCK: {                 /* savlev, had-locals flag */
                int savlev = ilrint();
                int hadlocals = ilrint();

                if (hadlocals) clrconts();
                stlev = savlev;
                sp = (lastst != RETURN) ? modstk(savlev) : savlev;
                break;
            }
            case IL_ENDFUNC:
                if (lastst != RETURN) {
                    modstk(0);
                    gen(RETURN,0,0);
                }
                endfunc();
                break;
            default:
                fatal("IL: unexpected record at statement level");
        }
    }
    epilogue();

    if (ferror(code)) fatal("error writing assembly code file");
    fflush(stdout);
    if (errcount) {
        fprintf(stderr,"errors in code generation : %d\n",errcount);
        exit(1);
    }
}

errexit()
{
    exit(1);
}
