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

struct pseudo_opcode pseudo[] = {
	{"=",		EQU,    HAS_OPERAND,					_equ},
	{"absolute",OTHER,  HAS_NO_OPERAND,					_null_op},
	{"align",   OTHER,  HAS_OPERAND,					_align},
	{"bsz",		FC,		HAS_NO_OPERAND,					_zmb},
	{"byte",	FC,		HAS_OPERAND,					_fcb},
	{"code",	OTHER,  HAS_NO_OPERAND,					_null_op},
	{"cond",	IF,		HAS_OPERAND,					_ifne},
	{"db",		RM,		HAS_OPERAND,					_fcb},
	{"ds",		RM,		HAS_OPERAND,					_rmb},
	{"dtb",		FC,		HAS_NO_OPERAND,					_dtb},
	{"dts",		FC,		HAS_NO_OPERAND,					_dts},
	{"dw",		RM,		HAS_OPERAND,					_fdb},
	{"dword",	FC,		HAS_OPERAND,					_fqb},
	{"else",	ELSE,   HAS_NO_OPERAND,					_else},
	{"emod",	OTHER,  HAS_NO_OPERAND,					_emod},
	{"end",		OTHER,  HAS_OPERAND,					__end},
	{"endc",	ENDC,   HAS_NO_OPERAND,					_endc},
	{"endif",	ENDC,   HAS_NO_OPERAND,					_endc},
	{"equ",		EQU,    HAS_OPERAND,					_equ},
	{"even",	OTHER,  HAS_NO_OPERAND,					_even},
	{"fcb",		FC,		HAS_OPERAND,					_fcb},
	{"fcc",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcc},
	{"fcn",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcz},
	{"fcr",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcr},
	{"fcs",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcs},
	{"fcz",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcz},
	{"fdb",		FC,		HAS_OPERAND,					_fdb},
	{"fill",	FC,		HAS_OPERAND,					_fill},
	{"fqb",		FC,		HAS_OPERAND,					_fqb},
	{"fzb",		FC,		HAS_OPERAND,					_zmb},
	{"fzd",		FC,		HAS_OPERAND,					_zmd},
	{"fzq",		FC,		HAS_OPERAND,					_zmq},
	{"if",		IF,		HAS_OPERAND,					_ifne},
	{"ifeq",	IF,		HAS_OPERAND,					_ifeq},
	{"ifge",	IF,		HAS_OPERAND,					_ifge},
	{"ifgt",	IF,		HAS_OPERAND,					_ifgt},
	{"ifle",	IF,		HAS_OPERAND,					_ifle},
	{"iflt",	IF,		HAS_OPERAND,					_iflt},
	{"ifn",		IF,		HAS_OPERAND,					_ifne},
	{"ifne",	IF,		HAS_OPERAND,					_ifne},
	{"ifp1",	IF,		HAS_NO_OPERAND,					_ifp1},
	{"ifp2",	IF,		HAS_NO_OPERAND,					_ifp2},
	{"include", OTHER,  HAS_OPERAND,					_use},
	{"lib",		OTHER,  HAS_OPERAND,					_use},
	{"mod",		OTHER,  HAS_OPERAND,					_mod},
	{"nam",		OTHER,  HAS_OPERAND_WITH_SPACES,		_nam},
	{"name",	OTHER,  HAS_OPERAND_WITH_SPACES,		_nam},
	{"odd",		EQU,    HAS_NO_OPERAND,					_odd},
	{"opt",		OTHER,  HAS_OPERAND,					_opt},
	{"org",		OTHER,  HAS_OPERAND,					_org},
	{"origin",  OTHER,  HAS_OPERAND,					_org},
	{"pag",		OTHER,  HAS_NO_OPERAND,					_page},
	{"page",	OTHER,  HAS_NO_OPERAND,					_page},
	{"page0",   OTHER,  HAS_NO_OPERAND,					_null_op},
	{"page1",   OTHER,  HAS_NO_OPERAND,					_null_op},
	{"relative",RM,		HAS_OPERAND,					_rmb},
	{"rmb",		RM,		HAS_OPERAND,					_rmb},
	{"rmd",		RM,		HAS_OPERAND,					_rmd},
	{"rmq",		RM,		HAS_OPERAND,					_rmq},
	{"set",		EQU,    HAS_OPERAND,					_set},
	{"setdp",   EQU,    HAS_OPERAND,					_setdp},
	{"spc",		OTHER,  HAS_NO_OPERAND,					_spc},
	{"title",   OTHER,  HAS_OPERAND_WITH_SPACES,		_ttl},
	{"ttl",		OTHER,  HAS_OPERAND_WITH_SPACES,		_ttl},
	{"use",		OTHER,  HAS_OPERAND,					_use},
	{"word",	FC,		HAS_OPERAND,					_fdb},
	{"zmb",		OTHER,  HAS_OPERAND,					_zmb},
	{"zmd",		OTHER,  HAS_OPERAND,					_zmd},
	{"zmq",		OTHER,  HAS_OPERAND,					_zmq}
};

