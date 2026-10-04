/*
 * proto.h - abstract ANSI prototypes for every GLOBAL function of the
 * "compexp" (expanded) compiler.
 *
 * cj.h #includes this file at its end, so every compexp/*.c sees these
 * declarations.  Two reasons they must exist:
 *   1. Without a prototype a K&R call is implicitly declared "int", so a
 *      pointer-returning function's result is truncated to 32 bits on a 64-bit
 *      (LP64) host -> garbage pointer -> crash.  (On the 16-bit 6809 target
 *      int == pointer, so the originals were fine.)
 *   2. They kill the cross-file implicit-function-declaration / implicit-int
 *      errors and let every call site type-check its arguments.
 *
 * These are ABSTRACT prototypes: parameter TYPES only, no parameter names.
 * Each one's return type and parameter types/count EXACTLY match the function's
 * own (preprocessor-active) definition in compexp/*.c.
 *
 * EXCLUDED: static (file-local) functions, main(), and the functions already
 * declared in cj.h's own extern block (parsexp, primary, explist, getcast,
 * newnode, optim, fold, chtype, fixup, tranexp, dimwalk).
 *
 * The sibling ANSI tree comp/proto.h is a style reference only; several compexp
 * signatures differ from comp's, so its prototypes cannot be used here.
 */

/* bool.c */
void tranbool(expnode *, labstruc *, labstruc *, int);
void tranrel(int, expnode *, labstruc *, labstruc *, int);
int setlabel(labstruc *);
void uselabel(labstruc *);
int isaleaf(expnode *);
int isauto(expnode *);
void checkop(expnode *);
int invrel(int);
int revrel(int);
int zeroconst(expnode *);

/* build.c */
int constexp(int);
int isop(void);
int constop(int, int, int);
void experr(void);

/* cmain.c */
void errexit(void);

/* codgen.c */
void gen(int, int, long, expnode *);
char regname(int);
void transfer(int, int);
void dolongs(int, int *);
void dofloats(int, int);
void getcon(int *, int);
void defcon(INTTYPE *, int);
void mwsyscall(char *);
void lcall(char *);
void fcall(char *);
void trouble(int, int, expnode *);
void doref(int, int, long, int);
void deref(int, long, int);
char *getrel(int);
void ot(char *);
void ol(char *);
void nl(void);
void ob(int);
void os(char *);
void od(int16_t);
void label(int);
void olbl(int);
void on(char *);
int modstk(int);
void nlabel(char *, int);
void outstr(int, int);

/* declare.c */
void extdef(void);
void argdef(void);
void blkdef(void);
int check_same(symnode *, int, elem *);
int chkreg(int, int);
void newfunc(symnode *, int);
void block(int);
void declist(symnode **);
int setclass(void);
int settype(long *, dimnode **, elem **);
int declarator(symnode **, dimnode **, int);
int shiftin(int, int);
int sizeup(symnode *, dimnode *, long);
int getsize(int, int, dimnode *);
void clear(symnode **);
void sizerr(void);
void identerr(void);

/* fadjust.c */
double fadjust(double);

/* floats.c */
void dload(expnode *);
void trandexp(expnode *);

/* get.c */
void preinit(void);
void blanks(void);
void getch(void);
char getline2(void);
char *cgets(void);

/* inits.c */
void initialise(symnode *, int, int);
void zero(int);
void initerr(void);

/* lex.c */
void getsym(void);
void lexinit(void);
void getword(char *);
int an(int);
symnode *lookup(char *);
void install(char *, int);
int hash(char *);
char *grab(int);
double scale(double, int, int);
double scale0(double, int, int);
void pstr(void);
void qstr(void);
void oc(int);
void oz(int);
char dobslash(void);
int isoct(char);
int ishex(char);

/* local.c */
void epilogue(void);
void locstat(int, int, int);
void defglob(symnode *, int, int);
void extstat(symnode *, int, int);
void defvar(symnode *, int, char);
void profname(char *, int);
#ifdef PROF
void startfunc(char *, int, int, int);
#else
void startfunc(char *, int, int);
#endif
void endfunc(void);
void defbyte(void);
void defword(void);
void comment(void);
void vsect(int);
void endsect(void);
void dumpstrings(void);
int tidy(void);

/* longs.c */
void lload(expnode *);
void tranlexp(expnode *);
void getadd(expnode *);
void pushcon(expnode *);

/* misc.c */
void pushdown(symnode *);
void pullup(symnode *);
void move(void *, void *, int);
void pfile(void);
void fatal(char *);
void multidef(void);
void error(char *);
void comperr(expnode *, char *);
void terror(expnode *, char *);
void doerr(int, char *, int);
void eprintf(char *, ...);
void eputs(char *);
void eputchar(char);
void reltree(expnode *);
void release(expnode *);
void nodecopy(void *, void *);
int istype(void);
int issclass(void);
int decref(int);
int incref(int);
int isbin(int);
int need(int);
void junk(void);

/* optim.c */
int isleaf(expnode *);
void diverr(void);
void chknull(expnode *);
int usual(expnode *);
int cvt(expnode *, int);
int tymatch(expnode *, expnode *);
void needlval(expnode *, int);
void chkdecl(expnode *);
int needmos(expnode *, int *, int *);
int chkstrct(expnode *);
void makedummy(expnode *);
int isint(int);
void integerr(expnode *);

/* prtree.c (debug tree dumpers; built under -DDEBUG) */
void getkeys(void);
void prtree(expnode *, char *);
void ptree(expnode *);
void pnode(expnode *);

/* regscont.c */
void clrconts(void);
void setdreg(expnode *);
void setxreg(expnode *);
void setcurr(expnode *);
int cmptrees(expnode *, expnode *);
expnode *subtree(expnode *, expnode *);
expnode *treecont(expnode *, int);
expnode *regcont(expnode *);
expnode *treecopy(expnode *);
void rplcnode(expnode *, int);
void walktree(expnode *, int (*)(void));

/* stats.c */
void statement(void);
void doif(void);
void dowhile(void);
void doswitch(void);
void docase(void);
void dodefault(void);
void dodo(void);
void dofor(void);
void doreturn(void);
void dobreak(void);
void docont(void);
void modnjmp(labstruc *);
void dogoto(void);
void dolabel(void);
void dotest(expnode *, labstruc *, labstruc *, int);

/* tranexp.c */
void lddexp(expnode *);
void ldxexp(expnode *);
void loadexp(expnode *);
void doload(expnode *);
expnode *transexp(expnode *);
void tranbinop(int, expnode *);
int isashift(int);
void dobool(expnode *);
void doquery(expnode *, int (*)(struct expstr *));
void docall(expnode *);
int isdleaf(expnode *);
int regandcon(expnode *);
int isxleaf(expnode *);
int isaddress(expnode *);
void getinx(expnode *);
void doass(expnode *);
void assop(expnode *, int);
void stackx(expnode *);
void dostar(expnode *);
void dotoggle(expnode *, int);
void loadxexp(expnode *);
void tranxexp(expnode *);
int isreg(int);
