/*****************************************************************************
	symtab.h	- declaration for the symbol table

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
#ifndef SYMTAB_H
#define SYMTAB_H

#include "config.h"
#include "struct.h"
#include "output.h"

/* Label types */
typedef enum {
	SYM_STRUCTURE = 0,
    SYM_STRUCTDATA,
	SYM_UNION,
	SYM_ADDRESS,
	SYM_VALUE,
	SYM_INTERNAL_VALUE,
	SYM_IMPORTED
} SYMBOL_TYPE;


typedef struct _symbol Symbol;
typedef struct _line Line;



/* linked list to hold line numbers */
struct _line {
	int 	L_num;		/* line number */
	Line	 *next;		/* pointer to next node */
};


/* basic symbol table entry */
struct _symbol {
	SEGMENT		segment;	/* Data segment the symbol was declared in */
	SYMBOL_TYPE	type;		/* Type of symbol */
	char		*name;		/* symbol name */
	int32		value;		/* Value of the symbol */
	Struct		*astruct;	/* Pointer to structure */
	int			stcount;	/* Structure count */
	Symbol		*Lnext;		/* left node of the tree leaf */
	Symbol		*Rnext;		/* right node of the tree leaf */
	Line		*L_list;	/* pointer to linked list of line numbers */
};

/*------------------------------------------------------------------------
	symtab.c
------------------------------------------------------------------------*/
void InitSymbolTable(EnvContext *ctx);
void ResetLocalLabels(EnvContext *ctx);
Symbol *AddSymbol(EnvContext *ctx, const char *str, const int val, const SYMBOL_TYPE type, Struct *astruct, const int astructCount);
Symbol *FindSymbol(EnvContext *ctx, const char *name, const bool noerror, const bool onlyns);
void AddExport(EnvContext *info, const char *name);
void EmitExports(EnvContext *ctx, FILE *outputFile);
void InitExports(EnvContext *ctx);
void SetExportAddress(const int mmuPage, const u_int16 address);
int GetNextLocalLabel();
void DumpSymTable(EnvContext *ctx);
void DumpCrossRef();
void SetNamespace(EnvContext *ctx, const char *ns);
void ClearNamespace(EnvContext *ctx);
bool IsInNamespace();


#endif	/* SYMTAB_H */
