/*****************************************************************************
	parse.c	- Line parsing routines

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
#include "as.h"
#include "proto.h"
#include "os9.h"
#include "label.h"


/*
	Add lines to macros
	Process ELSE/END conditional directives
	Add elements to structures
	Process the line as normal
*/

bool ProcessLine(EnvContext *ctx)
{
	bool condition;
	const Mneumonic *mne;

	/* Get the opcode if it exists */
	if(EOS != ctx->m_Opcode) {
		mne = mne_look(ctx, ctx->m_Opcode);
	} else {
		mne = NULL;
	}

	SetOldPCReg(GetPCReg());

	/*
		Lines within MACRO/ENDM must be processed first in order to allow for
		dynamic IF/ELSE/ENDIF conditionals. This means that IF/MACRO/ENDIF/ENDM
		constructs are not supported and conditionals must exist entirely
		inside or outside of the macro declaration or havoc will result
	*/
	if(true == IsProcessingMacro()) {
		if(NULL != mne && ENDM == mne->opcode) {
			/* Proc the pseudo op as normal */
			ProcPseudo(ctx, mne);
		} else {
			/* Add the line to the macro */
			AddLineToMacro(ctx, ctx->m_Line);
		}

		return true;
	}

	/* Get the current condition if it exists */
	condition = GetConditionalState(ctx);

	/* If we are in a ! conditional process the line if it's an ELSE or END pseudo */
	if(false == condition) {

		/* If there is no pseudo opcode just return; */
		if(NULL != mne && PSEUDO == mne->optype ) {
			switch(mne->opcode) {
			case ELSE:
			case ENDIF:
			case ENDP:
			case ENDC:
				ProcPseudo(ctx, mne);
				break;
			}
		}

		return false;
	}


	/*
		We have arrived here which means the line needs to be processed
		and we are not inside of a macro.
	*/

	 /* If we are inside of a structure add the element */
	if(instruct == true) {
		StructAddElement(ctx, ctx->m_Label, ctx->m_Opcode);
		return false;
	}

	/* Process special pseudo ops first */
	if(NULL != mne && PSEUDO == mne->optype) {
		switch(mne->opcode) {
		case MACRO:		/* Label is used in a different scope */
			ProcPseudo(ctx, mne);
			return false;

		case STRUCT:	/* Label is used in a different scope */
		case EQU:		/* Label is defined as a value instead of an address */
			ProcPseudo(ctx, mne);
			return false;
		}
	}


	/*
		DO NOT ADD SYMBOLS HERE! ProcPseudo is responsible for adding symbols
		for all ops that it handles.
	*/
	
	/* If no mneumonic was found, open as a macro */
	if(NULL == mne)	{

		if(*ctx->m_Opcode) {
			bool result;

			if(*ctx->m_Label) {
				AddSymbol(ctx, ctx->m_Label, GetPCReg(), SYM_ADDRESS, NULL, 0);
			}

			result = OpenMacro(ctx, ctx->m_Opcode, ctx->m_Operand);
			
			if(false == result) {
				error(ctx, ERR_SYNTAX, "unrecognized mneumonic '%s'", ctx->m_Opcode);
			}
		}
	}
	
	/* if its a pseudo, go do it */
	else if(mne->optype == PSEUDO) {	
		ProcPseudo(ctx, mne);
	}
		
	else {
		ctx->m_CycleCount = 0;

		/* Add the symbol if it's there */
		if(*ctx->m_Label) {
			AddSymbol(ctx, ctx->m_Label, GetPCReg(), SYM_ADDRESS, NULL, 0);
		}
		/*
		if(CPU_6809 == ctx->m_CPUType) {
			ctx->m_CycleCount = mne->cycles6809;
		} else {
			ctx->m_CycleCount = mne->cycles6309;
		}
		*/

		if(NOPAGE == mne->page && 0x39 == mne->opcode) {
			Symbol *sym = FindSymbol(ctx, "?RTS", true, false);
			if(NULL == sym) {
				AddSymbol(ctx, "?RTS", GetPCReg(), SYM_ADDRESS, NULL, 0);
			}
		}

		ProcOpcode(ctx, mne); /* FIXME - dunno why but FIXME */

		ctx->m_CycleTotal += ctx->m_CycleCount;
	}

	return false;
}


