/***************************************************************************
* symbol_bucket.c: symbol handle routines
*
* $Id$
*
* The Mamou Assembler - A Hitachi 6309 assembler
*
* (C) 2004 Boisy G. Pitre
***************************************************************************/

#include "mamou.h"
#include "h6309.h"
#include "pseudo.h"
#include "util.h"


/*!
	@function symbol_add
	@discussion Adds a symbol to the symbol bucket
	@param as The assembler state structure
	@param name The name of the symbol to be added
	@param val The value of the symbol
	@param override A flag which allows the changing of an existing symbol
 */
struct nlist *symbol_add(assembler *as, char *name, int val, int override)
{
	struct link		*lp;
	struct nlist	*np, *p, *backp;
	int				i;
	char			tmp_label[MAXLAB];

	/* 1. Does the symbol name meet our criteria? */	
	if (!alpha(*name) && *name != '@')
	{
		error(as, "Illegal Symbol Name");

		return NULL;
	}

	/* 2. If it's a temporary symbol, generate a unique symbol name based on
     *    current file index and number of blank lines.
	 */
//	if (name[strlen(name) - 1] == '@')
	if (strchr(name, '@') != NULL)
	{
		sprintf(tmp_label, "_tmp%s%04X%04X", name, (int)as->use_depth, (int)as->current_file->num_blank_lines);
		
		name = tmp_label;
	}
	
	/* See if the value is already defined. */
	if ((np = symbol_find(as, name, 0)) != NULL)
	{
		/* 1. Symbol has been defined already -- is this pass 2? */
		if (as->pass == 2)
		{
			/* 1. It's pass 2 -- determine if the value has changed from pass 1. */			
			if (np->def == val || override == 1)
			{
				/* 1. Is the existing variable is overridable? */				
				if (np->overridable == 1)
				{
					/* 1.  Yes, so we'll override it with new passed value. */					
					np->def = val;
				}
				
				return np;
			}
			else
			{
				/* 1. The value is different and we can't override -- it's a phasing error. */				
				error(as, "Phasing Error");

				return NULL;
			}
		}

		/* If we're here, it's pass 1 -- is the existing symbol overridable? */		
		if (np->overridable == 1)
		{
			/* 1. Yes it is. */			
			np->def = val;
			
			return np;
		}
		else
		{
			/* 2. No, it's not overridable. */			
			if (override == 0)
			{
				error(as, "Symbol Redefined");
			}
			
			return NULL;
		}
	}

	/* 3. It's not an existing symbol, so we'll add it to the bucket. */
	if (as->o_debug)
	{
		 printf("Installing %s as $%x\n", name, val);
	}

	/* 4. Allocate memory for a symbol entry. */	
	np = (struct nlist *)malloc(sizeof(struct nlist));
	if (np == NULL)
	{
		error(as, "Symbol table full");

		return NULL;
	}
	
	/* 5. Allocate memory for the symbol name. */
	np->name = (char *)malloc(strlen(name) + 1);
	if (np->name == NULL)
	{
		error(as, "Symbol table full");

		return NULL;
	}

	/* 6. Set up the symbol entry with the appropriate information. */
	strcpy(np->name, name);
	np->def = val;
	np->Lnext = NULL;  
	np->Rnext = NULL;
	np->overridable = override;

	/* 7. Allocate a link. */
	lp = (struct link *)malloc(sizeof(struct link));
	if (lp == NULL)
	{
		return NULL;
	}
	
	np->L_list = lp;

	if (as->current_file != NULL)
	{
		lp->L_num = as->current_file->current_line;
	}
	else
	{
		/* 1. Symbol was defined on the command line. */		
		lp->L_num = 0;
	}
	
	lp->next = NULL;
	p = as->bucket;
	backp = NULL;

	/* 8. Insert the symbol into the table in alphabetical order. */
	while (p != NULL) 
	{
		backp = p;
		
#ifdef CASE_SENSITIVE
		i = strcmp(name, p->name);
#else
		i = strcasecmp(name, p->name);
#endif
		if (i < 0)
		{
			p = p->Lnext;
		}
		else
		{
			p = p->Rnext;
		}
	}
	if (backp == NULL)
	{
		as->bucket = np;
	}
#ifdef CASE_SENSITIVE
	else if (strcmp(name, backp->name) < 0)
#else
	else if (strcasecmp(name, backp->name) < 0)
#endif
	{
		backp->Lnext = np;
	}
	else
	{
		backp->Rnext = np;
	}

	/* 9. We're done, and we were successful. */	
	return np;  
}


/*!
	@function symbol_find
	@discussion Finds a symbol in the symbol bucket
	@param as The assembler state structure
	@param name Name of the symbol to search for
	@param ignoreUndefined Ignore the symbol if it is not found
 */
struct nlist *symbol_find(assembler *as, char *name, int ignoreUndefined)
{
	struct nlist *np;
	int     i;
	char			tmp_label[MAXLAB];
	
