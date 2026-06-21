/*
    Modification history for cmain.c:
        25-May-83       Make *p be the register variable.
        24-Apr-84 LAC   Conversion for UNIX.
*/

#define MAIN
#include "cj.h"
#include "il.h"
#include <signal.h>

char *dumname = sdummy.sname;

char strname[] = "cstr.XXXXX";

#ifdef  SPLIT
direct char *infile;

direct int eflag;
#endif

#ifndef PASS2
/* pass1 only */
direct  int     *inclptr;       /* include stack pointer */
#endif      /* <<<- inserted to match preceeding "ifndef PASS2" */

main(int argc, char **argv)
{
        register char *p;

        int tidy();

#ifdef UNIX
        signal(2,tidy);
#else
        intercept(tidy);
#endif
        mktemp(strname);

        sdummy.type = INT;
        sdummy.size = 2;
        sdummy.dimptr = NULL;
        sdummy.offset = 0;
        sdummy.storage = EXTDEF;
        strcpy(dumname,"_dummy_");

        lexinit();
        in = stdin;
        code = stdout;

        while (--argc>0 ) {
                if (*(p= *++argv)=='-') {
                        while(*++p) switch(*p ) {
#ifdef  SPLIT
                            case 'e':
                                eflag = 1;
                                break;
#endif
                            case 's':
                                sflag = 1; break;
                            case 'n':
                                ++nocode; break;
#ifdef  PTREE
                            case 'd':
                                dflag = 1;
                                break;
#endif
                            case 'o':
                                if(*++p=='=') {
                                    if((code=fopen(++p,"w"))==NULL) {
                                        fprintf(stderr,"can't open %s\n",p);
                                        errexit();
                                    }
                                }
                                goto done;
#ifdef PROF
                            case 'p':
                                   pflag=1;
                                   break;
#endif
                            default:
                                fprintf(stderr,"unknown flag : -%c\n",*p);
                                errexit();
                        }
                        done: ;
                }
                else {
                        if(tflag)
                                continue;       /* have we one open already? */
                        ++tflag;
                        /* fopen, not freopen(...,stdin): on OS-9 freopen's
                           fclose(stdin)+reuse of path 0 leaves `in` reading the
                           console (getc blocks -> hang).  A fresh stream works
                           (same as c3comp2's cmain2.c), and an unused stdin slot
                           is harmless given the small iob table. */
                        if((in = fopen(*argv,"r")) == NULL)
                                fatal("can't open input file");
#       ifdef   SPLIT
                        infile = *argv;
#       endif
                }
        }


#ifdef  PTREE
        if(dflag)
                getkeys();
#endif

#ifdef MKIL
        ilwinit();
#endif
        preinit();
        getsym();
        while(sym!=EOF)
                extdef();          /* go do it!!! */

#ifndef MKIL
        epilogue();
#endif

        if(ferror(code))
                fatal("error writing assembly code file");
        fflush(code);       /* the output stream is `code` (stdout, or the -o file) */

        if (errcount) {
                fprintf(stderr,"errors in compilation : %d\n",errcount);
                errexit();
        }
}

errexit()
{
#ifdef  SPLIT
                if(infile)
                        unlink(infile);
#endif
                exit(1);
}
