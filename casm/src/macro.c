/*****************************************************************************
	macro.c	- 'macro' pseudo instruction handler

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
#include "macro.h"
#include "label.h"


typedef struct _MacroVar MacroVar;
typedef struct _MacroLine MacroLine;
typedef struct _Macro Macro;

typedef struct {
	Macro		*mac;
	MacroLine	*m_Line;
} OpenedMacro;


struct _MacroVar {
	char		m_Var[MAX_LABELSIZE];		/* variable */
	char		m_Value[MAX_BUFFERSIZE];	/* variable contents while processing macro */
	MacroVar	*m_Next;					/* next variable in list */
};


struct _MacroLine {
	char		m_Line[MAX_BUFFERSIZE];	/* Contents of the macro line */
	MacroLine	*m_Next;				/* Next macro line */
};


struct _Macro {
	char		m_Name[MAX_LABELSIZE];	/* Name of the macro */
	int			m_ID;					/* Unique ID */
	int			m_VarCount;				/* Number of variables in the macro */
	MacroVar	*m_Vars;				/* Variables */
	MacroLine	*m_Lines;				/* Macro lines */
	Macro		*m_Next;				/* Next macro in the list */
};



static void AddVarToMacro(EnvContext *ctx, const char *var);


/*
	Macro processing pointers, flags and counters
*/
static Macro		*macrosHead = NULL;
static Macro		*macrosTail = NULL;
static Macro		*currentmac = NULL;
static MacroLine	*curmacLine = NULL;
static bool			procmacro = false;	/* processing a macro? */
static bool			macroopen = false;	/* macro open */
static u_int16		macrooccur = 0;		/* macro local label occurance */
static int			macroline;
static Macro		*currentopenmac = NULL;
static OpenedMacro	openedmacros[50];
static int			macrosopened = 0;


bool IsMacroOpen()
{
	return (0 != macroopen);
}


bool IsProcessingMacro()
{
	return procmacro;
}


void ResetMacroLocalLabels()
{
	macrooccur = 0;	/* occurance of local labels in macros */
}


int GetNextMacroLocalLabel()
{
	return ++macrooccur;
}


int GetMacroLocalLabel()
{
	return macrooccur;
}


bool OnFirstMacroLine()
{
	return (NULL != currentopenmac && curmacLine == currentopenmac->m_Lines);
}


static Macro *allocMacro(void)
{
	Macro *temp;

	temp = (Macro*)CAllocMem(1, sizeof(Macro));

	return temp;
}


static MacroLine *allocMacroLine(void)
{
	MacroLine *temp;
	temp = (MacroLine*)CAllocMem(1, sizeof(MacroLine));
	macroline++;
	temp->m_Next = NULL;

	return temp;
}


static MacroVar *allocMacroVar(void)
{
	MacroVar *temp;

	temp = (MacroVar *)CAllocMem(1, sizeof(MacroVar));
	temp->m_Next = NULL;

	return temp;
}


static Macro *FindMacro(const char *name)
{
	Macro *mac = macrosHead;

	while(mac != NULL) {
		if(stricmp(name, mac->m_Name) == 0) return mac;
		mac = mac->m_Next;
	}
	return NULL;
}


void CreateMacro(EnvContext *ctx, const char *macroname, const char *macroargs)
{
	char delim;
	int vaflag;

	procmacro = true;
	macroline = 0;	/* reset macro line count */
	if(ctx->m_Pass == 2) return;

	/* FIXME - check lables, structs, and unions */
	if(FindMacro(macroname) != NULL) {
		error(ctx, ERR_REDEFINED, "'%s' has already been defined as a macro", macroname);
		return;
	}

	/* if macros have not been defined yet, allocate mem for one */
	if(macrosHead == NULL) {
		macrosHead = allocMacro();
		macrosTail = macrosHead;
		currentmac = macrosHead;
		curmacLine = NULL;	/* reset current macro line */
	}
	
	else { /* we already have macros, so lets allocate a new one */
		macrosTail->m_Next = allocMacro();
		macrosTail = macrosTail->m_Next;
		currentmac = macrosTail;
	}
	/*
		we have either created the initial macro or found the last
		slot and added to it.  Lets copy the contents
	*/
	currentmac->m_Next = NULL;				/* set the next macro and null */
	currentmac->m_Vars = NULL;				/* reset current variable pointer */
	currentmac->m_VarCount = 0;				/* no variables defined yet */
	strcpy(currentmac->m_Name, macroname);	/* copy over label as the name */

	vaflag = 0;

	do {
		char varBuffer[MAX_VAR_LENGTH + 1];
		char *dst;

		/* keep skipping if character isn't... */
		dst = varBuffer;
		while(EOS != *macroargs &&  ',' != *macroargs && false == IsWS(*macroargs) && false == IsCommentChar(ctx, *macroargs)) {
			/* Copy it */
			*(dst++) = *(macroargs++);
			if(dst - varBuffer >= MAX_VAR_LENGTH) {
				error(ctx, ERR_GENERAL, "Macro variable is too long");
				return;
			}

		}

		*dst = 0;

		if(true == IsCommentChar(ctx, *macroargs) || true == IsWS(*macroargs)) {
			vaflag++;
		}

		delim = *macroargs;	/* get current character */
		if(EOS != delim) {
			macroargs++;
		}

		if(strlen(varBuffer) == 0) {
			break;
		}

		AddVarToMacro(ctx, varBuffer);	/* add this variable to the macro */
	} while(delim == ',' && vaflag == 0);
}


