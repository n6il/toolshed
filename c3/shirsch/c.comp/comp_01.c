/* ******************************************************** *
 * comp_01.c - first module for c.comp                      *
 *                                                          *
 * from pass1_11.c - contains main()                        *
 *                                                          *
 * $Id:: comp_01.c 61 2008-09-19 03:09:17Z dlb            $ *
 *                                                          *
 * ******************************************************** */

/* Cause "pass1.h" to not define variables "extern"*/
#define MAIN

#include "ccomp.h"
#include <stdlib.h>

int
#ifdef __STDC__
main (int argc, char **argv)
#else
main (argc, argv)
    int argc;
    char **argv;
#endif
{
    /* Initialization routines */

#ifdef COCO
    intercept (quitcc);
    mktemp (CstrTmp);
#endif
#ifdef _OSK
    intercept (quitcc);
    mktemp (CstrTmp);
#endif
    strcpy (DummyNm, "_dummy_");
    tblsetup ();
    InPath = stdin;
    OutPath = stdout;

    /* Process arguments */

    while (--argc > 0)                  /*  L0295 */
    {
        register char *argstr;

        if (*(argstr = *(++argv)) == '-')   /* L01b7 */    /* else L0265 */
        {
            while (*(++argstr) != '\0')   /*  L0257 */
            {
                switch (*argstr)
                {
                    case 's': /* L01d2 */
                        NoStkck = 1;
                        break;
                    case 'n': /* L01da */
                        ++ModName;
                        break;
                    case 'o': /* L01e4 */
                        if (*(++argstr) == '=')
                        {
                            if ( ! (OutPath = fopen (++argstr,"w")))
                            {
                                fprintf (stderr, "can't open %s\n", argstr);
                                quit_cc();
                            }
                        }

                        goto L0261;
                    case 'p': /* L0219 */
                        DoProfil = 1;
                        break;
                    default:    /* L0220 */
                        fprintf (stderr, "unknown flag : -%c\n", *argstr);
                        quit_cc();
                        break;
                }
            }       /* end while *argv != 0 */

L0261:
            continue;
        }
        else
        {
            if ( ! D0052)         /* L0295 */
            {
                ++D0052;
                if ( ! (InPath = freopen (*argv, "r", stdin)))
                {
                    err_quit ("can't open input file");
                }
            }
        }
    }       /* end while argc != 0 */

    initbuf0 ();
    nxt_word ();

    /* Here is where all file reading and output occurs */

    while (D003f != -1)
    {
        funcmain ();
    }

    endprog ();      /* dump strings and write endsect for prog */

    if (ferror (OutPath))
    {
        err_quit ("error writing assembly code file");
    }

    fflush (stdout);

    if (ErrCount)
    {
        fprintf ( stderr, "errors in compilation : %d\n", ErrCount);
        quit_cc();
    }
#ifndef COCO
    return 0;
#endif
}

void 
#ifdef __STDC__
quit_cc (void)
#else
quit_cc()
#endif
{
    exit (1);
}