/*
 *	ParseLine --- split input line into label, op and operand
 */
bool ParseLine(LineBuffer *line, EnvContext *ctx)
{
	char *dst;
	const char *src;
	bool procMacro;


	line->m_Label[0] = EOS;
	line->m_Opcode[0] = EOS;
	line->m_Operand[0] = EOS;

	ctx->m_Line = line->m_Line;
	ctx->m_Label = line->m_Label;
	ctx->m_Opcode = line->m_Opcode;
	ctx->m_Operand = line->m_Operand;
	ctx->m_Ptr = ctx->m_Operand;


	/* Reset the contents of the buffers */
	src = line->m_Line;

	procMacro = IsProcessingMacro();


	/*************************************************************************
		check for and skip line numbers
	*************************************************************************/
	if(isdigit(*src)) {

		/* Skip the digits */
		while(isdigit(*src)) {
			src++;
		}

		/* Require a whitespace or EOL */
		if(EOS != *src && false == IsWS(*src)) {
			error(ctx, ERR_SYNTAX, "invalid character after line number");
			return false;
		}

		/* Skip trailing whitespace */
		if(true == IsWS(*src)) {
			src++;
		}
	}

	/*************************************************************************
		check for a comment line and return if one
	*************************************************************************/
	if(EOS == *src || true == IsCommentChar(ctx, *src)) {
		if(true == procMacro) {
			AddLineToMacro(ctx, line->m_Line);
		}
		return false;
	}

	/*************************************************************************
		Parse the label if it's there
	*************************************************************************/
	if(false == IsWS(*src)) {
		char *dst;
		char *limit;

		dst = line->m_Label;
		limit = line->m_Label + MAX_LABELSIZE - 1;

		if(true == procMacro && ASM_MODE_MACRO80C == ctx->m_Compat.m_AsmMode) {
			if('\\' == *src && '.' == *(src + 1)) {
				*(dst++) = *(src++);
				*(dst++) = *(src++);
			}
		}


		if(false == IsLabelStart(ctx, *src)) {
			error(ctx, ERR_INVALID_LABEL, "invalid character at beginning of label");
			return false;
		}

		/* Copy the label */
		while(true == IsLabelChar(ctx, *src) && dst < limit) {
			*dst++ = *src++;
		}

		if(':' == *src) {
			warning(ctx, WARN_COMPAT, "usage of a colon to terminate a symbol name assumes automatic export in object file");
			src++;
		}

		/* Check for length of label */
		if(EOS != *src && false == IsWS(*src)) {
			warning(ctx, WARN_LABELSIZE, "label too long");

			while(EOS != *src && false == IsWS(*src)) {
				src++;
			}
		}

		*dst = EOS;

		if(true == ctx->m_Compat.m_Warn) {
			bool islocal;

			dst = line->m_Label;
			islocal = false;

			while(*dst) {
				if('?' == *dst) {
					warning(ctx, WARN_COMPAT, "non-traditional character '?' used in local label");
				}

				if('@' == *dst) {
					if(0 != dst[1]) {
						warning(ctx, WARN_COMPAT, "non-traditional syntax used for local label");
					} else {
						islocal = true;
					}
				}

				dst++;
			}

			/* Check local label size */
			if(true == islocal && strlen(line->m_Label) > 2) {
				warning(ctx, WARN_COMPAT, "size of local label not compatible with Macro-80C");
			}

		}
	}

	/*************************************************************************
		Parse the opcode. Simple skip until a non ws is found
	*************************************************************************/
	src = SkipConstWS(src);
	dst = line->m_Opcode;
	if(EOS != *src) {
		char *limit;

		limit = line->m_Opcode + MAX_BUFFERSIZE - 1;

		while(EOS != *src && false == IsWS(*src) && dst < limit) {
			*(dst++) = *(src++);
		}

		if(EOS != *src && false == IsWS(*src)) {
			error(ctx, ERR_SYNTAX, "invalid character in mneumonic");

			while(EOS != *src && false == IsWS(*src)) {
				src++;
			}
		}
	}
	*dst = EOS;


	/*************************************************************************
		Copy the operand over
	*************************************************************************/
	src = SkipConstWS(src);
	dst = line->m_Operand;
	while(*src != EOS) {
		*dst++ = *src++;
	}
	*dst = EOS;

	return(true);
}



