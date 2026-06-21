/*
 * cmain2.c - c3comp2 (back end) driver.
 *
 * Reads the intermediate language emitted by c3comp1 (see il.h) and generates
 * 6809 assembly, reusing the existing back end (optim type-check, tranexp,
 * codgen, local).  This is the consumer half of the two-pass split; it owns no
 * parser and no symbol table - every NAME leaf in the IL carries the type/size/
 * storage/offset it needs, and a throwaway symnode is synthesised per leaf so
 * the unchanged codegen can dereference node->val.sp as before.
 *
 * Expression trees arrive in POSTFIX order, rebuilt here with an operand stack;
 * statements arrive as structured records so this pass owns all label/jump
 * generation.  WALKING SKELETON: currently functions whose bodies are just
 * return statements over int expressions.  Grows from here.
 */

#define MAIN
#include "cj.h"
#include "il.h"

extern char *grab();
extern expnode *treecopy();
extern direct expnode *dregcont, *xregcont;

char *dumname = sdummy.sname;
char strname[] = "cstr.XXXXX";

extern int dostmt();            /* statement / control-flow handler (cmain2c.c) */

/* Per-function cache of synthesised symnodes, SHARED with cmain2b.c (the
 * IL-read/symbol-synthesis layer that populates it: mksym/relsyms live there).
 * References to the SAME variable must share one symnode: the register-content
 * tracker (cmptrees) compares the symbol pointer (val.num), so distinct symnodes
 * for the same var would defeat it and force redundant reloads.  Reset at each
 * IL_FUNC.  Defined here (allocated in c2init); cmain2b.c sees these extern. */
#define MAXSYMS 256             /* max distinct symbols interned per function;
                                   cmain2.c's main() needs 220 (overflow is fatal,
                                   not a silent drop - see mksym) */
symnode **symcache;             /* [MAXSYMS], heap-allocated (see c2init) */
int nsyms;
symnode *symfree;               /* reclaimed synthesised symnodes (link: snext) */
dimnode *dimfree;               /* reclaimed dim-chain nodes      (link: dptr)  */

#define MAXSTR 192              /* string temp-id -> label map (pass 2 owns labels) */
int *strmap;                    /* [MAXSTR], heap-allocated (see c2init) */

/* count of local statics in the current function (the temp-id->label arrays
 * live with statlabel/statdef in cmain2b.c); reset per function by IL_FUNC. */
int nstat;

/* Per-function cache of user goto-label numbers (keyed by name), SHARED with
 * cmain2b.c's labelnum.  Pass 2 owns ALL label numbering, so user labels can
 * never collide with the if/loop labels it allocates.  labnames is allocated in
 * c2init and defined here; the number array labnums lives with labelnum. */
#define MAXLABS 12
char *labnames;                 /* [MAXLABS][IL_NAMEMAX] flat, heap (see c2init) */
int nlabs;

/* Allocate the IL-rebuild tables on the heap.  They need no static storage, and
 * keeping them off the bss lets c3comp2's static data area fit in a single 8 KB
 * block - required for OS-9 Level-2 F$Fork to load it (a 2-block data area makes
 * the task 7 blocks, leaving too few free to fork).  Full capacity is preserved;
 * only strmap is zeroed, to keep its former bss behaviour (an unset string id
 * reads back as label 0). */
static c2init()
{
    register int i;

    symcache = (symnode **)   grab(MAXSYMS * sizeof(symnode *));
    strmap   = (int *)        grab(MAXSTR  * sizeof(int));
    labnames = (char *)       grab(MAXLABS * IL_NAMEMAX);
    for (i = 0; i < MAXSTR; i++) strmap[i] = 0;
}