void EndMacro(EnvContext *ctx)
{
	procmacro = false;	/* Add things needed to end this macro */
	currentmac = NULL;
	if(ctx->m_Pass == 2) {
		return;
	}

	curmacLine = NULL;	/* reset current macro line */
}


void AddLineToMacro(EnvContext *ctx, const char *macroline)
{

	/* FIXME
	if(true == HasLocalLabelChar(ctx, ctx->m_Label)) {
		error(ctx, ERR_SYNTAX, "macro declarations require the use of local labels", ctx->m_Label);
	} else {
		AddLineToMacro(ctx, ctx->m_Line);
	}
	*/

	/* if a macro is empty, allocate first line */
	if(ctx->m_Pass == 2) {
		return;
	}

	if(curmacLine == NULL) {
		currentmac->m_Lines = allocMacroLine();	/* allocate macros first line */
		curmacLine = currentmac->m_Lines;	/* set the current line to this one */
	}

	/* its got something in it, allocate next line */
	else { 
		curmacLine->m_Next = allocMacroLine();
		curmacLine = curmacLine->m_Next;
	}

	curmacLine->m_Next = NULL;
	strcpy(curmacLine->m_Line, macroline);
}


static void AddVarToMacro(EnvContext *ctx, const char *var)
{
	MacroVar *curmacvar;

	if(ctx->m_Pass == 2) {
		return;
	}

	if(currentmac->m_Vars == NULL) {			/* if no macro variables are defined, do first one */
		curmacvar = allocMacroVar();		/* allocate variable memory */
		currentmac->m_Vars = curmacvar;		/* set first variable pointer in macro def */
	}
	
	else {
		curmacvar = currentmac->m_Vars;		/* variables exist, point to first one */

		/* find last macro */
		while(curmacvar->m_Next != NULL) {	/* are we at the last one? */
			curmacvar = curmacvar->m_Next;	/* go to next one */
		}

		curmacvar->m_Next = allocMacroVar();	/* allocate next variable */
		curmacvar = curmacvar->m_Next;			/* point to it */
	}

	currentmac->m_VarCount++;			/* increase number of variables used in macro */
	strcpy(curmacvar->m_Var, var);	/* copy variable name */
}


static void SetVarValue(EnvContext *ctx, const int var, const char *value)
{
	int i;
	MacroVar *curmacvar;

	if(var >= currentopenmac->m_VarCount) {
		warning(ctx, WARN_MACROPARAMS, "too many parameters passed to macro [%03i:%03i]", var, currentopenmac->m_VarCount);
		return;
	}

	/* find last macro */
	curmacvar = currentopenmac->m_Vars;

	for(i = 0; i < var; i++) {
		/* skip to specified variable */
		curmacvar = curmacvar->m_Next;
	}

	strcpy(curmacvar->m_Value, value);
}


static const char *GetVarEnd(EnvContext *ctx, const char *varptr, char *varBuffer)
{
	bool	quote = false;

	/* if no more definitions, don't process */
	if(true == IsWS(*varptr) || EOS == *varptr || true == IsCommentChar(ctx, *varptr)) {
		return NULL;
	}

	while(true) {
		/* handle quotes */
		if(EOS == *varptr) {
			break;
		}
		
		else if(*varptr == '"') {

			if(quote == 0) {
				/* if we aren't in quotes, enter them */
				quote = true;
			}

			else {
				/* already in quotes, exit them */
				quote = false;	
			}
		}
		
		/* if not in quotes and a comma is there */
		else if(false == quote && (',' == *varptr || true == IsWS(*varptr))) {
			varptr++;
			break;
		}

		/* copy the character */
		*(varBuffer++) = *(varptr++);
	}

	*varBuffer = EOS;

	if(quote != 0) {
		error(ctx, ERR_SYNTAX, "unterminated quotes in macro variable definition");
	}

	
	return(varptr);
}


