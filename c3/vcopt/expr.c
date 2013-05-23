#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "variable.h"
#include "copt2.h"
#include "expr.h"

const char     *curexpr = 0;
const char     *curident;
int             curidentlen;
int             curtok;
int             evalexpr;

#define T_SHIFTLEFT 'A'
#define T_SHIFTRIGHT 'B'
#define T_GTEQ 'C'
#define T_LTEQ 'D'
#define T_IDENT 'E'
#define T_INTNUM 'F'
#define T_NOTEQ 'G'
#define T_OROR 'H'
#define T_ANDAND 'I'
#define T_STRING 'J'
#define T_ISNUM 'K'
#define T_STRLEN 'L'

void
                PrintName(const char *name, int namelen)
{
	const char     *p = name;

	while (p - name < namelen)
	{
		putc(*p, stdout);
		++p;
	}
}

void
                RdToken(void)
{
	int             c;

	while (*curexpr == ' ')
		++curexpr;
	c = *curexpr++;

	if (isalpha(c) || c == '_')
	{
		curtok = T_IDENT;
		curident = curexpr - 1;
		while (isalpha(*curexpr) || *curexpr == '_' || isdigit(*curexpr))
		{
			++curexpr;
		}
		curidentlen = curexpr - curident;
		if (NameCmp("isnum", curident, curidentlen) == 0)
			curtok = T_ISNUM;
		else if (NameCmp("strlen", curident, curidentlen) == 0)
			curtok = T_STRLEN;
	}
	else if (isdigit(c) || c == '$' || c == '#')
	{
		curtok = T_INTNUM;
		curident = curexpr - 1;
		while (isdigit(*curexpr) || *curexpr == '$')
			++curexpr;
		curidentlen = curexpr - curident;
	}
	else if (c == '"')
	{
		curtok = T_STRING;
		curident = curexpr;
		while (*curexpr != '\0' && *curexpr != '"')
			++curexpr;
		if (*curexpr == '\0')
		{
			PatError("Non terminated string");
			exit(1);
		}
		curidentlen = curexpr - curident;
		++curexpr;
	}
	else if (c == '<')
	{
		if (*curexpr == '<')
		{
			curtok = T_SHIFTLEFT;
			++curexpr;
		}
		else if (*curexpr == '=')
		{
			curtok = T_LTEQ;
			++curexpr;
		}
		else
			curtok = '<';
	}
	else if (c == '>')
	{
		if (*curexpr == '>')
		{
			curtok = T_SHIFTRIGHT;
			++curexpr;
		}
		else if (*curexpr == '=')
		{
			curtok = T_GTEQ;
			++curexpr;
		}
		else
			curtok = '>';
	}
	else if (c == '=' && *curexpr == '=')
	{
		++curexpr;
		curtok = '=';
	}
	else if (c == '!')
	{
		if (*curexpr == '=')
		{
			curtok = T_NOTEQ;
			++curexpr;
		}
		else
			curtok = '!';
	}
	else if (c == '|')
	{
		if (*curexpr == '|')
		{
			curtok = T_OROR;
			++curexpr;
		}
		else
			curtok = '|';
	}
	else if (c == '&')
	{
		if (*curexpr == '&')
		{
			curtok = T_ANDAND;
			++curexpr;
		}
		else
			curtok = '&';
	}
	else
		curtok = c;
}

void
                RdPriExpr(Variable * value)
{
	if (curtok == '(')
	{
		RdToken();
		RdExpr(value);
		if (curtok != ')')
		{
			PatError("Unmatched parens");
			exit(1);
		}
		RdToken();
	}
	else
	{
		if (curtok == T_IDENT)
		{
			{
				Variable       *var = FindVar(curident, curidentlen);

				if (!var)
				{
					printf("variable '");
					PrintName(curident, curidentlen);
					printf("'\n");
					PatError("Unknown variable");
					exit(1);
				}
				value->type = var->type;
				switch (value->type)
				{
				case VT_INT:
					value->value.intval = var->value.intval;
					break;
				case VT_IDENT:
					strcpy(value->value.identval, var->value.identval);
					break;
				default:
					PatError("Unknown type");
					exit(1);
				}
			}
			RdToken();
		}
		else if (curtok == T_INTNUM)
		{
			value->type = VT_INT;
			value->value.intval = atoi(curident);
			value->name[0] = '\0';
			RdToken();
		}
		else if (curtok == T_STRING)
		{
			value->type = VT_IDENT;
			memcpy(value->value.identval, curident, curidentlen);
			value->value.identval[curidentlen] = '\0';
			value->name[0] = '\0';
			RdToken();
		}
		else
		{
			PatError("Unknown symbol");
			exit(1);
		}
	}
}

