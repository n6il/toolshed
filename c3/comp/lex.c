/*
     Modification history for lex.c:
        24-May-83       Changed string file to use fopen() update mode.
                        Made stringlen, sfile and datstring be DIRECT.
        25-May-83       Better register variable usage.
        09-Jun-83       Changed "out of space" to "out of memory".
        29-Aug-83       Float constants in the form 1e7 were not recognized.
        24-Apr-84 LAC   Conversion for UNIX.
*/

/*************************************************
 *                                               *
 *      SCANNER OR LEXICAL ANALYZER              *
 *                                               *
 *************************************************/

#include "cj.h"
#include "il.h"

static int numshf(char *);
static double normaliz(char *);

#define     isdigit(c)  (chartab[c]==DIGIT)

typedef union {
    INTTYPE *ip;
    LONGTYPE *lp;
#ifdef DOFLOATS
    FLOATYPE *fp;
    DBLETYPE *dp;
#endif
} numptrs;

#ifdef  DOFLOATS
extern double atoftbl[];
#endif
direct int stringlen;
static int strtempid;        /* pass-1 string temp-id; pass 2 maps it to a label */

extern symnode *lookup(char *);
extern char *grab(int);


void getsym(void)
{
    register symnode *ptr;
    register int numtype;
    register numptrs np;
    register LONGTYPE *longp;
#ifdef  DOFLOATS
    register DBLETYPE *dblp;
    DBLETYPE n;
#else
    LONGTYPE n;
#endif
    char name[NAMESIZE+1];

    skipblanks();
    if (curch==EOF) {sym=EOF; return; }

    symptr=lptr-1;
    symline=lineno;
    while(!(sym=chartab[curch])) {
        error("bad character");
        getch();
        symptr=lptr;
    }
    symval=valtab[curch];

    switch(sym) {
        case LETTER:
            getword(name);
            ptr=lookup(name);
            if((sym=ptr->type)==KEYWORD){
                if((symval=ptr->storage)==SIZEOF){
                    sym=SIZEOF;
                    symval=14;
                }
            } else {
                sym = NAME;
                symval = (argword) ptr;
            }
            break;
        case DIGIT:
#ifdef  DOFLOATS
            numtype = number(INT,&n);
#else
            numtype = number(&n);
#endif
donum:
#ifdef DOFLOATS
            np.dp = &n;
#else
            np.lp = &n;
#endif
            switch(numtype) {
                case INT:
                    symval = *np.ip;
                    sym = CONST;
                    break;
                case LONG:
                    longp = (long *) grab(sizeof(long));
                    *longp = *np.lp;
                    symval = (argword) longp;
#ifdef DEBUG
fprintf(stderr,"getsym: symval=%04X, *longp=%08lX\n",symval,*longp);
#endif
                    sym = LCONST;
                    break;
#ifdef  DOFLOATS
                case DOUBLE:
                    dblp = (double *) grab(sizeof(double));
                    *dblp = *np.dp;
                    symval = (argword) dblp;
                    sym = FCONST;
                    break;
#endif
                default:
                    error("constant overflow");
                    symval = 1;
                    sym = CONST;
                    break;
            }
            break;

        case PRIME: pstr(); sym=CONST; break;
        case QUOTE: qstr(); sym=STRING; break;
        default:
            getch();
            switch(sym) {
#ifdef  DOFLOATS
                case DOT:
                    if(isdigit(curch)) {
                        numtype=number(DOUBLE,&n);
                        goto donum;
                    }
                    break;
#endif
                case AMPER :
                    switch(curch) {
                        case '&':
                            sym=DBLAND;
                            getch();
                            symval=5;
                            break;
                        case '=':
                            sym=ASSAND;
                            symval=2;
                            getch();
                            break;
                    }
                    break;
                case ASSIGN:
                    if (curch=='=') {
                        sym=EQ;
                        symval=9;
                        getch();
                    }
                    break;
                case OR :
                    switch (curch) {
                        case '|':
                            sym=DBLOR;
                            getch();
                            symval=4;
                            break;
                        case '=':
                            sym=ASSOR;
                            getch();
                            symval=2;
                            break;
                    }
                    break;
                case NOT:
                    if(curch=='=') {
                        sym=NEQ;
                        getch();
                        symval=9;
                    }
                    break;
                case STAR:
                    if (curch=='=') {
                        sym=ASSMUL;
                        getch();
                        symval=2;
                    }
                    break;
                case DIV:
                case MOD:
                case XOR:
                    if(curch=='=') {
                        sym=sym+(ASSPLUS-PLUS);
                        getch();
                        symval=2;
                    }
                    break;
                case LT:
                    switch (curch) {
                        case '<':
                            sym=SHL;
                            symval=11;
                            getch();
                            if (curch=='='){
                                sym=ASSSHL;
                                symval=2;
                                getch();
                            }
                            break;
                        case '=':
                            sym=LEQ;
                            symval=10;
                            getch();
                            break;
                    }
                    break;
                case GT:
                    switch (curch) {
                        case '>':
                            sym=SHR;
                            symval=11;
                            getch();
                            if (curch=='=') {
                                sym=ASSSHR;
                                symval=2;
                                getch();
                            }
                            break;
                        case '=':
                            sym=GEQ;
                            symval=10;
                            getch();
                            break;
                    }
                    break;
                case PLUS :
                    switch(curch){
                        case '+':
                            sym=INCBEF;
                            symval=14;
                            getch();
                            break;
                        case '=':
                            sym=ASSPLUS;
                            symval=2;
                            getch();
                            break;
                    }
                    break;
                case NEG:
                    switch(curch){
                        case '-':
                            sym=DECBEF;
                            symval=14;
                            getch();
                            break;
                        case '=':
                            sym=ASSMIN;
                            symval=2;
                            getch();
                            break;
                        case '>':
                            sym=ARROW;
                            symval=15;
                            getch();
                            break;
                    }
                    break;
            }
    }
    return;
}


