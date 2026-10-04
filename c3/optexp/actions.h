/*
 * actions.h - c3optx peephole action-table types, and the symbols optim.c uses.
 *
 * The table DATA lives in actions.c, not here: keeping that large block of
 * initialised file-scope data out of optim.c is what lets c3comp1 (pass 1)
 * compile optim.c within the OS-9 Level 2 per-process heap.
 */

typedef struct ins
{
	struct ins     *nxtins;
	char           *mnp,
	               *opp;
}               instr;

typedef struct act
{
	struct act     *nxtact;
	instr          *actp,
	               *repp;
}               action;

extern action   actions[];      /* the action table (actions.c) */
extern char     one[];          /* operand string optim.c tests by identity */
