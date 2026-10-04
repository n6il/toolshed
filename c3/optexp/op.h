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
	struct chstruct *succ,		/* self-referential: c3 requires a member
					   name's type be consistent across all
					   structs, and rejects mixing a struct
					   pointer with void or scalar pointers */
	               *pred;
}               chain;

typedef struct istruct
{
	struct istruct            *succ,
	               *pred;
	struct lstruct
	{
		struct lstruct *succ,	/* self-referential (see chain above) */
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
extern chain    ltable[];
extern DIRECT label *lfree;
extern DIRECT int inscount;
extern FILE    *in,
               *outf,
               *datfile;
extern DIRECT int lbf,
                lbdone,
                opsdone;
extern DIRECT int dbflag;

/*
 * function prototypes - abstract (parameter TYPES only, no names).
 *
 * Every non-static (global) function defined across the optexp .c files is
 * declared here so each translation unit sees a real ANSI prototype.  Static
 * functions are NOT declared here (there are none in optexp); a static helper
 * would carry its prototype in its own .c file.
 *
 * No-argument functions use the ANSI (void) form in both prototype and
 * definition; c3 accepts (void) for an empty parameter list.
 *
 * error() and parse() keep their fixed four / four parameter shapes from their
 * definitions: error()/debug() are the classic "poor man's varargs" idiom that
 * forwards all trailing char* slots to fprintf (the format string decides how
 * many are read), and parse()'s fourth argument is dereferenced only for a
 * LABEL parse.  Call sites pass the full argument count to match.
 */
char           *parse(int, char *, char *, int *);
char           *newarg(int);
char           *grab(unsigned);
char           *newbra(char *);

void            movlab(instruction *, instruction *);
void            remins(instruction *);
void            insref(instruction *, label *);
void            error(char *, char *, char *, char *);
void            fix(instruction *, int);
void            freechain(chain *);
void            remref(instruction *, label *);
void            freearg(chain *);
void            freelabel(label *);
void            endsect(void);
void            optim(instruction *);
void            finddupl(instruction *);
void            rminst(void);
void            labelinit(void);
void            insinit(void);
void            optinit(void);
void            optimise(void);
void            fixcond(instruction *);
void            duplrefs(label *, instruction *);

int             estimate(int);
int             percent(unsigned, unsigned);
int             hash(char *);
int             match(char *, char *);
int             duplsrch(instruction *, instruction *);

chain          *newchain(void);
label          *inslabel(char *);
label          *newlabel(void);
label          *findlabel(char *);
instruction    *insins(instruction *, char *, char *, label **);
instruction    *newins(void);
char          **srchmnem(char *, char **, char **);

#ifdef DEBUG
void            debug(char *, char *, char *, char *);
void            prtins(instruction *);
#endif
