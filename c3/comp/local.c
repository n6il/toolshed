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
#include "il.h"
#include <errno.h>

#define GBLB ':'    /* global asm declarator */
#define LCLB ' '    /* local asm declarator */

static direct unsigned stklab;       /* label for stack check equ value */

void epilogue(void)
{
     dumpstrings();
     endsect();
     if(errcount)
          ol("fail source errors");
}

void locstat(int l, int size, int area)
{
#ifdef MKIL
     ilwrec(IL_LBSS);
     ilwint(l);            /* the static's label (a pass-1 temp-id) */
     ilwint(size);
     ilwint(area);
     ilweol();
#else
     vsect(area);
     olbl(l);
     fprintf(code," rmb %d\n",size);
     endsect();
#endif
}

void defglob(symnode *ptr, int size, int area)
{
#ifdef MKIL
     ilwrec(IL_BSS);
     ilwname(ptr->sname);
     ilwint(size);
     ilwint(1);            /* global: label gets a colon */
     ilwint(area);
     ilweol();
#else
     vsect(area);
     defvar(ptr,size,GBLB);
     endsect();
#endif
}

void extstat(symnode *ptr, int size, int area)
{
#ifdef MKIL
     ilwrec(IL_BSS);
     ilwname(ptr->sname);
     ilwint(size);
     ilwint(0);            /* file-local static: no colon */
     ilwint(area);
     ilweol();
#else
     vsect(area);
     defvar(ptr,size,LCLB);
     endsect();
#endif
}

void defvar(symnode *ptr, int size, int scope)
{
     fprintf(code,"%.8s%c rmb %d\n",ptr->sname,scope,size);
}

#ifdef PROF
void profname(char *name, int lab)
{
     olbl(lab);
     fprintf(code," fcc \"%.8s\"\n fcb 0\n",name);
}
#endif

#ifdef PROF
void startfunc(register char *name, int flag, int paramreg, int lab)
#else
void startfunc(register char *name, int flag, int paramreg)
#endif
{
#ifdef FUNCNAME
    extern direct int fnline;
#endif
#ifdef MKIL
    ilwrec(IL_FUNC);
    ilwname(name);
    ilwint(flag);
    ilwint(paramreg);
    ilwint(ftype);
    ilweol();
#else
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
#endif
}

void endfunc(void)
{
#ifdef MKIL
    ilwrec(IL_ENDFUNC);
    ilweol();
#else
        /* generate stack reservation value */
     if(!sflag)
          fprintf(code,"_%d equ %d\n\n",stklab,maxpush-callflag-64);
#endif
}

void defbyte(void)
{
        ot("fcb ");
}

void defword(void)
{
        ot("fdb ");
}

void comment(void)
{
        os("* ");
}

void vsect(int area)
{
     ol(area ? "vsect dp" : "vsect");
}

void endsect(void)
{
     ol("endsect");
}

void dumpstrings(void)
{
     register int c;

     if(strfile) {
          /* Close (flushing the writes) then reopen for reading, rather than
             rewind()-ing the "w+" stream in place: OS-9 RBF does not reliably
             read a file back after a write+rewind on the same open path, so
             update-mode read-after-write silently lost the whole string pool on
             the target.  A fresh read open after the close is robust on host and
             target alike (output is identical either way). */
          fclose(strfile);
          if((strfile = fopen(strname,"r")) == NULL)
               fatal("can't read strings file");
          while((c = getc(strfile)) != EOF)
               putc(c,code);
          fclose(strfile);
          unlink(strname);
     }
}

void tidy(void)
{
     int err = errno ? errno : 1;

     if(strfile) {
          fclose(strfile);
          unlink(strname);
     }
     _exit(err);
}
