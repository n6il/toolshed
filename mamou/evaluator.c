/***************************************************************************
 * evaluator.c: expression evaluation routines
 *
 * $Id$
 *
 * The Mamou Assembler - A Hitachi 6309 assembler
 *
 * (C) 2004 Boisy G. Pitre
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
 ***************************************************************************/

#include "mamou.h"


/* Static functions */
static int get_term(assembler *as, int *term, char **eptr, int ignoreUndefined);
static int is_op(char c);


/* Static variables */
static int forward = 0;


/*!
	@function evaluator
	@discussion Evaluates mathematical expressions and symbols
	@param as The assembler state structure
	@param result A pointer to the result of the evaluated expression
	@param eptr The expression to be evaluated
	@param ignoreUndefined Ignore any undefined symbols
	@result 1=evaluation was made ok
 */
int evaluate(assembler *as, int *result, char **eptr, int ignoreUndefined)
{
	int			left = 0;
	int			right = 0;			/* left and right terms for expression */
	char		o;					/* operator character */

	/* 1. Do any debugging output. */
	if (as->o_debug)
	{
		printf("Evaluating %s\n", *eptr);
	}	
	
	/* 2. Assume no forcing of result size. */
	as->line.force_byte = 0;
	as->line.force_word = 0;
	
	/* 3. Force byte or word size? */
	if (**eptr == '<')
	{
		as->line.force_byte = 1;
		(*eptr)++;
	}
	else
	if (**eptr == '>')
	{
		as->line.force_word = 1;
		(*eptr)++;
	}

	/* 4. Pickup first part of expression. */
	if ((get_term(as, &left, eptr, ignoreUndefined) == 0) && (forward == 0))
	{
//		*result = Dp * 256 + 0xfe;
		*result = 0;
//		return 0;
	}

	/* 5. Gather rest of line. */
	while (is_op(**eptr))
	{
		/* 1. Pickup operator and skip. */
 		o = *((*eptr)++);

		/* 2. Pickup current rightmost side. */		
		if (get_term(as, &right, eptr, ignoreUndefined) == 0)
		{
//			*result = Dp * 256 + 0xfe;
			*result = 0;
//			return 0;
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

	/* 6. Assign result to left. */	
	*result = left;

	/* 7. Print debugging information if requested. */	
	if (as->o_debug)
	{
		printf("Result     $%x\n", (int)*result);
		printf("force_byte %d\n", as->line.force_byte);
		printf("force_word %d\n", as->line.force_word);
	}

	/* 8. Return status. */	
	return 1;
}



/*!
	@function is_op
	@discussion Determines if a character is an expression operator
	@param c Character to evaluate
 */
static int is_op(char c)
{
	if (any(c, "+-*/&%|^!"))
	{
		return 1;
	}	
	
	return 0;
}


/*!
	@function get_term
	@discussion Evaluates a single item in an expression
	@param as The assembler state structure
	@param term A pointer to the term that is returned
	@param eptr The address of a pointer to the expression to be evaluated
	@param ignoreUndefined Ignore any undefined symbols
 */
static int get_term(assembler *as, int *term, char **eptr, int ignoreUndefined)
{
	char			hold[MAXBUF];
	char			*tmp;
	int				val = 0;				/* local value being built */
	int				minus = 0;				/* unary minus flag */
	int				complement = 0;			/* complement flag */
	struct nlist	*pointer;
	struct link		*pnt, *bpnt;

	forward = 0;

	/* 1. If we encounter end of string, something's wrong. */
	if (!**eptr)
	{
		error(as, "Unbalanced expression");

		return 0;
	}	
	/* 2. A leading minus is a negation. */	
	else if (**eptr == '-')
	{
		(*eptr)++;

		minus = 1;
	}
	/* 3. A leading tilde is a complement. */
	else if (**eptr == '~')
	{
		(*eptr)++;

		complement = 1;
	}

	/* 4. Open brace? */	
	if (**eptr == '(')
	{
		(*eptr)++;

		evaluate(as, &val, eptr, ignoreUndefined); /* evaluate the inside */

		if (**eptr != ')')
		{
			error(as, "Mismatched brace");

			return 0;
		}
	}
	else if (**eptr == ')')
	{
		error(as, "Mismatched brace");

		return 0;
	}

	/* 5. Skip over immediate character. */
	while (**eptr == '#')
	{
		(*eptr)++;
	}
	
	/* 6. Complement? */
	if (**eptr == '^')
	{
		int	term2;
		int		ret;

		/* 1. Complement next term. */
		(*eptr)++;
		ret = get_term(as, &term2, eptr, ignoreUndefined);
		*term = ~term2;

		return ret;
	}

	/* 7. Binary constant? */
	if (**eptr == '%')
	{
		(*eptr)++;

		while (any(**eptr, "01"))
		{
			val = (val * 2) + ((*((*eptr)++)) - '0');
		}
	}
	/* 8. Octal constant? */
	else if (**eptr == '@')
	{
		(*eptr)++;

		while (any(**eptr, "01234567"))
		{
			val = (val * 8) + ((*((*eptr)++)) - '0');
		}
	}
	/* 9. Hexadecimal constant? */
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
	/* 10. Decimal constant? */	
	else if (any(**eptr, "0123456789"))
	{
		while (**eptr >= '0' && **eptr <= '9')
		{
			val = (val * 10) + ((*((*eptr)++)) - '0');
		}
	}
	/* 11. Current program counter? */
	else if (**eptr == '*')
	{
		(*eptr)++;
		val = as->old_program_counter;
	}
	/* 12. Current data counter? */
	else if (**eptr == '.')
	{
		(*eptr)++;
		val = as->data_counter;
	}
	/* 13. Character literal? */
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
	else if (**eptr == '"')
	{
		/* double character literal */
		(*eptr)++;
		if (**eptr == EOS)
		{
			val = 0;
		}
		else
		{
			val = *((*eptr)++);
			if (**eptr == EOS)
			{
				val = 0;
			}
			else
			{
				val = val<<8;
				val += *((*eptr)++);
			}
		}
	}
	/* 14. Symbol? */
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
				
				pnt = (struct link *)malloc(sizeof(struct link));
				if (pnt == NULL)
				{
					pointer->L_list = pnt;
				}
				else
				{
					bpnt->next = pnt;
				}

				pnt->L_num = as->current_file->current_line;
				pnt->next = NULL;
			}

			val = as->last_symbol;
		}
		else
		{
			if (as->pass == 1)
			{
				/* NEW!! 04/09/05 - if symbol begins with
				 * _$, assume it is an environment var
				 */
				if (hold[0] == '_' && hold[1] == '$')
				{
					char *p;

					p = getenv(&(hold[2]));

					if (p != NULL)
					{
						symbol_add(as, hold, atoi(p), 0);
					}
				}

				/* forward ref here */
				fwd_mark(as);
#if 0
				if (force_byte == NO)
				{
					force_word = 1;
				}
#endif
				forward = 1;
				*term = 0;

				return 0;
			}
		}
		if (as->pass == 2 && as->current_file->current_line == as->F_ref && as->current_filename_index == as->Ffn)
		{
#if 0
			if (as->line->force_byte == NO)
			{
				as->line->force_word = 1;
			}
#endif
			fwd_next(as);
		}
	}

	/* Closing parentheses? */	
	else if (**eptr == ')')
	{
		(*eptr)++;
	}
	else
	{
		/* 1. None of the above */		
		val = 0;
	}

	/* Negate if needed. */	
	if (minus)
	{
		*term = -val;
	}
	else
	{
		*term = val;
	}

	/* Complement if needed. */
	if (complement)
	{
		*term = ~val;
	}

	return 1;
}