void
                RdPreExpr(Variable * value)
{
	{
		int             op = curtok;

		if (op == '+' || op == '-' || op == '!' || op == T_ISNUM || op == T_STRLEN)
		{
			RdToken();
			RdPreExpr(value);
			switch (value->type)
			{
			case VT_INT:
				if (op == '-')
					value->value.intval = -value->value.intval;
				else if (op == '!')
					value->value.intval = !value->value.intval;
				else if (op == T_ISNUM)
					value->value.intval = 1;
				else if (op == T_STRLEN && evalexpr)
					PatError("strlen on number");
				break;
			case VT_IDENT:
				if (op == T_ISNUM)
				{
					value->value.intval = 0;
					value->type = VT_INT;
				}
				else if (op == T_STRLEN)
				{
					value->value.intval = strlen(value->value.identval);
					value->type = VT_INT;
				}
				else if (evalexpr)
				{
					PatError("Operation on identifier");
				}
				break;
			default:
				PatError("Unknown type");
				break;
			}
		}
		else
			RdPriExpr(value);
	}
}

void
                RdBinExpr(
			                  Variable * value,
			                  int (*chkfunct) (void),
			                  int (*calcfunct) (int, int, int),
			                  void (*rdfunct) (Variable *)
)
{
	(*rdfunct) (value);
	while ((*chkfunct) ())
	{
		int             op = curtok;
		Variable        rvar;

		RdToken();
		(*rdfunct) (&rvar);
		if (evalexpr && (value->type == VT_IDENT || rvar.type == VT_IDENT))
		{
			PrintVar(value);
			PrintVar(&rvar);
			printf("op='%c'\n", op);
			PatError("operation on identifier");
			exit(1);
		}
		value->value.intval =
			(*calcfunct) (value->value.intval, rvar.value.intval, op);
	}
}

void
                RdIBExpr(
			                 Variable * value,
			                 int (*chkfunct) (void),
			                 int (*intcalcfunct) (int, int, int),
                    int (*identcalcfunct) (const char *, const char *, int),
			                 void (*rdfunct) (Variable *)
)
{
	(*rdfunct) (value);
	while ((*chkfunct) ())
	{
		int             op = curtok;
		Variable        rvar;

		RdToken();
		(*rdfunct) (&rvar);
		if (value->type == VT_IDENT && rvar.type == VT_IDENT)
		{
			value->value.intval =
				(*identcalcfunct) (value->value.identval, rvar.value.identval, op);
			value->type = VT_INT;
		}
		else if (value->type == VT_INT && rvar.type == VT_INT)
		{
			value->value.intval =
				(*intcalcfunct) (value->value.intval, rvar.value.intval, op);
		}
		else if (evalexpr)
		{
			PatError("Mismatched types in operation");
			exit(1);
		}
	}
}

int
                ChkMDExpr(void)
{
	return curtok == '*' || curtok == '/';
}

int
                CalcMDExpr(int lnum, int rnum, int op)
{
	if (op == '*')
		return lnum * rnum;
	else
		return lnum / rnum;
}

void
                RdMDExpr(Variable * value)
{
	RdBinExpr(value, ChkMDExpr, CalcMDExpr, RdPreExpr);
}

int
                ChkPMExpr(void)
{
	return curtok == '+' || curtok == '-';
}

int
                CalcPMExpr(int lnum, int rnum, int op)
{
	if (op == '+')
		return lnum + rnum;
	else
		return lnum - rnum;
}

void
                RdPMExpr(Variable * value)
{
	RdBinExpr(value, ChkPMExpr, CalcPMExpr, RdMDExpr);
}

int
                ChkShiftExpr(void)
{
	return curtok == T_SHIFTLEFT || curtok == T_SHIFTRIGHT;
}

int
                CalcShiftExpr(int lnum, int rnum, int op)
{
	if (op == T_SHIFTLEFT)
		return lnum << rnum;
	else
		return lnum >> rnum;
}

void
                RdShiftExpr(Variable * value)
{
	RdBinExpr(value, ChkShiftExpr, CalcShiftExpr, RdPMExpr);
}

int
                ChkRelExpr(void)
{
	return curtok == '<' || curtok == '>' || curtok == T_LTEQ || curtok == T_GTEQ;
}

int
                CalcRelExpr(int lnum, int rnum, int op)
{
	switch (op)
	{
		case '<':
		return lnum < rnum;
	case '>':
		return lnum > rnum;
	case T_LTEQ:
		return lnum <= rnum;
	case T_GTEQ:
		return lnum >= rnum;
	}
	return 0;
}

int
                CalcIRelExpr(const char *lident, const char *rident, int op)
{
	int             compval = strcmp(lident, rident);

	switch (op)
	{
	case '<':
		return compval < 0;
	case '>':
		return compval > 0;
	case T_LTEQ:
		return compval <= 0;
	case T_GTEQ:
		return compval >= 0;
	}
	return 0;
}

