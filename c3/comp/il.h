/*
 * il.h - c3 two-pass intermediate language (textual, with a codec seam).
 *
 * Emitted by c3comp1 (front end: lex / declare / build / stats parsing) and
 * consumed by c3comp2 (back end: optim type-check, tranexp, codgen).  The split
 * exists so each binary fits OS-9 Level 2's ~64K per-process limit; see the
 * project notes for the boundary rationale (front-heavy: the heap-hungry
 * symbol-table side must have the smaller code).
 *
 * CODEC SEAM
 *   All IL input/output goes through the ilw_* (write, c3comp1) and ilr_* (read,
 *   c3comp2) primitives below - the parser and codegen never touch the byte
 *   stream directly.  The textual encoding here can therefore be swapped for a
 *   binary one later behind #ifdef IL_BINARY with no change to the front or back
 *   ends; only ilwrite.c / ilread.c change.  Both passes MUST be built with the
 *   same setting - ilwinit writes a magic+version header that ilrinit checks,
 *   so a mismatched pair fails loudly instead of producing garbage.
 *
 * RECORD STREAM
 *   The IL is a flat sequence of records.  Each record is one opcode plus zero
 *   or more typed fields (integers and names).  Two opcode ranges share the
 *   space:
 *     0..199    expression-node ops, reused verbatim from cj.h (NAME, CONST,
 *               PLUS, ...).  Expression trees are emitted in POSTFIX order, so
 *               c3comp2 rebuilds them with an operand stack (each op's arity is
 *               known).  Leaf records carry full type info (c3comp2 has no
 *               symbol table); interior operator records carry only the op -
 *               their operands are already on the stack, and c3comp2's optim
 *               assigns operator result types and inserts coercions.
 *     200..     IL_* structural ops (below): module/section framing, function
 *               frames, and STRUCTURED control flow (IF/ELSE/ENDIF, WHILE, ...)
 *               so c3comp2 owns all label and jump generation.
 *   An expression belonging to a structural record is emitted as a postfix run
 *   immediately after that record, terminated by IL_ENDE.
 *
 * LEAF FIELD LAYOUTS (postfix expression nodes that carry data)
 *   NAME   : int type, int size, int storage, int offset, int modifier,
 *            name sname            (dimptr chain appended later, see IL_DIM)
 *   CONST  : int type, int value
 *   LCONST : int type, int hi, int lo          (32-bit split into two ints)
 *   STRING : int label                         (string-literal data label #)
 *   FCONST/DCONST and the dimptr chain are added when float/array support lands.
 */

#ifndef IL_H
#define IL_H

#define IL_MAGIC   "c3il"        /* stream header tag */
#define IL_VERSION 1             /* bump on any incompatible format change */

#define ILEOF      (-1)          /* ilrrec() return at end of stream */

#define IL_LINEMAX 256           /* longest IL record (read side line buffer) */
#define IL_NAMEMAX 16            /* longest name field (>= cj.h NAMESIZE) */

/* ---- structural opcodes (range 200+, disjoint from cj.h expnode ops) ---- */
#define IL_ENDE    200           /* end of a postfix expression tree */
#define IL_PSECT   201           /* psect: name, n,n,n,n,n attrs */
#define IL_NAM     202           /* nam: name */
#define IL_VSECT   203           /* begin data/bss image section */
#define IL_ENDSECT 204           /* end data/bss image section */
#define IL_DATA    205           /* initialised datum: int size, int value */
#define IL_BSS     206           /* uninitialised reservation: int nbytes */
#define IL_FUNC    207           /* function entry: name, int framesize */
#define IL_ENDFUNC 208           /* function exit */
#define IL_RET     209           /* return; followed by expr+ENDE, or bare */
#define IL_EXPR    210           /* side-effecting expression statement */
#define IL_IF      211           /* if: test expr+ENDE follows */
#define IL_ELSE    212
#define IL_ENDIF   213
#define IL_WHILE   214           /* while: test expr+ENDE follows */
#define IL_ENDWHILE 215
#define IL_DO      216
#define IL_DOWHILE 217           /* do-while trailing test: expr+ENDE follows */
#define IL_FOR     218           /* for: init/test/step framed (see ilwrite) */
#define IL_ENDFOR  219
#define IL_SWITCH  220           /* switch: control expr+ENDE follows */
#define IL_CASE    221           /* case: int value */
#define IL_DEFAULT 222
#define IL_ENDSW   223
#define IL_BREAK   224
#define IL_CONT    225
#define IL_GOTO    226           /* goto: int label */
#define IL_LABEL   227           /* user label site: int label */
#define IL_DIM     228           /* one dimptr-chain element for a preceding leaf */
#define IL_BLOCK   229           /* enter block: frame level (stlev) after local decls */
#define IL_ENDBLOCK 230          /* exit block: savlev, had-locals flag */
#define IL_NULL    231           /* a NULL child of a binary node (e.g. end of an ARG chain) */
#define IL_RAW     232           /* begin verbatim asm passthrough (initialised data) */
#define IL_ENDRAW  233           /* end of an IL_RAW block */
#define IL_STR     234           /* string literal: temp-id, then body lines + IL_ENDRAW */
#define IL_DWSTR   235           /* inside IL_RAW: fdb of a string's label (temp-id) */
#define IL_LBSS    236           /* function-local static: temp-id, size, area */
#define IL_LBL     237           /* inside IL_RAW: a local static's label site (temp-id) */
#define IL_PARMOV  238           /* register-param spill: int reg-op, int stack offset */

/* ---- codec seam: write side (c3comp1, defined in ilwrite.c) ---- */
extern int  ilwinit(void);          /* emit IL_MAGIC + IL_VERSION header */
extern int  ilwrec(int);           /* ilwrec(op): begin a record */
extern int  ilwint(int);           /* ilwint(n): append an integer field */
extern int  ilwname(char *);          /* ilwname(s): append a name field */
extern int  ilweol(void);           /* terminate the current record */

/* ---- codec seam: read side (c3comp2, defined in ilread.c) ---- */
extern int   ilrinit(void);         /* read + verify the header (fatal on mismatch) */
extern int   ilrrec(void);          /* read next record's opcode; ILEOF at end */
extern int   ilrint(void);          /* read the next integer field (16-bit) */
extern char *ilrname(void);         /* read the next name field (into a static buf) */
extern char *ilrline(void);         /* read a whole raw line (IL_RAW passthrough); NULL at EOF */
extern char *ilrbuf(void);          /* the current record's raw line text (for IL_RAW dispatch) */
/* 32-bit fields are emitted/read as two ilwint/ilrint halves (hi, lo);
 * ilwlong/ilrlong wrappers will be added with LCONST support. */

#endif /* IL_H */
