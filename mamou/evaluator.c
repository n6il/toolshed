/*
 * evaluator.c: expression evaluation routines
 *
 *      an expression is constructed like this:
 *
 *      expr ::=  expr + term |
 *                expr - term ;
 *                expr * term ;
 *                expr / term ;
 *                expr | term ;
 *                expr & term ;
 *                expr % term ;
 *                expr ^ term ;
 *
 *      term ::=  symbol |
 *                * |
 *                constant ;
 *
 *      symbol ::=  string of alphanumerics with non-initial digit
 *
 *      constant ::= hex constant |
 *                   binary constant |
 *                   octal constant |
 *                   decimal constant |
 *                   ascii constant;
 *
 *      hex constant ::= '$' {hex digits};
 *
 *      octal constant ::= '@' {octal digits};
 *
 *      binary constant ::= '%' { 1 | 0 };
 *
 *      decimal constant ::= {decimal digits};
 *
 *      ascii constant ::= ''' any printing char;
 *
 */

#include "mamou.h"


/* Static functions */

static BP_Bool get_term(assembler *as, BP_int32 *term, BP_char **eptr, BP_Bool ignoreUndefined);
static BP_Bool is_op(BP_char c);


/* Static variables */

static BP_Bool forward = BP_FALSE;



BP_Bool evaluate(assembler *as, BP_int32 *result, BP_char **eptr, BP_Bool ignoreUndefined)
{
	BP_int32	left, right;     /* left and right terms for expression */
	BP_char		o;					/* operator character */


	if (as->o_debug)
	{
		printf("Evaluating %s\n", *eptr);
	}
	
	as->force_byte = BP_FALSE;
	as->force_word = BP_FALSE;

	if (**eptr == '<')
	{
		as->force_byte = BP_TRUE;
		(*eptr)++;
	}
	else if (**eptr == '>')
	{
		as->force_word = BP_TRUE;
		(*eptr)++;
	}


	/* Pickup first part of expression. */
	
	if ((get_term(as, &left, eptr, ignoreUndefined) == BP_FALSE) && (forward == BP_FALSE))
	{
//		*result = Dp * 256 + 0xfe;
		*result = 0;
//		return BP_FALSE;
	}


	while (is_op(**eptr))
	{
		/* 1. Pickup operator and skip. */
		
 		o = *((*eptr)++);


		/* 2. Pickup current rightmost side. */
		
		if (get_term(as, &right, eptr, ignoreUndefined) == BP_FALSE)
		{
//			*result = Dp * 256 + 0xfe;
			*result = 0;
//			return BP_FALSE;
		}
		
		
		/* 3. Process left/right according to operator. */
		
		switch (o)
		{
			case '+':
				left += right;
				break;
			case '-':
				left -= right;
				break;
			case '*':
				left *= right;
				break;
			case '/':
				left /= right;
				break;
			case '|':
			case '!':
				left |= right;
				break;
			case '&':
				left &= right;
				break;
			case '%':
				left %= right;
				break;
			case '^':
				left = left ^ right;
				break;
		}
	}


	/* 4. Assign result to left. */
	
	*result = left;


	/* 5. Print debugging information if requested. */
	
	if (as->o_debug)
	{
		printf("Result     $%x\n", (int)*result);
		printf("force_byte %d\n", as->force_byte);
		printf("force_word %d\n", as->force_word);
	}


	/* 6. Return status. */
	
	return BP_TRUE;
}



/*
 *      is_op --- is character an expression operator?
 */
static BP_Bool is_op(BP_char c)
{
	if (any(c, "+-*/&%|^!"))
	{
		return BP_TRUE;
	}
	
	
	return BP_FALSE;
}



/*
 *      get_term --- evaluate a single item in an expression
 */
