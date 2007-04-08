/*****************************************************************************
	struct.c	- 'struct' implementation

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

bool			instruct = false;
static bool		procflag = false;
static Struct	*structhead = NULL;
static Struct	*structtail = NULL;
Struct			*laststructfound = NULL;
static Struct	*unionStack = NULL;






static void AddElement(EnvContext *ctx,
					   Struct *astruct, 
					   const char *name,
					   int size,
					   int count,
					   Struct *child)
{
	Element		*el;

	/* See if the element name is already used */
	el = StructGetElement(astruct, name);
	if(NULL != el) {
		error(ctx, ERR_REDEFINED, "'%s' member already in defined in structure '%s'", name, astruct->name);
		return;
	}

	/* Allocate element */
	el = (Element*)AllocMem(sizeof(Element));

	/* Assign info to element */
	strcpy(el->name, name);
	el->offset = astruct->size;
	el->size = size;
	el->count = count;
	el->child = child;
	el->next = NULL;

	if(astruct == unionStack) {
		int fullSize;

		if(NULL != child) {
			fullSize = child->size;
		} else {
			fullSize = size * count;
		}

		if(fullSize > astruct->size) {
			astruct->size = fullSize;
		}

		el->offset = 0;
	} else {
		/* Increase the size of the structure */
		astruct->size += (size * count);
	}

	/* Add element to list */
	if(NULL == astruct->el_head) {
		astruct->el_head = el;
	}

	if(NULL != astruct->el_tail) {
		astruct->el_tail->next = el;
	}

	astruct->el_tail = el;

}



void StructCreate(EnvContext *ctx, const char *structName)
{
	Struct *tempstruct;
	Symbol *structSym;

	/* Set procflag to false in case of error */
	procflag = false;

	/* Make sure we have a name for the structure */
	if(*structName == EOS) {
		error(ctx, ERR_SYNTAX, "label required for structure declaration");
		return;
	}

	/* Make sure struct does not conflict with an operant name */
	if(mne_look(ctx, structName) != NULL) {
		error(ctx, ERR_SYNTAX, "structure name conflicts with a standard operand name");
		return;
	}

	/* Allocate memory for structure */
	tempstruct = (Struct*)AllocMem(sizeof(Struct));

	strcpy(tempstruct->name, structName);

	/* Install structure name as symbol */
	structSym = AddSymbol(ctx, tempstruct->name, 0, SYM_STRUCTURE, tempstruct, 0);
	if(NULL == structSym) {
		free(tempstruct);
		return;
	}

	/* Add to struct list */
	if(structhead == NULL) {
		structhead = tempstruct;
	}

	else {
		structtail->next = tempstruct;
	}

	structtail = tempstruct;
	structtail->size = 0;
	structtail->next = NULL;
	structtail->el_head = NULL;
	structtail->el_tail = NULL;

	/* Reset processing flags */
	procflag = true;
}



void UnionCreate(EnvContext *ctx, const char *unionName)
{
	Struct *tempstruct;

	ASSERTX(NULL == unionStack);

	tempstruct = (Struct*)AllocMem(sizeof(Struct));

	strcpy(tempstruct->name, unionName);
	tempstruct->size = 0;
	tempstruct->el_head = NULL;
	tempstruct->el_tail = NULL;
	tempstruct->next = NULL;
	AddElement(ctx, structtail, unionName, 0, 1, tempstruct);
	unionStack = tempstruct;
}


void UnionEnd(EnvContext *ctx)
{
	ASSERTX(NULL != unionStack);
	structtail->size += unionStack->size;
	unionStack = NULL;

	/* FIXME - reset size of union */
}



/*
	FIXME - check for elements with no name!
*/
void StructAddElement(EnvContext *ctx, const char *elementName, const char *elementType)
{
	int32		result;
	const		Mneumonic *opcode;
	int			size;
	int			count;
	Struct		*addTo;

	ASSERTX(NULL != structtail);

	/* Go find opcode */
	opcode = mne_look(ctx, elementType);

	if(NULL == opcode) {
		error(ctx, ERR_SYNTAX, "invalid opcode in structure declaration");
		return;
	}

	size = 0;

	if(ENDUNION == opcode->opcode) {
		if(1 == ctx->m_Pass) {
			if(NULL == unionStack) {
				error(ctx, ERR_GENERAL, "endunion with no union declaration");
				return;
			}

			UnionEnd(ctx);
		}
		return;
	}


	/* If we are at the end of a structure, close it */
	if(ENDSTRUCT == opcode->opcode) {
		if(procflag == true) {
			if(ctx->m_Pass == 1 && structtail->size == 0) {
				warning(ctx, WARN_EMPTYSTRUCT, "structure declaration empty");
			}
		}
		instruct = false;
		procflag = false;
		laststructfound = NULL;
		return;
	}

	/* If on pass 2, do not process elements */
	if(ctx->m_Pass == 2 || procflag == false) {
		return;
	}

	/* If no opcode error */
	if(EOS == *ctx->m_Ptr) {
		error(ctx, ERR_SYNTAX, "structure element require operand");
		return;
	}

	if(opcode->optype != PSEUDO) {
		error(ctx, ERR_SYNTAX, "illegal mnemonic in structure");
		return;
	}

	/* Add the element */
	switch(opcode->opcode) {
	case ZMB:
	case FCB:
		size = 1;
		break;

	case FDB:
	case ZMD:
		size = 2;
		break;

	case FQB:
	case ZMQ:
		size = 4;
		break;

	case RMB:
		size = 1;
		break;

	case STRUCTDEC:
		ASSERTX(NULL != laststructfound);
		ASSERTX(0 == strcmp(laststructfound->name, elementType));
		size = laststructfound->size;
		break;

	case UNION:
		if(NULL != unionStack) {
			error(ctx, ERR_GENERAL, "cascading unions not allowed");
			return;
		}
		UnionCreate(ctx, elementName);
		return;

	case STRUCT:
		error(ctx, ERR_GENERAL, "cascading structures not allowed");
		return;

	default:
		error(ctx, ERR_GENERAL, "unsupported data type or operand found in member declaration");
		return;
	}

	if(false == IsCommentChar(ctx, *ctx->m_Ptr) && 0 != *ctx->m_Ptr) {
		if(true == Evaluate(ctx, &result, EVAL_NORMAL, NULL)) {
			count = result;
			if(size < 0) {
				error(ctx, ERR_SYNTAX, "element declarations must have a count greater than 0");
				count = 1;
			}
		} else {
			error(ctx, ERR_SYNTAX, "invalid expression for declaration");
			return;
		}
	} else {
		count = 1;
	}

	if(NULL != unionStack) {
		addTo = unionStack;
	} else {
		addTo = structtail;
	}


	if(0 != size) {
		AddElement(ctx, addTo, elementName, size, count, laststructfound);
	}
}


Struct *StructLookup(const char *name)
{
	Struct *list;

	laststructfound = NULL;
	list = structhead;

	while(list) {
		if(strcmp(list->name, name) == 0) {
			return(laststructfound = list);
		}
		list = list->next;
	}
	return(laststructfound = NULL);
}



Element *StructGetElement(const Struct *astruct, const char *elname)
{
	Element *el;

	el = astruct->el_head;

	while(el) {
		if(0 == strcmp(elname, el->name)) {
			return(el);
		}
		el = el->next;
	}
	return(NULL);
}


