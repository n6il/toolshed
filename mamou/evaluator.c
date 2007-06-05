/***************************************************************************
 * evaluator.c: expression evaluation routines
 *
 * $Id$
 *
 * The Mamou Assembler - A Hitachi 6309 assembler
 *
 * (C) 2004 Boisy G. Pitre
 *
 *
 * The Shunting yard algorithm is used to parse and evaluate the mathematical
 * expressions passed here.
 * http://en.wikipedia.org/wiki/Shunting_yard_algorithm
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
static int term(assembler *as, int *term, char **eptr, int ignoreUndefined);
static int higher_or_same_precedence(char op1, char op2);
static int compute(int left, char op, int right);
static char getop(char **eptr);


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

	if (**eptr && **eptr == ')')
	{
		error(as, "too many )'s");
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
	int			value = 0;
	char		op;

	int			expp = 0, expqueue[256];
	int			typqueue[256];
	int			opp = 0, opstack[256];

	while (**eptr)
	{
		/* pickup term part of expression */
		if ((term(as, &value, eptr, ignoreUndefined) == 0) && (forward == 0))
		{
			value = 0;
		}
		else
		{
			/* add value to expression queue and type to type queue */
			expqueue[expp] = value;
			typqueue[expp++] = 1;
		}

		op = getop(eptr);
		
		if (op)
		{
			/* Check for closing parenthesis */
			if (op == ')')
			{
				while (opp > 0)
				{
					op = opstack[--opp];
					expqueue[expp] = op;
					typqueue[expp++] = 0;
				}
			}
			else
			{
				/* If operator at top of stack is higher or same precedence, then
				 * pop it and add it to expression queue
				 */
				while (opp > 0 && higher_or_same_precedence(opstack[opp - 1], op))
				{
					expqueue[expp] = opstack[--opp];
					typqueue[expp++] = 0;
				}
				opstack[opp++] = op;
			}
		}
		else
		{
			/* Unrecognized operator -- break */
			break;
		}
	}
	
	/* Pop operators off stack and add them to queue */
	while (opp > 0)
	{
		expqueue[expp] = opstack[--opp];
		typqueue[expp++] = 0;
	}
	
	/* Now go through expression queue and compute until complete */
	{
		int v = 0;
		int popper;
		
		opp = 0;
		opstack[opp] = 0;

		while (v < expp)
		{
			int type = typqueue[v];
			
			popper = expqueue[v++];

			if (type == 1)
			{
				/* it's a value - push it onto the opstack */
				opstack[opp++] = popper;
			}
			else
			{
				/* it's an operator */
				int left, right;
					
				right = opstack[--opp];
				left = opstack[--opp];
				opstack[opp++] = compute(left, popper, right);
			}
		}
	}
	
	/* Result is now left value */
	*result = opstack[0];

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
static int term(assembler *as, int *result, char **eptr, int ignoreUndefined)
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
	else if (**eptr == ')')
	{
		return 0;
	}
	/* a leading minus is a negation */	
	else if (**eptr == '-')
	{
		int	term2;
		int		ret;

		/* negate next term */
		(*eptr)++;
		ret = term(as, &term2, eptr, ignoreUndefined);
		*result = -term2;

		return ret;
	}
	/* complement? */
	else if (**eptr == '~' || **eptr == '^')
	{
		int	term2;
		int	ret;

		/* complement next term */
		(*eptr)++;
		ret = term(as, &term2, eptr, ignoreUndefined);
		*result = ~term2;

		return ret;
	}

	/* open parenthesis? */	
	while (**eptr == '(')
	{
		int	term2;
		int	ret;

		(*eptr)++;
		
		ret = expr(as, &term2, eptr, ignoreUndefined);

		if (**eptr != ')')
		{
			error(as, "too many ('s");

			return 0;
		}

		(*eptr)++;
		*result = term2;

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
				*result = 0;

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
		
		return 0;
	}
		
	*result = val;
	
	return 1;
}


/*!
	@function compute
	@discussion Computes a basic operation
	@param left The left side of the operation
	@param op The operation code
	@param right The right side of the operation
	@result The value of the operation
 */
static int compute(int left, char op, int right)
{
	switch (op)
	{
		case '!':
		case '|':
			return left | right;

		case '&':
			return left & right;

		case '+':
			return left + right;

		case '-':
			return left - right;

		case '*':
			return left * right;

		case '/':
			if (right == 0) return 0;
			return left / right;

		case '%':
			if (right == 0) return 0;
			return left % right;

		case '>':
			return left > right;

		case 'G':
			return left >= right;

		case '<':
			return left < right;

		case 'L':
			return left <= right;

		case '=':
			return left == right;

		case 'N':
			return left != right;
	}

	return 0;
}
	

/*!
	@function higher_or_same_precedence
	@discussion Returns a comparison of passed operators' precedence
	@param op1 The first operator
	@param op2 The second operator
	@result 1 if op1 has a higher or same precedence than op2, else 0
 */
static int higher_or_same_precedence(char op1, char op2)
{
	int p1v = 0, p2v = 0;
	
	switch (op1)
	{
		case '*':
		case '/':
			p1v = 2;
			break;
			
		case '+':
		case '-':
			p1v = 1;
			break;
			
		case '>':
		case '<':
		case 'G':
		case 'L':
		case 'N':
			p1v = 0;
			break;
	}
	
	switch (op2)
	{
		case '*':
		case '/':
			p2v = 2;
			break;
			
		case '+':
		case '-':
			p2v = 1;
			break;
			
		case '>':
		case '<':
			p2v = 0;
			break;
	}
	
	return p1v >= p2v;
}


static char getop(char **eptr)
{
	char op = *((*eptr)++);

	switch (op)
	{
		case '=':
		case '!':
		case '|':
		case '&':
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
			break;
			
		case '>':
			if (**eptr == '=')
			{
				(*eptr)++;
				op = 'G';
			}
			break;

		case '<':
			if (**eptr == '>')
			{
				(*eptr)++;
				op = 'N';
			}
			else
			if (**eptr == '=')
			{
				(*eptr)++;
				op = 'L';
			}
			break;
			
		default:
			/* We don't recognize.. back up and return nul */
			(*eptr)--;
			op = EOS;
			break;
	}
	
	return op;
}