	/* 1. If it's a temporary symbol that hasn't had the _tmp tag prepended,
	 *    then generate a unique symbol name based on the current use depth
	 *    and number of blank lines.
	 */	
	if (strchr(name, '@') != NULL && strncmp(name, "_tmp", 4) != 0)
	{
		sprintf(tmp_label, "_tmp%s%04X%04X", name, (int)as->use_depth, (int)as->current_file->num_blank_lines);
		
		name = tmp_label;
	}
	
	np = as->bucket;

	while (np != NULL)
	{
#ifdef CASE_SENSITIVE
		i = strcmp(name, np->name);
#else
		i = strcasecmp(name, np->name);
#endif
		if (i == 0)
		{
			as->last_symbol = np->def;
			return(np);
		}
		else if (i < 0)
		{
			np = np->Lnext;
		}
		else
		{
			np = np->Rnext;
		}
	}

	as->last_symbol = 0;

	if (as->pass == 2 && ignoreUndefined == 0)
	{
		error(as, "symbol undefined on pass 2");
	}
	
	return(NULL); 
}


#define NMNE (sizeof(table) / sizeof(struct h6309_opcode))
#define NPSE (sizeof(pseudo) / sizeof(struct pseudo_opcode))

/*!
	@function mne_look
	@discussion Looks up a mnemonic
	@param as The assembler state structure
	@param str A pointer to the text of the mnemonic
	@param m A pointer to the mnemonic structure
 */
int mne_look(assembler *as, char *str, mnemonic *m)
{
	struct h6309_opcode		*low, *high, *mid;
	struct pseudo_opcode	*plow, *phigh, *pmid;
	int						cond;
	
	/* Assume opcode is unknown. */	
	m->type = OPCODE_UNKNOWN;

	/* Search machine mnemonics first. */
	low =  &table[0];
	high = &table[NMNE - 1];
	while (low <= high)
	{
		mid = low + (high - low) / 2;
		if ((cond = strcasecmp(str, mid->mnemonic)) < 0)
		{
			high = mid - 1;
		}
		else if (cond > 0)
		{
			low = mid + 1;
		}
		else
		{
			if (as->o_cpuclass < mid->cpuclass)
			{
				return 1;
			}

			m->type = OPCODE_H6309;
			m->opcode.h6309 = mid;
			
			return 0;
		}
	}

	/* Check for pseudo ops. */
	plow =  &pseudo[0];
	phigh = &pseudo[NPSE - 1];
	
	while (plow <= phigh)
	{
		pmid = plow + (phigh - plow) / 2;

		if ((cond = strcasecmp(str, pmid->pseudo)) < 0)
		{
			phigh = pmid - 1;
		}
		else if (cond > 0)
		{
			plow = pmid + 1;
		}
		else
		{
			m->type = OPCODE_PSEUDO;
			m->opcode.pseudo = pmid;			
			
			return 0;
		}
	}

	return 1;
}


static void symbol_dump_bucket_r(struct nlist *ptr);
static unsigned int	counter;

/*!
	@function symbol_bucket_dump
	@discussion Prints the symbol table in alphabetical order
	@param ptr Pointer to the symbol bucket tree
 */
void symbol_dump_bucket(struct nlist *ptr)
{
	/* 1. Reset the counter. */	
	counter = 0;

	/* 2. Print the symbol table heading. */	
	printf("Symbol table:\n");
	
	/* 3. Do the dump. */	
	symbol_dump_bucket_r(ptr);
}


static void symbol_dump_bucket_r(struct nlist *ptr)
{
	if (ptr != NULL)
	{
		symbol_dump_bucket_r(ptr->Lnext);
		
		printf("%-10s $%04X", ptr->name, (int)ptr->def);
		
		counter++;
		
		if (counter >= 4)
		{
			printf("\n");
			
			counter = 0;
		}
		else
		{
			printf("     ");
		}
		
		symbol_dump_bucket_r(ptr->Rnext);
	}
		
	return;
}


/*!
	@function symbol_cross_reference
	@discussion Prints the cross reference table
	@param ptr Pointer to the symbol table
 */
static void symbol_cross_reference_r(struct nlist *ptr);

void symbol_cross_reference(struct nlist *ptr)
{
	/* 1. Print the heading. */	
	printf("Cross-Reference table:\n");
	
	/* 2. Do the cross reference. */	
	symbol_cross_reference_r(ptr);
}


static void symbol_cross_reference_r(struct nlist *ptr)
{
	struct link *tp;
	int i = 1;
		
	if (ptr != NULL)
	{
		symbol_cross_reference_r(ptr->Lnext);
		
		printf("%-10s ($%04X) referenced from lines ", ptr->name, (int)ptr->def);
		
		tp = ptr->L_list;
		
		while (tp != NULL)
		{
			if (i++ > 10)
			{
				i = 1;
				
				printf("\n                      ");
			}
			
			printf("%05d ", (int)tp->L_num);
			
			tp = tp->next;
		}
		
		printf("\n");
		
		symbol_cross_reference_r(ptr->Rnext);
	}
		
	return;
}