char valtab[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,9,0,0,0,13,14,0,0,0,14,12,1,14,15,13,
  0,0,0,0,0,0,0,0,0,0,3,0,10,2,10,3,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,6,0,14,0
};

void lexinit(void)
{
     register symnode *ptr;

#ifdef  DOFLOATS
     install("double",DOUBLE);
     install("float",FLOAT);
#endif
     install("typedef",TYPEDEF);
     install("static",STATIC);
     install("sizeof",SIZEOF);
     install("int",INT);
     mosflg=1;
     install("int",INT);
#ifdef  DOFLOATS
     install("float",FLOAT);
#endif
     mosflg=0;
     install("char",CHAR);
     install("void",VOID);	/* ANSI void: c3 has no distinct void type, so
				   settype maps VOID->INT (return values are
				   ignored and all pointers are alike).  It stays
				   a distinct keyword only so declist can spot an
				   explicit "(void)" empty parameter list */
     install("short",SHORT);
     install("auto",AUTO);
     install("extern",EXTERN);
     install("direct",DIRECT);
     install("register",REG);
     install("goto",GOTO);
     install("return",RETURN);
     install("if",IF);
     install("while",WHILE);
     install("else",ELSE);
     install("switch",SWITCH);
     install("case",CASE);
     install("break",BREAK);
     install("continue",CONTIN);
     install("do",DO);
     install("default",DEFAULT);
     install("for",FOR);
     install("struct",STRUCT);
     install("union",UNION);
     install("unsigned",UNSIGN);
     install("long",LONG);
     ptr=lookup("errno");
     ptr->type=INT;
     ptr->storage=EXTERN;
     ptr->size=2;
     ptr=lookup("lseek");
     ptr->type=LONG | FUNCTION;
     ptr->storage=EXTDEF;
     ptr->size=4;
}

void getword(register char name[])
{
    int count;

    for (count=1; an(curch) && count<=NAMESIZE ; ++count) {
        *name++=curch;
        getch();
    }

    if(count == 2) *name++ = UNIQUE;
    *name='\0';

    while(an(curch)) getch();
}

int an(int c)
{
    return (chartab[c]==LETTER || chartab[c]==DIGIT);
}

direct  symnode *freesym;       /* list of free symbol table entries */

