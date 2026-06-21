/*
 * cmain2c.c - c3comp2 statement / control-flow codegen (split from cmain2.c).
 *
 * The IL statement-record handler (dostmt) and everything it needs: the
 * structured-control-flow frame stacks (if/while/for/switch), the per-block
 * symbol reclaim (flushsyms), and the leaf codegen helpers (emit2/doret/dotest/
 * modnjmp).  Split out of cmain2.c so each translation unit is small enough for
 * c3comp1 (pass 1) to compile within OS-9 Level 2's ~21 KB per-process heap when
 * the toolchain self-hosts.  The driver loop and the data/declaration records
 * stay in cmain2.c, which calls dostmt() for every record it does not handle
 * itself.  Pure code motion: output is byte-identical.
 *
 * The per-function symbol cache + free lists are defined in cmain2.c (allocated
 * in c2init); flushsyms reclaims onto them, so they are extern here.  The frame
 * stacks are private to this file (only dostmt and flushsyms touch them) and are
 * allocated by c2cinit(), which cmain2.c's main() calls at startup.
 */

#include "cj.h"
#include "il.h"

extern char *grab();
extern expnode *rdexpr();           /* IL expression-tree rebuilder (cmain2b.c) */
extern expnode *treecopy();
extern direct expnode *dregcont, *xregcont;

/* per-function symbol cache + free lists, defined in cmain2.c (see there) */
extern symnode **symcache;
extern int nsyms;
extern symnode *symfree;
extern dimnode *dimfree;

/* nesting stack for structured if/else: holds each open if's true/false
 * labels (the locals stats.c doif keeps on the C stack).  The IL marker
 * stream (IL_IF/IL_ELSE/IL_ENDIF) drives push/pop; bodies are processed by
 * the normal record loop in between, so nesting falls out for free. */
#define MAXNEST 12
typedef struct { labstruc itl, ifl; int iet; } ifframe;
static ifframe *ifstk;		/* [MAXNEST], heap (see c2cinit) */
static int ifsp;

/* nesting stack for while loops; carries the break/continue labels and the
 * test tree (a while tests at the BOTTOM, after the body). */
typedef struct {
    labstruc brk, cnt;
    int tlab;
    expnode *cond;
    labstruc *whsavb, *whsavc;
} whileframe;
static whileframe *whstk;	/* [MAXNEST], heap (see c2cinit) */
static int whsp;
static labstruc *breakptr, *contptr;

/* nesting stack for for loops: like while plus a separate test label and the
 * saved test/update trees (a for tests at the bottom, updates before the test) */
typedef struct {
    labstruc brk, cnt, tst;
    int slab;
    expnode *testp, *updp;
    labstruc *frsavb, *frsavc;
} forframe;
static forframe *forstk;		/* [MAXNEST], heap (see c2cinit) */
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
static switchframe *swstk;	/* [MAXNEST], heap (see c2cinit) */
static int swsp;

static marknode(t)              /* mark (blklev) every symnode a saved tree references */
register expnode *t;
{
    if (t) {
        if (t->op == NAME && t->val.sp) t->val.sp->blklev = 1;
        marknode(t->left);
        marknode(t->right);
    }
}

static marklab(L)               /* mark both register-content trees saved in a label */
register labstruc *L;
{
    marknode(L->labdreg);
    marknode(L->labxreg);
}

/* Block-scoped symbol reclaim - the companion to relsyms's whole-function one.
 * relsyms holds every symnode for a whole function, so the biggest function
 * (cmain2.c's main, 220 distinct symbols) needs more heap than OS-9 Level 2 can
 * give one process.  But a synthesised symnode only has to stay interned while
 * something can still REFERENCE it by identity: the register-content tracker
 * (cmptrees compares the symbol pointer), or a control-flow frame that saved a
 * tree to emit later (a loop/if test, or a register snapshot in a frame label).
 *
 * flushsyms() is called at the statement-level clrconts points.  clrconts has
 * just cleared dregcont/xregcont/currexpr, so the only live symnode references
 * left are in OPEN frames; everything else is dead now and can return to the
 * free lists instead of waiting for IL_FUNC.  That bounds the working set to a
 * basic block's symbols, which is what lets main() fit.
 *
 * Byte-identical to the no-flush build: across a clrconts the register state is
 * empty, so symnode identity never feeds a comparison that spans the boundary -
 * a symbol re-referenced after the flush gets a fresh symnode but is reloaded
 * anyway (registers were cleared), emitting the same code.  WITHIN a block there
 * is no flush, so interning there is untouched.  marknode keeps exactly the
 * symbols the open frames' saved trees still point at, so those trees stay
 * valid when finally emitted. */
