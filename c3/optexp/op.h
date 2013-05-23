#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* Accomodate both 6809 ("direct" storage class) and non-6809 */
#ifndef DIRECT
#define DIRECT
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

#define GLOBAL      0x01	/* global label flag */

typedef struct chstruct
{
	void           *succ,
	               *pred;
}               chain;

typedef struct istruct
{
	struct istruct            *succ,
	               *pred;
	struct lstruct
	{
		int            *succ,
		               *pred;
		struct istruct *dest;
		chain          *rlist;
		struct lstruct *nextl;
		int             lflags;
		char            lname[LABELSIZE + 1];
	}              *llist;
	int             itype;
	char            mnem[MNEMSIZE + 1];
	union
	{
		char           *args;
		struct lstruct *lab;
	}               alt;
}               instruction;

#if 0
typedef union istruct
{
	struct
	{
		int            *succ,
		               *pred;
		struct lstruct
		{
			int            *succ,
			               *pred;
			 /* struct */ union istruct *dest;
			chain          *rlist;
			struct lstruct *nextl;
			int             lflags;
			char            lname[LABELSIZE + 1];
		}              *llist;
		int             itype;
		char            mnem[MNEMSIZE + 1];
		char           *args;
	}               instyp1;
	struct
	{
		int            *succ,
		               *pred;
		struct lstruct *llist;
		int             itype;
		char            mnem[MNEMSIZE + 1];
		struct lstruct *lab;
	}               instyp2;
}               instruction;

#endif

typedef struct lstruct label;

extern DIRECT chain ilist;
extern chain    ltable[],
               *newchain();
extern char   *parse();
extern void    debug(), movlab(), remins(), insref(), prtins(), error(), fix(), freechain();
extern void    remref(), freearg(), freelabel(), endsect(), optim(), finddupl(), rminst();;
extern void    labelinit(), insinit(), optinit(), optimise();
extern int     estimate(), percent(), hash();
extern char    *newarg(), *grab();
extern label   *inslabel(), *newlabel(), *findlabel();
extern DIRECT label *lfree;
extern instruction *insins(), *newins();
extern DIRECT int inscount;
extern FILE    *in,
               *out,
               *datfile;
extern DIRECT int lbf,
                lbdone,
                opsdone;
extern DIRECT int dbflag;