main(argc,argv)
char **argv;
{
    int op;
    char *nm;
    int flag, paramreg, dp;
    register char *p;

    c2init();
    c2cinit();          /* allocate cmain2c.c's control-flow frame stacks */
    in = stdin;
    code = stdout;

    /* Optional args: a bare name is the input IL file (read instead of stdin),
       -o=FILE redirects the output.  The OS-9 pipeline needs this: its
       per-process memory forbids piping (one pass at a time, intermediate
       files), and stdin redirection is unreliable on this runtime, so pass 2
       takes its IL as a file argument like pass 1 does.  No args -> stdin/stdout
       (the host `c3comp1 | c3comp2` pipe is unchanged). */
    while (--argc > 0) {
        if (*(p = *++argv) == '-') {
            if (p[1] == 'o' && p[2] == '=') {
                if ((code = fopen(p+3,"w")) == NULL)
                    fatal("can't open output file");
            } else if (p[1] == 's' && p[2] == '\0')
                sflag = 1;
            else
                fatal("c3comp2: unknown flag");
        } else if ((in = fopen(p,"r")) == NULL)
            fatal("can't open input file");
    }

    /* the codegen references sdummy as a placeholder symbol */
    sdummy.type = INT;
    sdummy.size = 2;
    sdummy.dimptr = NULL;
    sdummy.offset = 0;
    sdummy.storage = EXTDEF;
    strcpy(sdummy.sname,"_dummy_");
    mktemp(strname);

    ilrinit();
    while ((op = ilrrec()) != ILEOF) {
        switch (op) {
            case IL_PSECT:                      /* name, dp-size */
                nm = ilrname();
                dp = ilrint();
                fprintf(code," psect %s,0,0,%d,0,0\n nam %s\n",nm,dp,nm);
                break;
            case IL_BSS: {                      /* name, size, scope, area */
                int sz, scope, area;

                nm = ilrname();
                sz = ilrint();
                scope = ilrint();
                area = ilrint();
                vsect(area);
                fprintf(code,"%.8s%c rmb %d\n", nm, scope ? ':' : ' ', sz);
                endsect();
                break;
            }
            case IL_LBSS: {                     /* local static: temp-id, size, area */
                int tid, sz, area, N;

                tid = ilrint();
                sz = ilrint();
                area = ilrint();
                N = statdef(tid);               /* pass 2 owns the static's label */
                vsect(area);
                olbl(N);
                fprintf(code," rmb %d\n", sz);
                endsect();
                break;
            }
            case IL_RAW: {                      /* data asm until IL_ENDRAW, with
                                                   embedded IL_STR/IL_DWSTR records */
                register char *ln;
                int rop;

                while ((ln = ilrline()) != (char *) 0) {
                    if (ln[0] >= '0' && ln[0] <= '9') {  /* an embedded IL record */
                        rop = ilrint();        /* (ilp is at the line start) */
                        if (rop == IL_ENDRAW) break;
                        if (rop == IL_STR) { dostr(); continue; }
                        if (rop == IL_DWSTR) {  /* string pointer -> fdb _label */
                            fprintf(code," fdb %c%d\n", UNIQUE, strmap[ilrint()]);
                            continue;
                        }
                        if (rop == IL_LBL) {    /* local static's label site */
                            olbl(statdef(ilrint()));
                            nl();
                            continue;
                        }
                    } else {                    /* a plain data asm line (or blank) */
                        fputs(ln, code);
                        putc('\n', code);
                    }
                }
                break;
            }
            case IL_STR:                        /* string def at statement level */
                dostr();
                break;
            case IL_FUNC:                       /* name, flag, paramreg, ftype */
                nm = ilrname();
                flag = ilrint();
                paramreg = ilrint();
                ftype = ilrint();
                /* frame level after the prologue: a D-register first arg is
                 * pushed (pshs d), putting the frame base 2 below entry sp */
                callflag = 0;
                sp = stlev = (paramreg == DREG) ? -2 : 0;
                maxpush = (stlev < 0) ? stlev : 0;
                relsyms();              /* reclaim prev function's symnodes; nsyms = 0 */
                nlabs = 0;              /* fresh goto-label cache per function */
                nstat = 0;              /* fresh local-static label map per function */
                clrconts();
                startfunc(nm,flag,paramreg);
                lastst = 0;
                break;
            case IL_PARMOV: {                   /* register-param spill: reg, offset */
                int rg, of;

                rg = ilrint();
                of = ilrint();
                gen(rg, of);
                break;
            }
            case IL_ENDFUNC:
                if (lastst != RETURN) {
                    modstk(0);
                    gen(RETURN,0,0);
                }
                endfunc();
                break;
            default:
                if (!dostmt(op))    /* statement / control-flow records (cmain2c.c) */
                    fatal("IL: unexpected record at statement level");
        }
    }
    epilogue();

    if (ferror(code)) fatal("error writing assembly code file");
    fflush(code);       /* the output stream is `code` (stdout, or the -o file) */
    if (errcount) {
        fprintf(stderr,"errors in code generation : %d\n",errcount);
        exit(1);
    }
}
