/*###########################################################################
#                                                                           #
#                          OS-9/6809 CROSS ASSEMBLER                        #
#                                                                           #
#                                                                           #
#############################################################################
#                                                                           #
# $Id$
#                                                                           #
#############################################################################
#                                                                           #
# File: pseudo.h                                                            #
# Purpose: pseudo code definitions                                          #
############################################################################*/

/*
 * ALPHABETIZED table of pseudo ops and some equivalent
 * pseudo ops added for compatibility with other assemblers.
 */

struct oper pseudo[] = {
	{"absolute",PSEUDO, NULL_OP, 0,	1,	_null_op},
	{"bsz",		PSEUDO, ZMB,    0,	1,	_zmb},
	{"code",	PSEUDO, NULL_OP,0,	1,	_null_op},
	{"cond",	PSEUDO, IFNE,   1,	1,	_ifne},
	{"db",		PSEUDO, FCB,    1,	1,	_fcb},
	{"ds",		PSEUDO, RMB,    1,	1,	_rmb},
	{"dw",		PSEUDO, FDB,    1,	1,	_fdb},
	{"else",	PSEUDO, ELSE,   0,	1,	_else},
	{"emod",	PSEUDO, EMOD,   0,	1,	_emod},
	{"end",		PSEUDO, NULL_OP,0,	1,	__end},
	{"endc",	PSEUDO, ENDC,   0,	1,	_endc},
	{"endif",	PSEUDO, ENDC,   0,	1,	_endc},
	{"equ",		PSEUDO, EQU,    1,	1,	_equ},
	{"fcb",		PSEUDO, FCB,    1,	1,	_fcb},
	{"fcc",		PSEUDO, FCC,    2,	1,	_fcc},
	{"fcn",		PSEUDO, FCZ,    2,	1,	_fcz},
	{"fcs",		PSEUDO, FCS,    2,	1,	_fcs},
	{"fcz",		PSEUDO, FCZ,    2,	1,	_fcz},
	{"fdb",		PSEUDO, FDB,    1,	1,	_fdb},
	{"fill",	PSEUDO, FILL,   1,	1,	_fill},
	{"if",		PSEUDO, IFNE,   1,	1,	_ifne},
	{"ifeq",	PSEUDO, IFEQ,   1,	1,	_ifeq},
	{"ifge",	PSEUDO, IFGE,   1,	1,	_ifge},
	{"ifgt",	PSEUDO, IFGT,   1,	1,	_ifgt},
	{"ifle",	PSEUDO, IFLE,   1,	1,	_ifle},
	{"iflt",	PSEUDO, IFLT,   1,	1,	_iflt},
	{"ifn",		PSEUDO, IFNE,   1,	1,	_ifne},
	{"ifne",	PSEUDO, IFNE,   1,	1,	_ifne},
	{"ifp1",	PSEUDO, IFP1,   0,	1,	_ifp1},
	{"ifp2",	PSEUDO, IFP2,   0,	1,	_ifp2},
	{"include", PSEUDO, USE,    1,	1,	_use},
	{"lib",		PSEUDO, USE,    1,	1,	_use},
	{"mod",		PSEUDO, MOD,    1,	1,	_mod},
	{"nam",		PSEUDO, NULL_OP,4,	1,	_nam},
	{"name",	PSEUDO, NULL_OP,4,	1,	_nam},
	{"opt",		PSEUDO, OPT,    1,	1,	_opt},
	{"org",		PSEUDO, ORG,    1,	1,	_org},
	{"origin",  PSEUDO, ORG,    1,	1,	_org},
	{"pag",		PSEUDO, PAGE,   0,	1,	_page},
	{"page",	PSEUDO, PAGE,   0,	1,	_page},
	{"page0",   PSEUDO, NULL_OP,0,	1,	_null_op},
	{"page1",   PSEUDO, NULL_OP,0,	1,	_null_op},
	{"relative",PSEUDO, RMB,  0,	1,	_rmb},
	{"rmb",		PSEUDO, RMB,    1,	1,	_rmb},
	{"set",		PSEUDO, EQU,    1,	1,	_set},
	{"setdp",   PSEUDO, EQU,    1,	1,	_setdp},
	{"spc",		PSEUDO, NULL_OP,0,	1,	_spc},
	{"ttl",		PSEUDO, NULL_OP,4,	1,	_ttl},
	{"title",   PSEUDO, NULL_OP,4,	1,	_ttl},
	{"use",		PSEUDO, USE,    1,	1,	_use},
	{"zmb",		PSEUDO, ZMB,    1,	1,	_zmb},
};

