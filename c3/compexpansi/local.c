/*
     Modification history for local.c:
          24-May-83      Added direct page initializers.
                         Make dumpstrings use fopen() update mode.
                         Made stklab be DIRECT.
          25-May-83      Make better use of register variables.
          08-Jun-83      Profile option sometimes output garbage
                         to the asm file if function name was exactly 8
                         chars.
          09-Jun-83      Made tidy() exit with signal code.
          18-Aug-83      Add conditional to dump function names to stdout
          01-Sep-83      Put "fail" directive in asmb output for errors
*/

#include "cj.h"
#include <stdlib.h>

#define GBLB ':'    /* global asm declarator */
#define LCLB ' '    /* local asm declarator */

static direct unsigned stklab;       /* label for stack check equ value */
static void defvar(symnode *ptr, int size, char scope);

void epilogue(void)
{
     dumpstrings();
     endsect();
     if(errcount)
          ol("fail source errors");
}

void locstat(l,size,area)
{
     vsect(area);
     olbl(l);
     fprintf(code," rmb %d\n",size);
     endsect();
}

void defglob(symnode *ptr, int size, int area)
{
     vsect(area);
     defvar(ptr,size,GBLB);
     endsect();
}

void extstat(symnode *ptr, int size, int area)
{
     vsect(area);
     defvar(ptr,size,LCLB);
     endsect();
}

static void defvar(symnode *ptr, int size, char scope)
{
     fprintf(code, "%.8s%c rmb %d\n", ptr->sname, scope, size);
}

#ifdef PROF
void profname(name,lab)
char *name;
{
     olbl(lab);
     fprintf(code," fcc \"%.8s\"\n fcb 0\n",name);
}
#endif

#ifdef PROF
void startfunc(name,flag,paramreg,lab)
#else
void startfunc(name,flag,paramreg)
#endif
register char *name;
int paramreg;
{
#ifdef FUNCNAME
    extern direct int fnline;
#endif
    fprintf(code," ttl %.8s\n",name);
#ifdef FUNCNAME
    printf("%-10.10s %-8.8s %5d\n",filename,name,fnline);
#endif
    nlabel(name,flag);

    /* push possible register variables */
    if (paramreg == DREG) ol("pshs d,u");
    else ol("pshs u");
    if (paramreg == UREG) ol("tfr d,u");
    if (!sflag)
        fprintf(code," ldd #_%d\n lbsr _stkcheck\n",stklab=getlabel());

#ifdef PROF
     if(pflag) {
          ot("leax ");
          olbl(lab);
          os(",pcr\n pshs x\n leax ");
          on(name);
          os(",pcr\n pshs x\n lbsr _prof\n leas 4,s\n");
     }
#endif
}

void endfunc()
{
        /* generate stack reservation value */
     if(!sflag)
          fprintf(code,"_%d equ %d\n\n",stklab,maxpush-callflag-64);
}

void defbyte()
{
        ot("fcb ");
}

void defword()
{
        ot("fdb ");
}

void comment()
{
        os("* ");
}

void vsect(int area)
{
     ol(area ? "vsect dp" : "vsect");
}

void endsect()
{
     ol("endsect");
}

/*
dumpstrings()
{
     register int c;

     if(strfile) {
          fclose(strfile);
          if((strfile=fopen(strname,"r")) == NULL)
               fatal("can't read strings file");
          while((c = getc(strfile)) != EOF)
               putc(c,code);
          fclose(strfile);
          unlink(strname);
     }
}
*/
void dumpstrings()
{
     register int c;

     if(strfile) {
          rewind(strfile);
          while((c = getc(strfile)) != EOF)
               putc(c,code);
          if(ferror(strfile)) fatal("dumpstrings");
          fclose(strfile);
          unlink(strname);
     }
}

int tidy(void)
{
     int err = errno ? errno : 1;

     if(strfile) {
          fclose(strfile);
          unlink(strname);
     }
     _exit(err);
}
