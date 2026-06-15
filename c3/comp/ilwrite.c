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

extern FILE *code;

ilwinit()
{
    fprintf(code,"%s %d\n",IL_MAGIC,IL_VERSION);
}

ilwrec(op)
int op;
{
    fprintf(code,"%d",op);
}

ilwint(n)
int n;
{
    fprintf(code," %d",n);
}

ilwname(s)
char *s;
{
    fprintf(code," %s",s);
}

ilweol()
{
    putc('\n',code);
}
