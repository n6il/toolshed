/*
 * proto.h - abstract ANSI prototypes for every GLOBAL function of the "comp"
 * compiler (the c3 self-hosting compiler).
 *
 * #included by cj.h, so every comp/*.c sees these declarations.  Two reasons
 * they must exist:
 *   1. Without a prototype a K&R call is implicitly declared "int", so a
 *      pointer-returning function's result is truncated to 32 bits on a 64-bit
 *      (LP64) host -> garbage pointer -> crash.  (On the 16-bit 6809 target
 *      int == pointer, so the originals were fine.)
 *   2. They kill the cross-file implicit-function-declaration warnings and let
 *      every call site type-check its arguments.
 *
 * These are ABSTRACT prototypes: parameter TYPES only, no parameter names.
 * Each one's return type and parameter types/count EXACTLY match the function's
 * definition.  No-argument functions use "()" not "(void)": comp is itself
 * cross-compiled by c3 for OS-9, whose dialect maps void->int, so a literal
 * "(void)" would be read as one int parameter.
 *
 * STATIC functions are prototyped in their own .c file, not here.  Functions
 * built under only one configuration are guarded the same way as their
 * definition (MKIL = the two-pass split: ilemit/ilwrite are pass-1-only,
 * ilread/cmain2* are pass-2-only; DOFLOATS/PROF/PTREE as in cj.h / the source).
 *
 * stats.c's dotest()/modnjmp() are deliberately NOT here: they are static to
 * stats.c (called only within it), so they are prototyped in stats.c itself.
 */

/* bool.c */
void tranbool(expnode *, labstruc *, labstruc *, int);
void tranrel(int, expnode *, labstruc *, labstruc *, int);
int setlabel(labstruc *);
void uselabel(labstruc *);
int isaleaf(expnode *);
void checkop(expnode *);
int invrel(int);
int revrel(int);
int zeroconst(expnode *);

/* build.c */
expnode *parsexp(int);
expnode *primary();
expnode *explist();
int constexp(int);
int isop();
expnode *getcast();
void experr();
int need(int);
void junk();

/* cmain.c (cmain2.c provides its own main() in the pass-2 binary) */
void errexit();

/* codgen.c */
/* gen() and eprintf() are called throughout with FEWER than their declared
   arguments (the trailing K&R args are simply unused on those paths), so they
   keep an empty-paren "unspecified arguments" prototype - a fixed-arity ANSI
   prototype would turn every short call into a hard error.  The empty parens
   still kill the implicit-declaration warning and pin the return type. */
void gen();
char regname(int);
void transfer(int, int);
void dolongs();          /* int/argword pun arg: empty parens to suppress coercion */
#ifdef DOFLOATS
void dofloats();         /* long param fed an argword: empty parens, no coercion */
#endif
void getcon();
void mwsyscall(char *);
void lcall(char *);
void fcall(char *);
void trouble(int, int, expnode *);
void doref();
char *getrel(int);
void outstr(int, int);

/* emit.c */
void addoff(int);
void deref();
void defcon();
int hostbig();
void defbytes(char *, int);
void deflong(char *);
void ot(char *);
void ol(char *);
void nl();
void ob(int);
void os(char *);
void od();
void label(int);
void olbl(int);
void on(char *);
int modstk(int);
void nlabel(char *, int);

/* declare.c */
void extdef();
void argdef();
void blkdef();
int check_same(symnode *, int, elem *);
int chkreg(int, int);
void newfunc(symnode *, int);
void block(int);
void declist(symnode **);
int setclass();
int settype(long *, dimnode **, elem **);
int declarator(symnode **, dimnode **, int);
int shiftin(int, int);
void clear(symnode **);
void sizerr();
void identerr();

/* get.c */
void preinit();
void skipblanks();
void getch();
char getline1();
char *cgets();

/* inits.c */
void initialise(symnode *, int, int);
void zero(int);
void initerr();

/* lex.c */
void getsym();
void lexinit();
void getword(char *);
int an(int);
symnode *lookup(char *);
void install(char *, int);
int hash(char *);
char *grab(int);
/* number()'s parameters differ by DOFLOATS (numptrs struct vs long *), and
   numptrs is a lex.c-local type not visible here; an empty-paren prototype
   keeps it correct under both configs without leaking the type. */
int number();
#ifdef DOFLOATS
double scale(double, int, int);
double scale0(double, int, int);
#endif
void pstr();
void qstr();
void oc(int);
void oz(int);
int dobslash();
int isoct(int);
int ishex(int);
int addin(char *, int);

