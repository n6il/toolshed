/*
 * ilread.c - c3 intermediate-language codec, READ side (c3comp2).
 *
 * Textual decoder for the IL record stream (see il.h).  Reads from the
 * compiler's normal input (`in`), one newline-framed record at a time:
 * ilrrec() loads the next record and returns its opcode; ilrint()/ilrname()
 * pull that record's fields in order.  The ONLY place that knows the textual
 * format on the read side - a binary codec replaces it behind #ifdef IL_BINARY.
 */

#include <stdio.h>
#include "il.h"

#define direct          /* a no-op on every target now - see cj.h for why */
/* `in`/`code` are `global direct` in cj.h; this extern MUST match its qualifier
   exactly or the OS-9 codegen addresses it inconsistently (one site via the data
   base, the other via DP) - a wrong address that read garbage ("empty IL
   stream").  With `direct` neutralised everywhere they are plain Y-relative. */
extern direct FILE *in;
extern fatal();

static char ilbuf[IL_LINEMAX];          /* current record */
static char *ilp;                       /* parse cursor within ilbuf */
static char ilnamebuf[IL_NAMEMAX];      /* last name field returned */

/* read one newline-terminated record into ilbuf; 0 at end of stream */
static ilgetln()
{
    register int c;
    register char *d;

    d = ilbuf;
    if ((c = getc(in)) == EOF) return 0;
    while (c != EOF && c != '\n') {
        if (d < ilbuf + IL_LINEMAX - 1) *d++ = c;
        c = getc(in);
    }
    *d = '\0';
    ilp = ilbuf;
    return 1;
}

/* advance to the next field; NULL at end of record */
static char *iltok()
{
    while (*ilp == ' ' || *ilp == '\t') ilp++;
    if (*ilp == '\0') return (char *) 0;
    return ilp;
}

ilrinit()
{
    register char *m;

    if (!ilgetln()) fatal("empty IL stream");
    for (m = IL_MAGIC; *m; m++)
        if (*ilp++ != *m) fatal("not a c3 IL stream");
    if (ilrint() != IL_VERSION) fatal("IL version mismatch");
}

ilrrec()
{
    if (!ilgetln()) return ILEOF;
    return ilrint();
}

ilrint()
{
    register char *t;
    register int n, neg;

    if ((t = iltok()) == (char *) 0) fatal("IL: missing integer field");
    neg = 0;
    if (*t == '-') { neg = 1; t++; }
    n = 0;
    while (*t >= '0' && *t <= '9') n = n * 10 + (*t++ - '0');
    ilp = t;
    return neg ? -n : n;
}

/* read one raw line verbatim (for IL_RAW passthrough); NULL at end of stream */
char *ilrline()
{
    if (!ilgetln()) return (char *) 0;
    return ilbuf;
}

/* the current record's raw line text (used to pass non-record lines through) */
char *ilrbuf()
{
    return ilbuf;
}

char *ilrname()
{
    register char *t, *d;

    if ((t = iltok()) == (char *) 0) fatal("IL: missing name field");
    d = ilnamebuf;
    while (*t && *t != ' ' && *t != '\t')
        if (d < ilnamebuf + IL_NAMEMAX - 1) *d++ = *t++; else t++;
    *d = '\0';
    ilp = t;
    return ilnamebuf;
}