symnode *lookup(char name[])
{
    /* return a pointer to a symbol table entry for 'name'  */
    /* if one is not found create one                       */
    register symnode *nptr,**tptr,**tab;
    char *cp;

    /* which symbol table is it in? */
    tab = mosflg ? mostab : hashtab;

    /* point to hash table entry */
    tptr = &tab[hash(name)];

    /* chunter down the list until found or run off the end */
    for (nptr = *tptr; nptr; nptr = nptr->hlink) {
        if (nptr->sname[0] == name[0]
                    && strncmp(name,nptr->sname,NAMESIZE) == 0)
            return nptr;
	}

    /* the entry cannot be found; make a new one */
    if (nptr = freesym) freesym = nptr->hlink;
    else nptr = (symnode *) grab(SYMSIZE);

    strncpy(nptr->sname,name,NAMESIZE);
    nptr->type = UNDECL;
    nptr->storage = 0;
    nptr->args = NULL;      /* no prototype yet: an unprototyped (incl.
			       implicitly-declared) function must have args NULL
			       so call coercion does not walk a wild list */
    /* Fully reset the rest of the entry too.  An entry reused from the freesym
       list (a symbol freed at an earlier block/parameter-scope exit) keeps its
       old blklev/size/etc.; a fresh symbol must read as clean, otherwise e.g. a
       reused parameter name still shows blklev==1 and trips the "named twice"
       check in declist. */
    nptr->blklev = 0;
    nptr->size = 0;
    nptr->dimptr = NULL;
    nptr->offset = 0;
    nptr->x.labflg = 0;

    /* put the new entry on the front of the list;  the chance  */
    /* that a lookup is for a recently declared variable is     */
    /* high - this should therefore speed things up             */
    nptr->hlink = *tptr;
    *tptr = nptr;

    /* prepare for release to free list by 'clear()' */
    nptr->downptr = (symnode *) tptr;

    return nptr;
}

void install(char *word, int typ)
{
        /* put a keyword in the symbol table */
        register symnode *cptr;
        char neword[NAMESIZE+1];

        /* cut the word down to size */
        strncpy(neword,word,NAMESIZE);

        neword[NAMESIZE] = '\0';
        cptr=lookup(neword);

        cptr->type=KEYWORD;
        cptr->storage = typ;
}

int hash(register char *word)
{
    register int n = 0, c;

    while (c = *word++) n += c;

    return (n & 127);
}


#ifdef  DOFLOATS
int number(int type, register numptrs np)
{
    long i;
    double n, scale(), normaliz();
    int exp, esign, digcount = 0;
    register char *cp;

    i = 0;
    n = 0;
    cp = (char *) &n;

    if (type == DOUBLE) goto fraction;

    if (curch == '0') {
        getch();
        if (curch == '.') {
            getch();
            goto fraction;
        }
        if (curch == 'x' || curch == 'X') {
            int c;
            getch();
            while (c = ishex(curch)) {
                i = (i << 4) + (c - (c >= 'A' ? 'A' - 10 : '0'));
                getch();
            }
        } else
            while (isoct(curch)) {
                i = (i << 3) + (curch - '0');
                getch();
            }

        if (curch == 'L' || curch == 'l') {
            getch();
            goto retlong;
        }

        if ((i & 0xFFFF0000) == 0) {
            *np.ip = i;
            return INT;
        }

retlong:
        *np.lp = i;
        return LONG;
    }

    /* here all types still possible; collect in 8 byte integer */
    while (isdigit(curch)) {
        if (addin(&n,curch)) { /* overflow */
            getch();
            return 0;
        }
        getch();
    }

    /* if no previous dp,
     * there must be either a '.' or 'e'
     * here if it is to be double
     */
    if (curch=='.' || curch=='e' || curch=='E') { /* double */
        if (curch == '.') {
            getch();
fraction:
            while (isdigit(curch)) {
                if (addin(&n,curch)) { /* overflow */
                    getch();
                    return 0;
                }
                getch();
                ++digcount;
            }
        }

        n = normaliz(&n);

        if (curch=='E' || curch=='e') {
            esign=1;
            getch();
            if (curch=='+') getch();
            else if(curch=='-') {
                getch();
                esign=0;
            }
            exp=0;
            while(isdigit(curch)) {
                exp=exp*10 + (curch - '0');
                getch();
            }

            if (exp>=40) return 0; /* overflow */

            digcount += esign ? -exp : exp;
        }
        if (digcount<0) {
            digcount= -digcount;
            esign=1;
        } else esign=0;
        *np.dp = scale(n,digcount,esign);
        return DOUBLE;
    }

    /*
     * n[] holds the parsed value as 8 big-endian bytes (n[0] most
     * significant).  The original code aliased it as host long/short
     * (*((long*)&n) etc.), which assumed a big-endian host with 16-bit int
     * and 32-bit long (the 6809).  Extract the high and low 32-bit halves
     * by hand so it works on any host (e.g. little-endian LP64).  Target
     * semantics are preserved: int = low 16 bits, long = low 32 bits,
     * overflow if any of the high 32 bits are set.
     */
    {
        register char *nb;
        long hi, lo;
        int k;

        nb = (char *) &n;
        hi = lo = 0;
        for (k = 0; k < 4; k++) hi = (hi << 8) | (nb[k]     & 0xFF);
        for (k = 4; k < 8; k++) lo = (lo << 8) | (nb[k]     & 0xFF);

        if (hi) return 0;                   /*  overflow (> 32 bits)  */
        if (curch=='l' || curch=='L') {           /* explicitly long */
            getch();
            *np.lp = lo;
            return LONG;
        } else if (lo & 0xffff0000) {       /*  implicitly long (> 16 bits)  */
            *np.lp = lo;
            return LONG;
        } else {                            /*  int  */
            *np.ip = lo;
            return INT;
        }
    }
}