/* local.c */
void epilogue();
void locstat(int, int, int);
void defglob(symnode *, int, int);
void extstat(symnode *, int, int);
void defvar(symnode *, int, int);
#ifdef PROF
void profname(char *, int);
void startfunc(char *, int, int, int);
#else
void startfunc(char *, int, int);
#endif
void endfunc();
void defbyte();
void defword();
void comment();
void vsect(int);
void endsect();
void dumpstrings();
void tidy();

/* longs.c */
void lload(expnode *);
void tranlexp(expnode *);
void getadd(expnode *);
void pushcon(expnode *);

#ifdef DOFLOATS
/* floats.c */
void dload(expnode *);
void trandexp(expnode *);
#endif

/* misc.c */
void pushdown(symnode *);
void pullup(symnode *);
void move(char *, char *, int);
void pfile();
void fatal(char *);
void multidef();
void error(char *);
void comperr(expnode *, char *);
void terror(expnode *, char *);
void doerr(int, char *, int);
void eprintf();         /* printf-style: called with fewer args (see gen) */
void eputs(char *);
void eputchar(int);
void reltree(expnode *);
void release(expnode *);
void nodecopy(char *, char *);
int istype();
int issclass();
int decref(int);
int incref(int);
int isbin(int);
dimnode *dimwalk(dimnode *);
#ifdef PTREE
void prtree(expnode *);
void ptree(expnode *);
void pnode(expnode *);
#endif

/* optim.c */
expnode *optim(expnode *);
expnode *fold(expnode *);
int isleaf(expnode *);
void diverr();
expnode *chtype(expnode *);
void chknull(expnode *);
int usual(expnode *);
int cvt(expnode *, int);
int tymatch(expnode *, expnode *);
expnode *fixup(int, int, expnode *, expnode *);
void needlval(expnode *, int);
void chkdecl(expnode *);
int needmos(expnode *, int *, int *);
int chkstrct(expnode *);
void makedummy(expnode *);
int isint(int);
void integerr();

/* regscont.c */
void clrconts();
void setdreg(expnode *);
void setxreg(expnode *);
void setcurr(expnode *);
int cmptrees(expnode *, expnode *);
expnode *subtree(expnode *, expnode *);
expnode *treecont(expnode *, int);
expnode *regcont(expnode *);
expnode *treecopy(expnode *);
void rplcnode(expnode *, int);
void walktree(expnode *, funcp);

/* stats.c */
void statement();
void doif();
void dowhile();
void doswitch();
void docase();
void dodefault();
void dodo();
void dofor();
void doreturn();
void dobreak();
void docont();
void dogoto();
void dolabel();

/* tranexp.c */
void lddexp(expnode *);
void ldxexp(expnode *);
void loadexp(expnode *);
void doload(expnode *);
expnode *tranexp(expnode *);
void tranbinop(int, expnode *);
int isashift(int);
void dobool(expnode *);
void doquery(expnode *, funcp);
void docall(expnode *);
int isdleaf(expnode *);
int regandcon(expnode *);
int isxleaf(expnode *);
int isaddress(expnode *);
void getinx(expnode *);
void doass(expnode *);
expnode *transexp(expnode *);
void assop(expnode *, int);
void stackx(expnode *);
void dostar(expnode *);
void dotoggle(expnode *, int);
void loadxexp(expnode *);
void tranxexp(expnode *);
int isreg(int);

/* treeutil.c */
expnode *newnode(int, expnode *, expnode *, argword, int, char *);
int constop(int, int, int);
int sizeup(symnode *, dimnode *, long);
int getsize();           /* def's int size param is fed a long at call sites;
                            empty parens preserve the original (uncoerced) push */
int arity(int);

/* The IL codec functions (ilwinit/ilwrec/.../ilrinit/ilrrec/.../ilexpr) are
   declared in il.h, which every file that uses them already includes - they
   are intentionally NOT duplicated here. */

#ifdef MKIL
/* ilemit.c (pass-1 IL emission - c3comp1 only); called from declare.c/stats.c
   under #ifdef MKIL.  The IL codec functions themselves live in il.h. */
void ilexpr(expnode *);
#endif

/* cmain2b.c / cmain2c.c - the pass-2 family (compiled and linked only into
   c3comp2, never with -DMKIL).  Declared unconditionally so clang is clean in
   BOTH configs; under -DMKIL these are simply unused declarations.  None of the
   names collide with the MKIL-side (pass-1) symbols. */
int statlabel(int);
int statdef(int);
void dostr();
symnode *mksym(int, int, int, int, char *, int *, int, int, int);
void relsyms();
int labelnum(char *);
expnode *rdexpr();
void c2cinit();
int dostmt(int);
