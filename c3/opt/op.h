#include <stdio.h>

/* "direct" is a 6809 storage class (direct page); empty off-target. */
#ifndef OS9
#define direct
#endif

#define LABELSIZE       11
#define MNEMSIZE        10
#define ARGSIZE         90

#define LABEL       1
#define MNEM        2
#define ARGS        3
#define BRALIM     36
#define MAXINS    300

/* instruction type flags */
#define SUBR        0x0001
#define BRANCH      0x0020
#define BRATYPE     0x001F
#define BRAMASK     (BRANCH|BRATYPE)
#define LONG        0x0040
#define LONGBRA     (LONG|BRANCH)
#define BLOCKDUP    0x0080
#define CODEBRK     0x0100
#define DESTPTR     0x0200

#define TRUE    1
#define FALSE   0

#define FORWARD 4
#define BACK    5

#define GLOBAL      0x01    /* global label flag */

/*
 * chain, label (lstruct) and instruction are intrusive circular-list nodes
 * that all begin with succ/pred and are freely interchanged.  The original
 * K&R sources typed the links as the generic pointer "int *"; modern C will
 * not dereference an int * as a struct, so the links are typed as "chain *"
 * here.  The cross-node assignments that remain are flagged only as
 * incompatible-pointer warnings (demoted in rules.mak); the layout, and hence
 * the runtime behaviour, is unchanged.
 */
typedef struct chstruct {
    struct chstruct *succ,*pred;
} chain;

typedef union istruct {
    struct {
        chain *succ,*pred;
        struct lstruct {
            chain *succ,*pred;
            union istruct *dest;        /* istruct is a union, not a struct */
            chain *rlist;
            struct lstruct *nextl;
            int lflags;
            char lname[LABELSIZE+1];
        } *llist;
        int itype;
        char mnem[MNEMSIZE+1];
        char *args;
    } instyp1;
    struct {
        chain *succ,*pred;
        struct lstruct *llist;
        int itype;
        char mnem[MNEMSIZE+1];
        struct lstruct *lab;
    } instyp2;
} instruction;

typedef struct lstruct label;

extern direct chain ilist;
extern chain ltable[],*newchain();
extern char *newarg(),*grab();
extern label *inslabel(),*newlabel(),*findlabel();
extern direct label *lfree;
extern instruction *insins(),*newins();
extern direct int inscount;
extern FILE *in,*out,*datfile;
extern direct int lbf,lbdone,opsdone;
extern direct int dbflag;
