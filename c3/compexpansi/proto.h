/* bool.c */
void uselabel(labstruc *lab);
void tranrel(int op, expnode *node, labstruc *tlab, labstruc *flab, int nj);
int setlabel(labstruc *lab);
void tranbool(expnode *node, labstruc *tlab, labstruc *flab, int nj);
int zeroconst(expnode *node);
int isaleaf(expnode *node);
int isauto(expnode *node);
int revrel(int op);

/* build.c */
int isop(void);
expnode *parsexp(int priority);
expnode *primary(void);
void experr(void);
expnode *explist(void);
int constexp(int p);
int constop(int op, int r, int l);

/* cmain.c */
void errexit(void);
void fatal(char *errstr);

/* codgen.c */
void gen(int op, int rtype, int arg, expnode *val);
char regname(int r);
void transfer(int r1, int r2);
void dofloats(int op, int arg);
void getcon(int *p, int n);
void defcon(INTTYPE *p, int n);
void mwsyscall(char *s);
void lcall(char *s);
void fcall(char *s);
void trouble(int op, int arg, expnode *val);
void deref(int arg, int val, int offset);
void dolongs(int op, int arg);
char *getrel(int op);
void ot(char *s);
void ol(char *s);
void nl(void);
void ob(int b);
void os(char *s);
void od(short n);
void label(int n);
void olbl(int n);
void on(char *s);
int modstk(int nsp);
void nlabel(char *ptr, int scope);
void outstr(int reg, int l);

/* declare.c */
void argdef(void);
void blkdef(void);
int check_same(symnode *ptr, int type, elem *eptr);
int chkreg(int sclass, int type);
void newfunc(symnode *fptr, int sclass);
void declist(symnode **list);
int setclass(void);
int shiftin(int a, int b);
void sizerr(void);
void identerr(void);
int settype(int *size, dimnode **dimptr, elem **ellist);
int declarator(symnode **ptr, dimnode **dptr, int bastype);
void clear(symnode **list);
int sizeup(symnode *ptr, dimnode *dimptr, int size);
int getsize(int t, int size, dimnode *dptr);
void extdef(void);
void block(int stkadj);

/* get.c */
void preinit(void);
void blanks(void);
void getch(void);
char getline(void);
char *cgets(void);

/* inits.c */
void initialise(symnode *ptr, int tsc, int type);
void zero(int n);
void initerr(void);

/* lex.c */
void lexinit(void);
void getsym(void);
char *grab(int size);
void getword(char *name);
int an(int c);
void install(char *word, int typ);
int hash(char *word);
char *grab(int size);
double scale(double n, int power, int esign);
double scale0(double n, int power, int esign);
void pstr(void);
void qstr(void);
void oc(int c);
void oz(int n);
char dobslash(void);
int isoct(char c);
int ishex(char c);


/* local.c */
void epilogue(void);
void locstat(int l, int size, int area);
void defglob(symnode *ptr, int size, int area);
void extstat(symnode *ptr, int size, int area);

#ifdef PROF
void profname(char *name, int lab);
void startfunc(char *name, int flag, int paramreg, int lab);
#else
void startfunc(char *name, int flag, int paramreg);
#endif
void endfunc(void);
void defbyte(void);
void defword(void);
void comment(void);
void vsect(int area);
void endsect(void);
void dumpstrings(void);
int tidy(void);

/* longs.c */
void lload(expnode *ptr);
void tranlexp(expnode *node);
void getadd(expnode *ptr);
void pushcon(expnode *p);

/* floats.c */
void dload(expnode *ptr);
void trandexp(expnode *node);

/* optim.c */
int isleaf(expnode *node);
void diverr(void);
void chkdecl(expnode *node);
int cvt(expnode *node, int t);
int isint(int t);

/* tranexp.c */
expnode *tranexp(expnode *tree);
void lddexp(expnode *tree);
void doquery(expnode *tree, void (*loadfunc)(struct expstr *));
void getinx(expnode *node);
int isreg(int r);
void loadexp(expnode *tree);
int isdleaf(expnode *node);
int isxleaf(expnode *node);
void docall(expnode *node);
void dostar(expnode *node);

/* regscont.c */
void clrconts(void);
void setdreg(expnode *tree);
void setcurr(expnode *tree);
int cmptrees(expnode *tree, expnode *cont);
expnode *subtree(expnode *tree, expnode *cont);
expnode *treecont(expnode *tree, int op);
expnode *regcont(expnode *tree);
void setxreg(expnode *tree);
expnode *treecopy(expnode *tree);
void rplcnode(expnode *tree, int reg);
void walktree(expnode *tree, void (*funct)(expnode *tree));

/* misc.c */
void pushdown(symnode *sptr);
void pullup(symnode *sptr);
void move(void *p1, void *p2, int count);
void pfile(void);
void reltree(expnode *tree);
void fatal(char *errstr);
void comperr(expnode *node, char *errstr);
void terror(expnode *node, char *errstr);
void dowarning(int n, char *warnstr, int lno);
void doerr(int n, char *errstr, int lno);
void eprintf(char *s1, char *s2, char *s3);
void eputs(char *s);
void eputchar(char c);
void nodecopy(expnode *n1, expnode *n2);
int need(int key);
void error(char *s);
int istype(void);
int issclass(void);
int decref(int t);
int incref(int t);
int isbin(int op);
dimnode *dimwalk(dimnode *dptr);
void release(expnode *node);
void multidef(void);
void junk(void);
#ifdef PTREE
void prtree(expnode *node);
void ptree(expnode *node);
void pnode(expnode *node);
#endif

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
void modnjmp(labstruc *labptr);
void dogoto(void);
void dolabel(void);
void dotest(expnode *ptr, labstruc *tl, labstruc *fl, int nj);

