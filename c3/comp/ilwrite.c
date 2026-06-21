/*
 * ilwrite.c - c3 intermediate-language codec, WRITE side (c3comp1).
 *
 * Textual encoding of the IL record stream (see il.h).  Records go to the
 * compiler's normal code output (the `code` stream), which c3comp1 pipes to
 * c3comp2.  Keep this the ONLY place that knows the textual format on the write
 * side, so a binary codec can replace it behind #ifdef IL_BINARY untouched.
 */

#include <stdio.h>
#include "il.h"

#define direct          /* a no-op on every target now - see cj.h for why */
/* `code` is `global direct` in cj.h; this extern MUST match its qualifier
   exactly or the OS-9 codegen addresses it inconsistently (data base vs DP) - a
   wrong address that made every fprintf(code,...) write through a garbage FILE*
   (c3comp1's wild-jump crash on real L2).  `direct` is now neutralised. */
extern direct FILE *code;

ilwinit()
{
    fprintf(code,"%s %d\n",IL_MAGIC,IL_VERSION);
}

ilwrec(int op)
{
    fprintf(code,"%d",op);
}

ilwint(int n)
{
    fprintf(code," %d",n);
}

ilwname(char *s)
{
    fprintf(code," %s",s);
}

ilweol()
{
    putc('\n',code);
}