void
                RdRelExpr(Variable * value)
{
	RdIBExpr(value, ChkRelExpr, CalcRelExpr, CalcIRelExpr, RdShiftExpr);
}

int
                ChkEqExpr(void)
{
	return curtok == '=' || curtok == T_NOTEQ;
}

int
                CalcEqExpr(int lnum, int rnum, int op)
{
	if (op == '=')
		return lnum == rnum;
	else
		return lnum != rnum;
}

int
                CalcIEqExpr(const char *lident, const char *rident, int op)
{
	int             compval = strcmp(lident, rident);

	return (op == '=' && compval == 0) || (op == T_NOTEQ && compval != 0);
}

void
                RdEqExpr(Variable * value)
{
	RdIBExpr(value, ChkEqExpr, CalcEqExpr, CalcIEqExpr, RdRelExpr);
}

int
                ChkBAndExpr(void)
{
	return curtok == '&';
}

int
                CalcBAndExpr(int lnum, int rnum, int op)
{
//#pragma argsused
	return lnum & rnum;
}

void
                RdBAndExpr(Variable * value)
{
	RdBinExpr(value, ChkBAndExpr, CalcBAndExpr, RdEqExpr);
}

int
                ChkXOrExpr(void)
{
	return curtok == '^';
}

int
                CalcXOrExpr(int lnum, int rnum, int op)
{
//#pragma argsused
	return lnum ^ rnum;
}

void
                RdXOrExpr(Variable * value)
{
	RdBinExpr(value, ChkXOrExpr, CalcXOrExpr, RdBAndExpr);
}

int
                ChkBOrExpr(void)
{
	return curtok == '|';
}

int
                CalcBOrExpr(int lnum, int rnum, int op)
{
//#pragma argsused
	return lnum | rnum;
}

void
                RdBOrExpr(Variable * value)
{
	RdBinExpr(value, ChkBOrExpr, CalcBOrExpr, RdXOrExpr);
}

void
                RdLAndExpr(Variable * value)
{
	RdBOrExpr(value);
	if (curtok == T_ANDAND)
	{
		if (evalexpr && value->type != VT_INT)
			PatError("&& on identifier");
		while (curtok == T_ANDAND)
		{
			Variable        rvar;

			if (!evalexpr || !value->value.intval)
			{
				int             oldeval = evalexpr;

				evalexpr = 0;
				while (curtok == T_ANDAND)
				{
					RdToken();
					RdBOrExpr(value);
				}
				evalexpr = oldeval;
				break;
			}
			RdToken();
			RdBOrExpr(&rvar);
			if (rvar.type == VT_IDENT)
				PatError("&& on identifier");
			if (rvar.type == VT_IDENT)
			{
				PatError("operation on identifier");
				exit(1);
			}
			value->value.intval = rvar.value.intval;
		}
	}
}

void
                RdLOrExpr(Variable * value)
{
	RdLAndExpr(value);
	if (curtok == T_OROR)
	{
		if (evalexpr && value->type != VT_INT)
			PatError("|| on identifier");
		while (curtok == T_OROR)
		{
			Variable        rvar;

			if (!evalexpr || value->value.intval)
			{
				int             oldeval = evalexpr;

				evalexpr = 0;
				while (curtok == T_OROR)
				{
					RdToken();
					RdBOrExpr(value);
				}
				evalexpr = oldeval;
				break;
			}
			RdToken();
			RdLAndExpr(&rvar);
			if (rvar.type == VT_IDENT)
				PatError("|| on identifier");
			if (rvar.type == VT_IDENT)
			{
				PatError("operation on identifier");
				exit(1);
			}
			value->value.intval = rvar.value.intval;
		}
	}
}

void
                RdCondExpr(Variable * value)
{
	RdLOrExpr(value);
	if (curtok == '?')
	{
		Variable        other;
		Variable       *ifptr;
		Variable       *elseptr;

		RdToken();
		if (evalexpr && value->type != VT_INT)
		{
			PatError("Invalid conditional");
			exit(1);
		}
		if (value->value.intval)
		{
			ifptr = value;
			elseptr = &other;
		}
		else
		{
			ifptr = &other;
			elseptr = value;
		}
		RdExpr(ifptr);
		if (curtok != ':')
		{
			PatError("expecting ':'");
			exit(1);
		}
		RdToken();
		RdCondExpr(elseptr);
	}
}

void
                RdExpr(Variable * value)
{
	RdCondExpr(value);
}

const char     *
                ReadExpr(const char *expr, Variable * value)
{
	curexpr = expr;
	RdToken();
	RdExpr(value);
	return curexpr;
}
