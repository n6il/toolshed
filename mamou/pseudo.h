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
	{"=",		EQU,    HAS_OPERAND,					_equ,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"absolute",OTHER,  HAS_NO_OPERAND,					_null_op,	RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"align",   OTHER,  HAS_OPERAND,					_align,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"bsz",		FC,		HAS_NO_OPERAND,					_zmb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"byte",	FC,		HAS_OPERAND,					_fcb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"code",	OTHER,  HAS_NO_OPERAND,					_null_op,	RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"cond",	IF,		HAS_OPERAND,					_ifne,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"db",		RM,		HAS_OPERAND,					_fcb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ds",		RM,		HAS_OPERAND,					_rmb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"dtb",		FC,		HAS_NO_OPERAND,					_dtb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"dts",		FC,		HAS_NO_OPERAND,					_dts,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"dw",		RM,		HAS_OPERAND,					_fdb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"dword",	FC,		HAS_OPERAND,					_fqb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"else",	ELSE,   HAS_NO_OPERAND,					_else,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"emod",	OTHER,  HAS_NO_OPERAND,					_emod,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"end",		OTHER,  HAS_OPERAND,					__end,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"endc",	ENDC,   HAS_NO_OPERAND,					_endc,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"endif",	ENDC,   HAS_NO_OPERAND,					_endc,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"equ",		EQU,    HAS_OPERAND,					_equ,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"even",	OTHER,  HAS_NO_OPERAND,					_even,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fcb",		FC,		HAS_OPERAND,					_fcb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fcc",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcc,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fcn",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcz,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fcr",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcr,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fcs",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcs,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fcz",		FC,		HAS_OPERAND_WITH_DELIMITERS,	_fcz,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fdb",		FC,		HAS_OPERAND,					_fdb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fdbs",		FC,		HAS_OPERAND,					_fdbs,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fill",	FC,		HAS_OPERAND,					_fill,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fqb",		FC,		HAS_OPERAND,					_fqb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fzb",		FC,		HAS_OPERAND,					_zmb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fzd",		FC,		HAS_OPERAND,					_zmd,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"fzq",		FC,		HAS_OPERAND,					_zmq,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"if",		IF,		HAS_OPERAND,					_ifne,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ifdef",	IF,		HAS_OPERAND,					_ifne,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ifeq",	IF,		HAS_OPERAND,					_ifeq,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ifge",	IF,		HAS_OPERAND,					_ifge,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ifgt",	IF,		HAS_OPERAND,					_ifgt,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ifle",	IF,		HAS_OPERAND,					_ifle,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"iflt",	IF,		HAS_OPERAND,					_iflt,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ifn",		IF,		HAS_OPERAND,					_ifne,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ifne",	IF,		HAS_OPERAND,					_ifne,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ifp1",	IF,		HAS_NO_OPERAND,					_ifp1,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ifp2",	IF,		HAS_NO_OPERAND,					_ifp2,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"include", OTHER,  HAS_OPERAND,					_use,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"lib",		OTHER,  HAS_OPERAND,					_use,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"mod",		OTHER,  HAS_OPERAND,					_mod,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"nam",		OTHER,  HAS_OPERAND_WITH_SPACES,		_nam,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"name",	OTHER,  HAS_OPERAND_WITH_SPACES,		_nam,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"odd",		EQU,    HAS_NO_OPERAND,					_odd,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"opt",		OTHER,  HAS_OPERAND,					_opt,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"org",		OTHER,  HAS_OPERAND,					_org,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"origin",  OTHER,  HAS_OPERAND,					_org,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"pag",		OTHER,  HAS_NO_OPERAND,					_page,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"page",	OTHER,  HAS_NO_OPERAND,					_page,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"page0",   OTHER,  HAS_NO_OPERAND,					_null_op,	RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"page1",   OTHER,  HAS_NO_OPERAND,					_null_op,	RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"relative",RM,		HAS_OPERAND,					_rmb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"rmb",		RM,		HAS_OPERAND,					_rmb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"rmd",		RM,		HAS_OPERAND,					_rmd,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"rmq",		RM,		HAS_OPERAND,					_rmq,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"set",		EQU,    HAS_OPERAND,					_set,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"setdp",   EQU,    HAS_OPERAND,					_setdp,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"spc",		OTHER,  HAS_NO_OPERAND,					_spc,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"title",   OTHER,  HAS_OPERAND_WITH_SPACES,		_ttl,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"ttl",		OTHER,  HAS_OPERAND_WITH_SPACES,		_ttl,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"use",		OTHER,  HAS_OPERAND,					_use,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"word",	FC,		HAS_OPERAND,					_fdb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"zmb",		OTHER,  HAS_OPERAND,					_zmb,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"zmd",		OTHER,  HAS_OPERAND,					_zmd,		RMA_PSECT | RMA_VSECT | RMA_CSECT},
	{"zmq",		OTHER,  HAS_OPERAND,					_zmq,		RMA_PSECT | RMA_VSECT | RMA_CSECT}
};

