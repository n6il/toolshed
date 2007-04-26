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
 *      expr ::=  rel > expr |
 *                rel < expr |
 *                rel >= expr |
 *                rel <= expr |
 *                rel <> expr |
 *                rel = expr |
 *				  rel
 *                
 *      rel  ::=  fact + rel |
 *                fact - rel |
 *                fact | rel |
 *                fact & rel |
 *                fact ^ rel |
 *				  fact
 *                
 *      fact ::=  term * fact |
 *                term / fact |
 *                term % fact |
 *                term
 *
 *      term ::=  symbol   |
 *                *        |
 *                constant |
 *                (expr)
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
static int expr(assembler *as, int *term, char **eptr, int ignoreUndefined);
static int rel(assembler *as, int *term, char **eptr, int ignoreUndefined);
static int factor(assembler *as, int *term, char **eptr, int ignoreUndefined);
static int term(assembler *as, int *term, char **eptr, int ignoreUndefined);
static int is_factor_op(char c);
static int is_term_op(char c);
static int is_rel_op(char c);


/* Static variables */
static int forward = 0;


/*!
	@function evaluate
	@discussion Evaluates mathematical expressions and symbols
	@param as The assembler state structure
	@param result A pointer to the result of the evaluated expression
	@param eptr The expression to be evaluated
	@param ignoreUndefined Ignore any undefined symbols
	@result 1=evaluation was made ok
 */
int evaluate(assembler *as, int *result, char **eptr, int ignoreUndefined)
{
	int			retval;
	
	/* show any debugging output. */
	if (as->o_debug)
	{
		printf("Evaluating %s\n", *eptr);
	}	
	
	/* assume no forcing of result size for this line */
	as->line.force_byte = 0;
	as->line.force_word = 0;
	
	/* force byte or word size? */
	if (**eptr == '<')
	{
		as->line.force_byte = 1;
		(*eptr)++;
	}
	else if (**eptr == '>')
	{
		as->line.force_word = 1;
		(*eptr)++;
	}

	/* parse the expression */
	retval = expr(as, result, eptr, ignoreUndefined);

	if (**eptr == ')')
	{
		error(as, "unbalanced parentheses");

		return 0;
	}
	
	/* print debugging information if requested */
	if (as->o_debug)
	{
		printf("Result     $%x\n", (int)*result);
		printf("force_byte %d\n", as->line.force_byte);
		printf("force_word %d\n", as->line.force_word);
	}

	return retval;
}


/*!
	@function expr
	@discussion Evaluates mathematical expressions
	@param as The assembler state structure
	@param result A pointer to the result of the evaluated expression
	@param eptr The expression to be evaluated
	@param ignoreUndefined Ignore any undefined symbols
	@result 1=evaluation was made ok
 */
static int expr(assembler *as, int *result, char **eptr, int ignoreUndefined)
{
	int			left = 0;
	int			right = 0;			/* left and right terms for expression */
	char		o, o2;				/* operator characters */

	/* pickup first part of expression */
	if ((rel(as, &left, eptr, ignoreUndefined) == 0) && (forward == 0))
	{
//		*result = Dp * 256 + 0xfe;
		left = 0;
//		return 0;
	}

	/* gather rest of line */
	if (is_rel_op(**eptr))
	{
		/* pickup operator and skip */
 		o = *((*eptr)++);
		o2 = EOS;
		if (is_rel_op(**eptr))
		{
			o2 = *((*eptr)++);
		}
		
		/* pickup current rightmost side */		
		if (expr(as, &right, eptr, ignoreUndefined) == 0)
		{
//			*result = Dp * 256 + 0xfe;
			right = 0;
//			return 0;
		}
		
		/* process left/right according to operator */
		switch (o)
		{
			case '>':
				if (o2)
				{
					switch (o2)
					{
						case '=':
							left = (left >= right);
							break;

						default:
							error(as, "bad operator");
							return 0;
					}
				}
				else
				{
					left = (left > right);
				}
				break;
			case '<':
				if (o2)
				{
					switch (o2)
					{
						case '=':
							left = (left <= right);
							break;

						case '>':
							left = (left != right);
							break;

						default:
							error(as, "bad operator");
							return 1;
					}
				}
				else
				{
					left = (left < right);
				}
				break;
			case '=':
				left = left == right;
				break;
		}
	}

	/* assign left to result */
	*result = left;

	/* return status */
	return 1;
}



/*!
	@function rel
	@discussion Evaluates relational expressions
	@param as The assembler state structure
	@param result A pointer to the result of the evaluated expression
	@param eptr The expression to be evaluated
	@param ignoreUndefined Ignore any undefined symbols
	@result 1=evaluation was made ok
 */
static int rel(assembler *as, int *result, char **eptr, int ignoreUndefined)
{
	int			left = 0;
	int			right = 0;			/* left and right terms for expression */
	char		o;					/* operator character */

	/* pickup first part of expression */
	if ((factor(as, &left, eptr, ignoreUndefined) == 0) && (forward == 0))
	{
//		*result = Dp * 256 + 0xfe;
		left = 0;
//		return 0;
	}

	/* gather rest of line */
	if (is_term_op(**eptr))
	{
		/* pickup operator and skip */
 		o = *((*eptr)++);

		/* pickup current rightmost side */		
		if (expr(as, &right, eptr, ignoreUndefined) == 0)
		{
//			*result = Dp * 256 + 0xfe;
			right = 0;
//			return 0;
		}
		
		/* process left/right according to operator */
		switch (o)
		{
			case '+':
				left += right;
				break;
			case '-':
				left -= right;
				break;
			case '|':
			case '!':
				left |= right;
				break;
			case '&':
				left &= right;
				break;
		}
	}

	/* assign left to result */
	*result = left;

	/* return status */
	return 1;
}



