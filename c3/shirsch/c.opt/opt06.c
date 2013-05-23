/* ******************************************************************** *
 * opt06.c - part 6 for c.opt source                                    *
 *                                                                      *
 * $Id:: opt06.c 52 2008-08-26 14:59:09Z dlb                         $  *
 * ******************************************************************** */

/* We won't #include copt.h (and subsequently proto.h) since the needs
 * are so small */

#include <stdio.h>
#include <stdlib.h>

#ifdef COCO
#   define void int
#endif

/* NOTE: we could avoid this prototype if we placed errexit() first */
void 
#ifdef __STDC__
errexit (char *pmpt, char *parm2);
#else
errexit ();
#endif

void *
#ifdef __STDC__
add_mem (int memreq)
#else
add_mem (memreq)
    int memreq;
#endif
{
    void *memaddr;

#ifdef COCO
    if ((memaddr = sbrk (memreq)) == -1)
    {
        errexit ("memory overflow");
    }
#else
        /* sbrk under OSK gets produces "memory oveflow" error */
#  ifdef _OSK
    if ((memaddr = ibrk (memreq)) == -1)
    {
        errexit ("memory overflow");
    }

#   else
    if ( ! (memaddr = calloc (1, memreq)))
    {
        errexit ("memory overflow", 0);
    }
#   endif
#endif

    return memaddr;
}

/* ******************************************************************** *
 * errexit () - Print error message and exit                            *
 * NOTE: Original version provided for format string + 3 vars.  None of *
 *          the calls asked for more than one var besides fmt string    *
 *          so for non-COCO, we only use one.                           *
 *          Except for testing and comparing, this could be reduced     *
 *          for the COCO also                                           *
 * ******************************************************************** */

void
#ifdef __STDC__
errexit (char *pmpt, char *parm2)
#else
errexit (pmpt, parm2, parm3, parm4)
    char *pmpt;
    int parm2, parm3;
    char *parm4;
#endif
{
    fprintf (stderr, "C optimiser error: ");
    fprintf (stderr, pmpt, parm2
#ifdef COCO
            , parm3, parm4
#endif
            );
    putc ('\n', stderr);
    exit (1);
}