/* routines for opening and reading from a macro */
bool OpenMacro(EnvContext *ctx, const char *name, const char *macroargs)
{
	char varBuffer[MAX_VAR_LENGTH + 1];
	const char *vptr;
	const char *next;
	int count;
	Macro *temp;

	macroline = 0;
	temp = FindMacro(name);
	if(NULL == temp) {
		return false;
	}

	if(currentopenmac) {

		if(macrosopened > 49) {
			error(ctx, ERR_GENERAL, "too many macros opened");
			return false;
		}
		openedmacros[macrosopened].mac = currentopenmac;
		openedmacros[macrosopened].m_Line = curmacLine;
		macrosopened++;
	}

	currentopenmac = temp;
	macroopen = true;
	curmacLine = currentopenmac->m_Lines;

	vptr = macroargs;
	count = 0;
	if(currentopenmac->m_VarCount != 0) {
		while((next = GetVarEnd(ctx, vptr, varBuffer)) != NULL) {
			SetVarValue(ctx, count, varBuffer);
			count++;
			vptr = next;
		}
	}

	return(NULL != currentopenmac ? true : false);
}


void CloseMacro(void)
{
	if(macrosopened) {
		do {
			macrosopened--;
			currentopenmac = openedmacros[macrosopened].mac;
			curmacLine = openedmacros[macrosopened].m_Line;
		} while(macrosopened > 0 && NULL == curmacLine);
	} else {
		currentopenmac = NULL;
		curmacLine = NULL;
		macroopen = false;
	}

	GetNextMacroLocalLabel();
}


char *FindChar(char *vptr, char ch)
{
	while(*vptr != 0) {
		if(*vptr++ == ch) {
			return (vptr);
		}
	}
	return vptr;
}


/* find a macro variable */
char *GetMacroVarByName(char *varName)
{
	MacroVar *var;

	var = currentopenmac->m_Vars;

	while(var != NULL) {
		if(strcmp(var->m_Var, varName) == 0) {
			return (var->m_Value);
		}
		var = var->m_Next;
	}
	return (NULL);
}


char *GetMacroVarByIndex(int index)
{
	MacroVar *var;

	var = currentopenmac->m_Vars;

	while(var != NULL && index > 0) {
		index--;	
		var = var->m_Next;
	}

	return (NULL == var ? NULL : var->m_Value);
}


char *GetMacroLine(EnvContext *ctx, char *outBufferPtr)
{
	outBufferPtr[0] = EOS;

	if(curmacLine == NULL) {
		CloseMacro();
		if(curmacLine) {
			return(GetMacroLine(ctx, outBufferPtr));
		}
		return(NULL);
	}

	if(currentopenmac->m_VarCount == 0) {
		strcpy(outBufferPtr, curmacLine->m_Line);
	}

	else {
		char	*src;
		char	*dst;

		src = curmacLine->m_Line;	/* get start of macro line */
		dst = outBufferPtr;
		while(*src != 0x00) {

			/* Handle Macro-80c variable access */
			if('\\' == *src) {

				src++;

				/* If we hit a period assume that it's some kind of internal or local label */
				if('.' == *src) {
					src++;

					/* KLUDGE - convert to a non-standard local label */
					*(dst++) = '@';
					while(IsStructLabelChar(ctx, *src)) {
						*(dst++) = *(src++);
					}
					*dst = 0;
				}
				
				/* Check for parameter access */
				else if(isdigit(*src)) {
					int index;
					char *value;

					index = atoi(src++);

					value = GetMacroVarByIndex(index);
					if(NULL == value) {
						error(ctx, ERR_UNDEFINED, "No variable at index %d", index);
						sprintf(outBufferPtr, "%s", src - 2);
						curmacLine = curmacLine->m_Next;
						return outBufferPtr;
					}
					strcpy(dst, value);
					dst += strlen(value);

				}

				else {
				}
			}

			else if(*src == '{') {
				char	*var;
				char	temp[100];

				src++;
				var = temp;
				/* search for end of variable */
				while(*src != '}') {
					if(true == IsWS(*src) || true == IsCommentChar(ctx, *src)) {
						error(ctx, ERR_SYNTAX, "illegal variable assignment in macro '%s'", currentopenmac->m_Name);
						*var = 0;
						curmacLine = curmacLine->m_Next;
						sprintf(dst, "{%s%s", var, src);
						return outBufferPtr;
					}
					if(EOS == *src) {
						error(ctx, ERR_SYNTAX, "unterminated variable assignment in macro '%s'", currentopenmac->m_Name);
						curmacLine = curmacLine->m_Next;
						sprintf(dst, "{%s%s", var, src);
						return outBufferPtr;
					}
					*var++ = *src++;
				}
				*var = 0;	/* set end of variable */
				var = GetMacroVarByName(temp);	/* go find the variable */
				if(var == NULL) {
					error(ctx, ERR_UNDEFINED, "unknown variable '%s' in macro '%s'", temp, currentopenmac->m_Name);
					curmacLine = curmacLine->m_Next;
					sprintf(dst, "{%s%s", temp, src);
					return outBufferPtr;
				} else {
					strcpy(dst, var);
					dst += strlen(var);
				}
				src++;
			}
			
			else {
				*(dst++) = *(src++);
			}
		}
		*dst = 0;
	}

	curmacLine = curmacLine->m_Next;

	return outBufferPtr;
}