#ifndef SPLIT
double scale(double n, int power, int esign)
{
        double scale0();

        if (power > 9)
                n = scale0(n,(power / 10) + 9,esign);

        return scale0(n, power % 10, esign);
}

double scale0(double n, int power, int esign)
{

        if (power) {
                double *p;

                p = atoftbl;
                p += power;
                return (esign ? n * *p
                              : n / *p);
        }
        return n;
}
/*
char atoftbl[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,
  0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x84,
  0x48,0x00,0x00,0x00,0x00,0x00,0x00,0x87,
  0x7A,0x00,0x00,0x00,0x00,0x00,0x00,0x8A,
  0x1C,0x40,0x00,0x00,0x00,0x00,0x00,0x8E,
  0x43,0x50,0x00,0x00,0x00,0x00,0x00,0x91,
  0x74,0x24,0x00,0x00,0x00,0x00,0x00,0x94,
  0x18,0x96,0x80,0x00,0x00,0x00,0x00,0x98,
  0x3E,0xBC,0x20,0x00,0x00,0x00,0x00,0x9B,
  0x6E,0x6B,0x28,0x00,0x00,0x00,0x00,0x9E,
  0x15,0x02,0xF9,0x00,0x00,0x00,0x00,0xA2,
  0x2D,0x78,0xEB,0xC5,0xAC,0x62,0x00,0xC3,
  0x49,0xF2,0xC9,0xCD,0x04,0x67,0x4F,0xE4,
};
*/

double atoftbl[] = {
        1,
        10,
        100,
        1000,
        10000,
        1.000000e+05 ,
        1.000000e+06,
        1.000000e+07,
        1.000000e+08,
        1.000000e+09,
        1.000000e+10,
        1.000000e+20,
        1.000000e+30,
};
#endif
#else
int number(register long *np)
{
       long n;
       register char *cp;

       n=0;
       cp= &n;

       if (curch=='0') {
               long n1;
               int *ip;

               getch();

               n1=0;
               if (curch=='x' || curch=='X'){
                       int c;
                       getch();
                       while(c=ishex(curch)) {
                               n1=(n1<<4)+ (c- (c>='A' ? 'A'-10 : '0'));
                               getch();
                       }
               }
               else while(isoct(curch)) {
                       n1=(n1<<3)+(curch-'0');
                       getch();
               }

               ip= &n1;
               if(curch=='L' || curch=='l') {
                       getch();
                       goto retlong;
               }

               if(*ip==0) {
                       *(int *)np=n1;
                       return INT;
               }

       retlong:
               *np = n1;
               return LONG;
       }

       /* here all types still possible; collect in 4 byte integer */
       while(isdigit(curch)) {
                n = (n * 10) + (curch - '0');
                getch();
       }

       if(curch=='l' || curch=='L') { /* explicitly long */
               getch();
       longint:
               *np = n;
               return LONG;
       }
       else if(cp[0] || cp[1]) goto longint; /* implicitly long */

       else { /* int */
               *(int *)np = (int)n;
               return INT;
       }
}
#endif

void pstr(void)
{
        getch();

        if (curch=='\\')
                symval=dobslash();

        else {
                symval=curch;
                getch();
        }

        if (curch!='\'')
                error("unterminated character constant");
        else
                getch();
}

static direct FILE *sfile;
extern direct int datstring;