static flushsyms()
{
    register int i, k;
    register symnode *s;
    register dimnode *d, *dn;

    /* mark symbols still reachable from open frames: saved loop/if tests and the
       register-content snapshots stashed in each frame's labels (IL_ELSE,
       IL_WHILE, IL_ENDSW save dregcont/xregcont into label.lab[dx]reg) */
    for (i = 0; i < ifsp; i++) { marklab(&ifstk[i].itl); marklab(&ifstk[i].ifl); }
    for (i = 0; i < whsp; i++) {
        marknode(whstk[i].cond); marklab(&whstk[i].brk); marklab(&whstk[i].cnt);
    }
    for (i = 0; i < forsp; i++) {
        marknode(forstk[i].testp); marknode(forstk[i].updp);
        marklab(&forstk[i].brk); marklab(&forstk[i].cnt); marklab(&forstk[i].tst);
    }
    for (i = 0; i < swsp; i++) marklab(&swstk[i].brk);

    /* sweep: keep the marked symbols (compacting the cache), reclaim the rest
       onto the free lists exactly as relsyms does per symbol */
    k = 0;
    for (i = 0; i < nsyms; i++) {
        s = symcache[i];
        if (s->blklev) { s->blklev = 0; symcache[k++] = s; continue; }
        for (d = s->dimptr; d; d = dn) { dn = d->dptr; d->dptr = dimfree; dimfree = d; }
        if (btype(s->type) == USTRUCT && s->size) {
            register symnode *tag = (symnode *) s->size;
            tag->snext = symfree; symfree = tag;
        }
        s->snext = symfree; symfree = s;
    }
    nsyms = k;
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

/* break/continue jump (stats.c modnjmp) */
static modnjmp(labptr)
register labstruc *labptr;
{
    modstk(labptr->labsp);
    gen(JMP, setlabel(labptr), 0);
}

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

/* Allocate the control-flow frame stacks on the heap (like c2init's IL-rebuild
 * tables; keeping them off the bss lets c3comp2's static data area fit a single
 * 8 KB block, required for OS-9 Level-2 F$Fork).  Called once from main(). */
c2cinit()
{
    ifstk  = (ifframe *)    grab(MAXNEST * sizeof(ifframe));
    whstk  = (whileframe *) grab(MAXNEST * sizeof(whileframe));
    forstk = (forframe *)   grab(MAXNEST * sizeof(forframe));
    swstk  = (switchframe *)grab(MAXNEST * sizeof(switchframe));
}

/* Handle one statement / control-flow IL record (everything the cmain2.c driver
 * loop does not own itself).  Returns 1 if op was handled, 0 if unrecognised. */
dostmt(op)
int op;
{
    switch (op) {
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
            /* field-at-a-time, not one chained assignment: c3comp1 (pass 1)
               builds a whole expression as one tree before emitting, so a long
               a=b=c=...=NULL chain of struct derefs is a big transient tree that
               overran the OS-9 Level 2 heap when self-hosting.  Separate
               statements keep each tree tiny; c3comp2's output is unchanged. */
            fr->itl.labnum = 0; fr->ifl.labnum = 0;
            fr->itl.labdreg = NULL; fr->itl.labxreg = NULL;
            fr->ifl.labdreg = NULL; fr->ifl.labxreg = NULL;
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
            fr->brk.labnum = 0; fr->cnt.labnum = 0; fr->tst.labnum = 0;
            fr->brk.labdreg = NULL; fr->brk.labxreg = NULL;  /* not chained: keep */
            fr->cnt.labdreg = NULL; fr->cnt.labxreg = NULL;  /* pass-1 trees small */
            fr->tst.labdreg = NULL; fr->tst.labxreg = NULL;
            fr->slab = getlabel(0);
            if (fr->testp) { fr->testp = optim(fr->testp); chkdecl(fr->testp); }
            if (fr->updp)  { fr->updp  = optim(fr->updp);  chkdecl(fr->updp); }
            if (fr->testp) gen(JMP, setlabel(&fr->tst), 0);
            clrconts(); flushsyms();
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
            w->brk.labnum = 0; w->cnt.labnum = 0;
            w->brk.labdreg = NULL; w->brk.labxreg = NULL;  /* not chained: keep */
            w->cnt.labdreg = NULL; w->cnt.labxreg = NULL;  /* pass-1 trees small */
            w->cond = NULL;         /* a do-loop never saves a cond here (the
                                       test is read at IL_DOWHILE); NULL it so
                                       flushsyms doesn't walk a garbage tree */
            clrconts(); flushsyms();
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
            clrconts(); flushsyms();
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

            clrconts(); flushsyms();
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
            clrconts(); flushsyms();
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
            clrconts(); flushsyms();
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

            if (hadlocals) { clrconts(); flushsyms(); }
            stlev = savlev;
            sp = (lastst != RETURN) ? modstk(savlev) : savlev;
            break;
        }
        default:
            return 0;
    }
    return 1;
}
