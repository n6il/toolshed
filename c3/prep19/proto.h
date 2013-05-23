/* cp.c */

int getln(int a);
int trigraph(int a);
int gettri(int b);
int xtndln(int a);
int wspace(int a);
void convert(char x, char y);
int cmnt(int a);
int comm(int a, int *b);
int qa(char *ln, char x, int b);
int qa2(char *ln, char x, int b);
int space(int a, int b);
int tstline(int a);
int usage(void);
int getident(char *ln, int b);
int skpbl(char *ln, int a);
int rskpbl(char *ln, int a);
char *escseq(char *l, char *ln);


/* cp1.c */

void doinclude(char *ln);
void tstdupdef(void);
void doundef(char *ln);
void splittok(char *ln, int b);
int tokopr(char *ln, int b);
int toknum(char *ln, char *lnptr);
void initstdefs(void);
void cncatstr(char *ln);


/* cp2.c */

int gettoken(char *ln, int b);
int toksrch(char *ln, char *tk, int pv, int kv, int bv, int b, int *tcnt);
int tstargs(int i);
void addqmac(char *mac);
char *getoknum(char *tok, char *buf, int num);
void putdtbl(int b, int c);
void putiddtbl(int b, int d);
void dodefine(int a, int b);
char prep(void);
char dopragma(char *ln);
void doline(char *ln);


/* cp3.c */

void doif(char *ln);
void doelse(void);
void doendif(void);
void doelif(char *ln);
int gettorf(char *ln);
int dodef(char *ln, int b);
int ifcalc(char *ln);
void dodfined(char *ln);
void zeroident(char *ln);
void lnprint(int x, int y, char *str);
int doerr(int code, int errptr);



/* cp4.c */

void expand(char *ln, int *loop, int *lpcntr);
void expln(char *ln, int *l, int *lc);


/* findstr.c */

int findstr(int pos, char *line_str, char *string);


/* findchar.c */

int findchar(char *search_str, char c);


/* findchar.c */

int strcmp2(char *str1, char *str2);


/* lnread.c */

int lnread(int path, char *s, int n);


/* solve.c */

unsigned        solve(char *ln);


/* max.c */

int max(int a, int b);
int min(int a, int b);


/* itoa.c */

char *itoa(char *str, int num);
