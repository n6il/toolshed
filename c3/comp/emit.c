/*
 * emit.c - low-level 6809 assembly / data emitter primitives.
 *
 * SHARED by both halves of the two-pass split: compiled into pass 1 (c3comp1),
 * pass 2 (c3comp2), and the single-pass compiler, exactly like treeutil.c.
 * Pass 1 needs these to emit initialised data for the IL_RAW passthrough
 * (inits.c: initialise/defobject/datdef/ilist/zero -> deref/defcon/deflong/
 * defbytes + the ot/od/nl/... primitives); the code generator (codgen.c, which
 * is pass-2/single-pass only) uses them too.  Split out of codgen.c so pass 1
 * no longer links the heavy expression/statement generator.
 *
 * All functions and the datflag flag are moved here VERBATIM from codgen.c -
 * pure code motion, so generated output is unchanged.  addoff is de-static'd
 * (deref calls it here; gen calls it from codgen.c).  The yind/spind index
 * strings are duplicated in codgen.c (both files use them; keeping each copy
 * static is simpler than a cross-file extern for a 3-byte string).
 */

#include "cj.h"

direct int datflag;

/* no advantage putting the following ptrs on dp (ptrs always access indexed)*/
static char *yind = ",y";
static char *spind = ",s";

addoff(register int offset)
{
    if(offset) {
        if(offset > 0) ob('+');
        od(offset);
    }
}

/* val is an expnode * in NODE mode, an int otherwise - hence argword, not long. */
deref(int arg, argword val, int offset)
{
    register expnode *node;
    register symnode *sn;
    int sc;

    if (arg & INDIRECT) ob('[');

    switch (arg & NOTIND) {
        case NODE:
            node = (expnode *)val;
            if (node->op == AMPER) {
                ob('#');
                deref(NODE,(argword)node->left,offset);
                return;
            }
            deref(node->op,(argword)node->val.sp,offset+node->modifier);
            return;
        case CONST:
            ob('#');
            od(val);
            break;
        case FREG:
            os("_flacc");
            addoff(offset);
            os(yind);
            break;
        case NAME:
            if (sn = (symnode *)val)
                switch (sc = sn->storage) {
                    case AUTO:
                        od(val = sn->offset-sp+offset);
                        os(spind);
                        break;
                    case STATICD:
                        if(!datflag) ob((arg & INDIRECT) ? '>' : '<');
                    case STATIC:
                        olbl(sn->offset);
                        goto dooff;
                    case DIRECT:
                    case EXTERND:
                        if(!datflag) ob((arg & INDIRECT) ? '>' : '<');
                    case EXTERN:
                    case EXTDEF:
                        on(sn->sname);
dooff:
                        addoff(offset);
                        if(arg & INDIRECT || (!datflag && (sc != DIRECT
                                    && sc != EXTERND && sc != STATICD))) {
                            if(isftn(sn->type)) os(",pcr");
                            else os(yind);
                        }
                        break;
                    default:
                        error("storage error");
                        break;
                }
            else od(offset);
            break;
        case XIND:
        case YIND:
        case UIND:
            od(val += offset);
            ob(',');
            switch(arg & NOTIND) {
                case XIND:  ob('x'); break;
                case YIND:  ob('y'); break;
                case UIND:  ob('u'); break;
            }
            break;
        case STACK:
            os(spind);os("++");
            sp += 2;
            break;
        default:
            error("dereference");
    }

    if(arg & INDIRECT) ob(']');
}

defcon(register int *p, int n)
{
    register int i;

    defword();
    if (n == 1) od(p);
    else if (p == NULL) {
        for (i = 1; i++ < n; ) os("0,");
        ob('0');
    } else
        for (i = 0; i < n; ++i){
            od(*p++);
            if (i != n - 1) ob(',');
        }
    nl();
}

/*
 * Emit long / float / double constants as TARGET (6809, big-endian) 16-bit
 * words from the HOST-format value.  The generic defcon() above reads the
 * constant as int-sized words: right on the 6809 (int = 16 bits) but wrong on
 * a 64-bit host (int = 32 bits) - it over-reads and mis-orders the bytes, so
 * float/double/long constants came out as garbage.  These helpers are
 * host-int-size- and host-endianness-independent.
 *
 * float/double are emitted as IEEE big-endian (the host `double` is IEEE
 * binary64, the same representation the MC6839 FP package uses).  long is
 * emitted as the low 32 bits, high word first.  Both also stay correct when c3
 * eventually self-hosts on the 6809 (big-endian, int = 16 bits): defbytes()
 * then emits the bytes in source order, deflong() the two 16-bit halves.
 */
hostbig()                       /* nonzero if the host is big-endian */
{
    int probe;

    probe = 1;
    return *((char *) &probe) == 0;
}

defbytes(p,nbytes)              /* host-order float/double bytes -> BE words */
char *p;
{
    register int i, hi, lo;

    defword();
    for (i = 0; i < nbytes; i += 2) {
        if (hostbig()) { hi = p[i]          & 0xFF; lo = p[i+1]        & 0xFF; }
        else           { hi = p[nbytes-1-i] & 0xFF; lo = p[nbytes-2-i] & 0xFF; }
        od((hi << 8) | lo);
        if (i + 2 < nbytes) ob(',');
    }
    nl();
}

deflong(p)                      /* host long's low 32 bits -> 2 BE words */
char *p;
{
    long v;

    v = *((long *) p);
    defword();
    od((int) (v >> 16));
    ob(',');
    od((int) v);
    nl();
}

ot(char *s)
{
    putc(' ',code);
    os(s);
}

ol(char *s)
{
    ot(s);
    nl();
}

nl()
{
    putc('\n',code);
}

ob(int b)
{
    putc(b,code);
}

os(char *s)
{
    fprintf(code,"%s",s);
}

od(int n)
{
    /* Emit as a 16-bit value.  Cast to INTTYPE (16-bit on host and target) so a
       negative operand prints the SAME on both: `n & 0xFFFF` left it positive on
       the 32-bit host (e.g. 65534) but signed on the 16-bit target (-2) - same
       bit pattern to the assembler, but it broke host/target text identity. */
    fprintf(code,"%d",(INTTYPE)n);
}

label(int n)
{
    olbl(n);
    lastst = 0;
    nl();
}

olbl(int n)
{
    ob(UNIQUE);
    od(n);
}

on(char *s)
{
    fprintf(code,"%.8s",s);
}

modstk(int nsp)
{
    int x;

    if (x=nsp-sp) {
        ot("leas ");
        od(x);
        os(spind);
        nl();
    }
    return nsp;
}

nlabel(char *ptr, int scope)
{
    on(ptr);
    if (scope) ob(':');
    nl();
}
