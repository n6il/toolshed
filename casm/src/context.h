/*****************************************************************************
	context.h	- Context definitions for free floating implementation

	Copyright (c) 2004 Chet Simpson, Digital Asphyxia. All rights reserved.

	The distribution, use, and duplication this file in source or binary form
	is restricted by an Artistic License (see license.txt) included with the
	standard distribution. If the license was not included with this package
	please refer to http://www.oarizo.com for more information.


	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that: (1) source code distributions
	retain the above copyright notice and this paragraph in its entirety, (2)
	distributions including binary code include the above copyright notice and
	this paragraph in its entirety in the documentation or other materials
	provided with the distribution, and (3) all advertising materials
	mentioning features or use of this software display the following
	acknowledgement:

		"This product includes software developed by Chet Simpson"
	
	The name of the author may be used to endorse or promote products derived
	from this software without specific priorwritten permission.

	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/
#ifndef CONTEXT_H
#define CONTEXT_H

#include "config.h"
#include "pseudo.h"


#define NO_CONDITION	-1

typedef enum {
	ASM_MODE_CASM,
	ASM_MODE_EDTASM,
	ASM_MODE_EDTASM6309,
	ASM_MODE_RMA,
	ASM_MODE_CCASM,
	ASM_MODE_MACRO80C
} ASM_MODE;



#define ASM_EDTASM		0x0001
#define ASM_EDTASMX		0x0002
#define ASM_MACRO80		0x0004
#define ASM_RMA			0x0008
#define ASM_CCASM		0x0010
#define ASM_CASM		0x8000
#define ASM_ALL			0xffff
#define ASM_NOEDTASM	(ASM_ALL & ~ASM_EDTASMX)





typedef struct {
	CPUTYPE		m_CPUType;				/* Type of CPU to assemble for				*/
	OUTPUT_TYPE	m_OutputType;				/* File generation type						*/
	int			m_Pass;					/* Which pass the assembler is on			*/
	u_int32		m_LineNumber;			/* Line number of the current file			*/
	u_int32		m_CycleCount;			/* # of cycles per instruction				*/
	u_int32		m_CycleTotal;			/* # of cycles seen so far					*/
	bool		m_ForceWord;			/* Result should be a word when set			*/
	bool		m_ForceByte;			/* Result should be a byte when set			*/
	bool		m_OptimizeLine;			/* Flag to indicate line can be optimized	*/
	bool		m_Finished;				/* flag indicating END directive       		*/
	bool		m_HasEntryPoint;		/* m_EntryPoint is valid					*/
	u_int16		m_EntryPoint;			/* 16 bit address to the files entry point	*/
	int			m_ErrorCount;			/* # of errors encountered during assembly	*/
	int			m_WarningCount;			/* # of warnings encounted during assembly	*/
	bool		m_SilentMode;			/* Only display errors and warnings			*/
	bool		m_OriginSet;			/* Has the origin been set?					*/
	u_int16		m_OrgBase;				/* ORG base									*/
	bool		m_DisableWarnings;		/* Disable warnines							*/

	struct {
		ASM_MODE	m_AsmMode;			/* Assemble files in X compatibility mode	*/
		u_int16		m_AsmOpMask;		/* Mask indicating available instructions	*/
		bool		m_Warn;				/* Warn of source code compatibility issues	*/
		bool		m_DisablePCIndex;	/* Disable use of PC in indexing			*/
		bool		m_ForcePCR;			/* Force PC to act as PCR					*/
		bool		m_ForceZeroOffset;	/* Force a 0 offset postbyte on indexed ops	*/
		bool		m_StrictLocals;		/* Use strict (Macro-80c) macro style		*/
		bool		m_DisableMacros;	/* Disable Macros							*/
		bool		m_DisableLocals;	/* Disable local labels						*/
		bool		m_IgnoreCase;		/* Ignore case on symbols					*/
		bool		m_UsePrecedence;	/* Use operator precedence (Macro-80C)		*/
		bool		m_SemicolonComment;	/* Allow semi-colons as comments?			*/
	} m_Compat;

	struct {
		char	m_Title[MAX_TITLESIZE];	/* Assembly title							*/
		bool	m_CmdEnabled;			/* Enable listing							*/
		bool	m_OptEnabled;			/* Enabled/Disabled by source options		*/
		bool	m_ListSymbols;			/* List symbols and end of assembly			*/
		bool	m_ExpandMacros;			/* Expand Macros in listing?				*/
		int		m_PageNumber;			/* Page number								*/
		u_char	P_bytes[PMIT_LIMIT];	/* Bytes collected for listing				*/
		bool	P_force;				/* force listing line to include regtenOldPC */
		int		P_total;				/* current number of bytes collected		*/
		bool	OptNoLineNumbers;		/* Do not print line numbers in listing		*/
		bool	OptNoOpData;			/* Do not print op data in listing			*/
		bool	OptCommentMacros;		/* Comment out macros in listing			*/
	} m_ListingFlags;

	struct {
		struct {
			bool		m_State;		/* Conditional state						*/
			PSEUDO_OP	m_Op;			/* Conditional op							*/
		}  m_Cond[MAX_CONDITIONS];
		int			m_Count;			/* number of conditionals in effect			*/
	} m_Cond;

	struct {
		bool	Oflag;					/* enable/disable output file				*/
		bool	Cflag;					/* cycle count flag 						*/
		bool	Sflag;					/* symbol table flag, 0=no symbol			*/
		bool	N_page;					/* new page flag							*/
		bool	OptFlag;				/* Display number of optimizable lines		*/
		bool	OptShow;				/* Display lines that could be optimized	*/
	} m_Misc;



	const char	*m_Line;				/* Pointer to the full buffered line		*/
	const char	*m_Label;				/* Pointer to the label for the line		*/
	const char	*m_Opcode;				/* Pointer to the opcode for the line		*/
	const char	*m_Operand;				/* Pointer to the operand buffer			*/
	const char	*m_Ptr;					/* Current operand pointer					*/
} EnvContext;


/* Context helper functions */
int GetConditionalCount(EnvContext *ctx);
bool GetConditionalState(EnvContext *ctx);
PSEUDO_OP GetConditionalOp(EnvContext *ctx);
void PushConditionalState(EnvContext *ctx, const char *opname, PSEUDO_OP op);
void PopConditionalState(EnvContext *ctx);
bool ToggleConditionalState(EnvContext *ctx);



#endif	/* CONTEXT_H */