/*!
	@function factor
	@discussion Evaluates factors
	@param as The assembler state structure
	@param result A pointer to the result of the evaluated expression
	@param eptr The expression to be evaluated
	@param ignoreUndefined Ignore any undefined symbols
	@result 1=evaluation was made ok
 */
static int factor(assembler *as, int *result, char **eptr, int ignoreUndefined)
{
	int			left = 0;
	int			right = 0;			/* left and right terms for expression */
	char		o;					/* operator character */

	if ((term(as, &left, eptr, ignoreUndefined) == 0) && (forward == 0))
	{
//		*result = Dp * 256 + 0xfe;
		left = 0;
//		return 0;
	}

	/* gather rest of line */
	if (is_factor_op(**eptr))
	{
		/* pickup operator and skip */
 		o = *((*eptr)++);

		/* pickup current rightmost side */
		if (factor(as, &right, eptr, ignoreUndefined) == 0)
		{
//			*result = Dp * 256 + 0xfe;
			right = 0;
//			return 0;
		}
		
		/* process left/right according to operator */
		switch (o)
		{
			case '*':
				left *= right;
				break;
			case '/':
				left /= right;
				break;
			case '%':
				left %= right;
				break;
		}
	}

	/* assign result to left */	
	*result = left;

	/* return status */	
	return 1;
}



/*!
	@function term
	@discussion Evaluates a single term
	@param as The assembler state structure
	@param term A pointer to the term that is returned
	@param eptr The address of a pointer to the expression to be evaluated
	@param ignoreUndefined Ignore any undefined symbols
 */
static int term(assembler *as, int *term, char **eptr, int ignoreUndefined)
{
	char			hold[MAXBUF];
	char			*tmp;
	int				val = 0;				/* local value being built */
	struct nlist	*pointer;
	struct link		*pnt, *bpnt;

	forward = 0;

	/* if we encounter end of string, something's wrong */
	if (!**eptr)
	{
		error(as, "unbalanced expression");

		return 0;
	}	
	/* a leading minus is a negation */	
	else if (**eptr == '-')
	{
		int	term2;
		int		ret;

		/* negate next term */
		(*eptr)++;
		ret = expr(as, &term2, eptr, ignoreUndefined);
		*term = -term2;

		return ret;
	}
	/* complement? */
	else if (**eptr == '~' || **eptr == '^')
	{
		int	term2;
		int	ret;

		/* complement next term */
		(*eptr)++;
		ret = expr(as, &term2, eptr, ignoreUndefined);
		*term = ~term2;

		return ret;
	}

	/* open parenthesis? */	
	if (**eptr == '(')
	{
		int	ret;

		(*eptr)++;

		ret = expr(as, term, eptr, ignoreUndefined); /* evaluate the inside */

		if (**eptr != ')')
		{
			error(as, "unbalanced parentheses");

			return 0;
		}

		(*eptr)++;

		return ret;
	}
	
	/* binary constant? */
	if (**eptr == '%')
	{
		(*eptr)++;

		while (any(**eptr, "01"))
		{
			val = (val * 2) + ((*((*eptr)++)) - '0');
		}
	}
	/* octal constant? */
	else if (**eptr == '@')
	{
		(*eptr)++;

		while (any(**eptr, "01234567"))
		{
			val = (val * 8) + ((*((*eptr)++)) - '0');
		}
	}
	/* hexadecimal constant? */
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
	/* decimal constant? */	
	else if (any(**eptr, "0123456789"))
	{
		while (**eptr >= '0' && **eptr <= '9')
		{
			val = (val * 10) + ((*((*eptr)++)) - '0');
		}
	}
	/* current program counter? */
	else if (**eptr == '*')
	{
		(*eptr)++;
		val = as->old_program_counter;
	}
	/* current data counter? */
	else if (**eptr == '.')
	{
		(*eptr)++;
		val = as->data_counter;
	}
	/* character literal? */
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
				val = val << 8;
				val += *((*eptr)++);
			}
		}
	}
	/* symbol? */
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
			/* SYMBOL FOUND! */
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

			val = pointer->def;
		}
		else
		{
			/* SYMBOL *NOT* FOUND! */
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

	/* closing parentheses? */	
	else if (**eptr == ')')
	{
		(*eptr)++;
	}
	else
	{
		/* none of the above */		
		val = 0;
	}
		
	*term = val;
	
	return 1;
}


/*!
	@function is_factor_op
	@discussion Determines if a character is a factor operator
	@param c Character to evaluate
 */
static int is_factor_op(char c)
{
	if (any(c, "*/%"))
	{
		return 1;
	}	
	
	return 0;
}


/*!
	@function is_term_op
	@discussion Determines if a character is a term operator
	@param c Character to evaluate
 */
static int is_term_op(char c)
{
	if (any(c, "+-&|^!"))
	{
		return 1;
	}	
	
	return 0;
}


/*!
	@function is_rel_op
	@discussion Determines if a character is a relation operator
	@param c Character to evaluate
 */
static int is_rel_op(char c)
{
	if (any(c, "=<>!"))
	{
		return 1;
	}	
	
	return 0;
}