static BP_Bool get_term(assembler *as, BP_int32 *term, BP_char **eptr, BP_Bool ignoreUndefined)
{
	char			hold[MAXBUF];
	char			*tmp;
	BP_int32		val = 0;				/* local value being built */
	BP_Bool			minus = BP_FALSE;		/* unary minus flag */
	struct nlist	*pointer;
	struct link		*pnt, *bpnt;


	forward = BP_FALSE;

	if (!**eptr)
	{
		error(as, "Unbalanced expression");

		return BP_FALSE;
	}
	else if (**eptr == '-')
	{
		(*eptr)++;
		minus = BP_TRUE;
	}


	/* Open brace? */
	
	if (**eptr == '(')
	{
		(*eptr)++;
		evaluate(as, &val, eptr, ignoreUndefined); /* evaluate the inside */
		if (**eptr != ')')
		{
			error(as, "Mismatched brace");
			return BP_FALSE;
		}
	}
	else if (**eptr == ')')
	{
		error(as, "Mismatched brace");
		return BP_FALSE;
	}


	/* Skip over immediate character. */
	
	while (**eptr == '#')
	{
		(*eptr)++;
	}
	
	
	/* Complement? */
	
	if (**eptr == '^')
	{
		BP_int32	term2;
		BP_Bool		ret;


		/* 1. Complement next term. */

		(*eptr)++;
		ret = get_term(as, &term2, eptr, ignoreUndefined);
		*term = ~term2;


		return ret;
	}
	

	/* Binary constant? */
	
	if (**eptr == '%')
	{
		(*eptr)++;

		while (any(**eptr, "01"))
		{
			val = (val * 2) + ((*((*eptr)++)) - '0');
		}
	}


	/* Octal constant? */
	
	else if (**eptr == '@')
	{
		(*eptr)++;

		while (any(**eptr, "01234567"))
		{
			val = (val * 8) + ((*((*eptr)++)) - '0');
		}
	}


	/* Hexadecimal constant? */
	
	else if (**eptr == '$')
	{
		(*eptr)++;

		while (any(**eptr, "0123456789abcdefABCDEF"))
		{
			if (**eptr > '9')
			{
				val = (val * 16) + 10 + (mapdn(*((*eptr)++)) - 'a');
			}
			else
			{
				val = (val * 16) + ((*((*eptr)++)) - '0');
			}
		}
	}


	/* Decimal constant? */
	
	else if (any(**eptr, "0123456789"))
	{
		while (**eptr >= '0' && **eptr <= '9')
		{
			val = (val * 10) + ((*((*eptr)++)) - '0');
		}
	}


	/* Current program counter? */
	
	else if (**eptr == '*')
	{
		(*eptr)++;
		val = as->old_program_counter;
	}


	/* Current data counter? */
	
	else if (**eptr == '.')
	{
		(*eptr)++;
		val = as->data_counter;
	}
	
	
	/* Character literal? */

	else if (**eptr == '\'')
	{
		(*eptr)++;

		if (**eptr == EOS)
		{
			val = 0;
		}
		else
		{
			val = *((*eptr)++);
		}
	}


	/* Symbol? */
	
	else if (alpha(**eptr))
	{
		/* 1. Let's collect the symbol name. */

		tmp = hold;

		while (alphan(**eptr))
		{
			*tmp++ = *((*eptr)++);
		}

		*tmp = EOS;

		pointer = symbol_find(as, hold, ignoreUndefined);

		if (pointer != NULL)
		{
			if (as->pass == 2)
			{
				pnt = pointer->L_list;
				bpnt = NULL;
				while (pnt != NULL)
				{
					bpnt = pnt;
					pnt = pnt->next;
				}
				
				if (BP_kal_mem_alloc((void **)&pnt, sizeof(struct link)) != BPE_OK)
				{
					pointer->L_list = pnt;
				}
				else
				{
					bpnt->next = pnt;
				}

				pnt->L_num = as->file_stack[as->file_stack_index].current_line;
				pnt->next = NULL;
			}

			val = as->last_symbol;
		}
		else
		{
			if (as->pass == 1)
			{
				/* forward ref here */
				fwd_mark(as);
#if 0
				if (force_byte == NO)
				{
					force_word = BP_TRUE;
				}
#endif
				forward = BP_TRUE;
				*term = 0;
				return BP_FALSE;
			}
		}
		if (as->pass == 2 && as->file_stack[as->file_stack_index].current_line == as->F_ref && as->current_filename_index == as->Ffn)
		{
#if 0
			if (as->force_byte == NO)
			{
				as->force_word = BP_TRUE;
			}
#endif
			fwd_next(as);
		}
	}
	else if (**eptr == ')')
	{
		(*eptr)++;
	}
	else
	{
		/* none of the above */
		val = 0;
	}

	if (minus)
	{
		*term = -val;
		return(BP_TRUE);
	}
	else
	{
		*term = val;
		return(BP_TRUE);
	}
}