void qstr(void)
{
#ifdef MKIL
        /* code string (datstring 0) and string-pointer data initialisers
           (datstring 2): pass 1 sends body + a temp-id; pass 2 assigns the real
           label (so all label numbering stays in pass 2).  datstring 1 is a
           char-array initialiser - chars go inline, handled by the old path. */
        if (datstring != 1) {
                extern FILE *code;

                symval = strtempid++;
                ilwrec(IL_STR); ilwint(symval); ilweol();
                sfile = code;
                stringlen = 0;
                getch();
                while (curch != '"') {
                        if ((lptr-line) == 0) { error("unterminated string"); break; }
                        if (curch == '\\') oc(dobslash());
                        else { oc(curch); getch(); }
                }
                oc('\0');
                putc('\n', sfile);
                scount = 0;
                getch();
                ilwrec(IL_ENDRAW); ilweol();
                return;
        }
#endif
        switch(datstring) {
                case 0:
                case 2:
                        if(strfile == NULL) {
                                if((strfile = fopen(strname,"w+")) == NULL)
                                        fatal("can't open strings file");
                        }
                        sfile = strfile;
                        break;
                case 1:
                        sfile = code;
        }

        stringlen = 0;
        if(datstring != 1) {
#ifdef  SPLIT
                putc('l',sfile);
                putw(symval = getlabel(),sfile);
#else
                fprintf(sfile,"%c%d",UNIQUE,symval = getlabel());
#endif
        }
        getch();

#ifdef  SPLIT
        putc('s',sfile);
#endif
        while(curch!='"') {
                if ((lptr-line)==0) {
                        error("unterminated string");
                        break;
                }

                if (curch=='\\')
                        oc(dobslash());
                else {
                        oc(curch);
                        getch();
                }
        }
#ifdef  SPLIT
        putc('\0',sfile);
        ++stringlen;
#else
        oc('\0');
        putc('\n',sfile);
        scount = 0;
#endif
        getch();

}

void oc(int c)
{
#ifdef  SPLIT
        if(c == '\0' || c == '\\')
                putc('\\',sfile);
        putc(c,sfile);
#else
/*
        fprintf(sfile,"%s%d",scount ? "," : " fcb ",c);
        if(scount++ == 16) {
                putc('\n',sfile);
                scount = 0;
        }
*/
     if(!scount)
          fprintf(sfile," fcc \"");
     if(c < 0x20 || c == '"') {    /* speedy (c is not printable || c is " */
          fprintf(sfile,"\"\n fcb $%x\n",c);
          scount=0;
     }
     else {
          putc(c,sfile);
          if(scount++ >= 75) {
               fprintf(sfile,"\"\n");
               scount=0;
          }
     }
#endif
        ++stringlen;
}

void oz(int n)
{
#ifdef SPLIT
     passthru();
#endif
     fprintf(sfile," rzb %d\n",n);
}

int dobslash(void)
{
        register int c,n;

        getch();
        c=curch;
        getch();

        switch(c) {
                case 'n': return NEWLINE;
                case 'l': return LINEFEED;
                case 't': return TAB_CHAR;
                case 'b': return BACKSPACE;
                case 'v': return VTAB_CHAR;
                case 'r': return RET_CHAR;
                case 'f': return FORMFEED;
                case '\n':return ' ';
        }

        if (c=='x') {
                int c1;

                c = n = 0;
                while((c1 = ishex(curch)) && n++ < 2) {
                        c =(c<<4)+ (c1 < 'A' ? c1-'0' : c1-'A'+10);
                        getch();
                }
        }
        else if (c=='d') {
                c=n=0;
                while(isdigit(curch) && n++<3) {
                        c= c*10 + curch - '0';
                        getch();
                }
        }

        else if (isoct(c)) {
                c -= '0';
                n=0;
                while(isoct(curch) && n++<3) {
                        c = (c << 3) + (curch - '0');
                        getch();
                }
        }

        return c;
}

int isoct(int c)
{
        return (c<='7' && c>='0');
}

int ishex(int c)
{
        return ( isdigit(c) ||  ((c &=0x5f) >='A' && c<='F')) ? c : 0;
}

static double normaliz(char n[])
{
    /*
     * n[] is the parsed digits as an 8-byte big-endian integer (n[0] most
     * significant), built by addin().  Sum it into a double byte by byte so it
     * is independent of host `long` width - the original `n[0]*2^32 + n[1]`
     * aliased n as two 32-bit longs, which over-reads on a 64-bit host (where
     * long is 64-bit), producing garbage.
     */
    register int i;
    double v;

    v = 0.0;
    for (i = 0; i < 8; i++) v = v * 256.0 + (n[i] & 0xFF);
    return v;
}

int addin(register char n[], int c)
{
    register int i,x;
    char ntemp[8];

    if (numshf(n)) return 1;
    for (i = 7; i > 0; --i) ntemp[i] = n[i];
    if (numshf(n)) return 1;
    if (numshf(n)) return 1;
    x = c - '0';
    for (i = 7; i > 0; --i) {
        n[i] = x = (n[i] & 0xFF) + (ntemp[i] & 0xFF) + x;
        x >>= 8;
    }
    return x;
}

static int numshf(register char n[])
{
    register int i, x = 0;

    for (i = 7; i > 0; --i) {
        n[i] = x = ((n[i] & 0xFF) << 1) | x;
        x >>= 8;
    }
    return x;
}

