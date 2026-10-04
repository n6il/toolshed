/*
 * proto.h - prototypes for the original "comp" compiler.
 *
 * Generated from comp's own (preprocessor-active) function definitions, the
 * same way compexp/proto.h is.  Without these declarations, pointer-returning
 * K&R functions are implicitly declared "int" at their call sites, so on a
 * 64-bit (LP64) host their results are truncated to 32 bits -> garbage pointer
 * -> crash.  (On the 16-bit 6809 target int == pointer, so the originals were
 * fine.)  ANSI-style definitions get an exact prototype, K&R-style ones get an
 * empty parameter list.  The functions already in cj.h's extern block and
 * main() are omitted.
 */

char *cgets();
char *getrel();
char *grab();
symnode *lookup();
expnode *regcont();
char regname();
double scale();
double scale0();
expnode *subtree();
expnode *tranbool();
expnode *transexp();
expnode *treecont();
expnode *treecopy();
