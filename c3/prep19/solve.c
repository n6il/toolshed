#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define MAXVAL 258
#define OPMAX 10
#define NUMBER 1
#define OP1    2
#define OP2    3



/* Static functions */
static void pushstk(unsigned val, unsigned *stk, int *sp);
static void pushop(char *val, char (*opstk)[4], int *opsp);
static char *popop(char (*opstk)[4], int *opsp);
static void pshunop(char *val, char *unop, int *unopsp);
static unsigned popstk(unsigned *stk, int *sp);
static char popunop(char *unop, int *unopsp);
static int getnum(unsigned *val, char **lptr);
static int getopr(char *val, char **lptr);
static int getop1(char *val, char **lptr);
static int tstnxtop(char *c_op, char **lptr);
static int opval(char *val);
static unsigned calc1(unsigned opd, char opr);
static unsigned calc2(unsigned opd1, unsigned opd2, char *opr);
static int _errmsg(int err, char *fmt, char *op);


unsigned        solve(ln)
	char           *ln;
{
	int             sp,
	                opsp,
	                unopsp,
	                typ;
	unsigned        stk[MAXVAL],
	                val;
	char            c_op[4],
	                opstk[OPMAX][4],
	                unop[OPMAX];
	char          **lptr,
	                num[MAXVAL],
	               *lnptr;

	sp = opsp = unopsp = stk[0] = c_op[0] = 0;
	lnptr = ln - 1;
	lptr = &lnptr;
	while (*(lnptr + 1))
	{
		typ = getop1(num, lptr);	/* get unary operator */
		/*
		        fprintf(stderr,"MAKERPN0: typ=%d num=|%s|\n",typ,num);
		*/
		if (typ == OP1)
		{
			pshunop(num, unop, &unopsp);
			continue;	/* get another unary operator */
		}
		typ = getnum(&val, lptr);	/* get number, parens, or
						 * unary operator */
		/*
		        fprintf(stderr,"MAKERPN: typ=%d val=%u\n",typ,val);
		*/
		if (typ == NUMBER)
		{
			pushstk(val, stk, &sp);
			while (unopsp)	/* push all unary operators on stack
					 * now */
				pushstk(calc1(popstk(stk, &sp), popunop(unop, &unopsp)), stk, &sp);
			while (*c_op)
			{
				if (tstnxtop(c_op, lptr))
				{
					pushop(c_op, opstk, &opsp);
					*c_op = 0;
				}
				else
				{
					pushstk(calc2(popstk(stk, &sp), popstk(stk, &sp), c_op), stk, &sp);
					*c_op = 0;
					if (opsp)
						strcpy(c_op, popop(opstk, &opsp));
				}
			}
		}
		if (!(typ = getopr(num, lptr)))	/* get binary operator */
			break;	/* finished with line */
		/*
		        fprintf(stderr,"MAKERPN2: typ=%d num=|%s|\n",typ,num);
		*/
		strcpy(c_op, num);
	}
	if (sp == 1)
		return stk[0];
	else
		exit(_errmsg(0, "Stack val=%d.  equation error.\n", sp));
}



static void pushstk(val, stk, sp)
	unsigned        val,
	               *stk;
	int            *sp;
{
	/*
		int             i;

	    fprintf(stderr,"PUSHSTK: val=%u sp=%d\n",val,*sp);
	    for (i=0;i<*sp;++i)
	        fprintf(stderr,"PUSHSTK: stk[%d]=%u\n",i,stk[i]);
	*/
	if (*sp < MAXVAL)
		stk[(*sp)++] = val;
	else
		exit(_errmsg(0, "PUSHSTK: Stack full!\n", NULL));
}



static unsigned        popstk(stk, sp)
	unsigned       *stk;
	int            *sp;
{
	/*
	    fprintf(stderr,"POPSTK: stk[]=%u\n",stk[*(sp-1)]);
	*/
	return stk[--*sp];
}



static void pushop(val, opstk, opsp)
	char           *val,
	                (*opstk)[4];
	int            *opsp;
{
	/*
	    fprintf(stderr,"PUSHOP: *opsp=%d\n",*opsp);
	*/
	if (*opsp < OPMAX)
		strcpy(opstk[(*opsp)++], val);
	else
		exit(_errmsg(0, "PUSHOP: Stack full!\n", NULL));
}



static char           *popop(opstk, opsp)
	char            (*opstk)[4];
	int            *opsp;
{
	/*
	    fprintf(stderr,"POPOP: *opsp=%d\n",*opsp);
	*/
	if (*opsp > 0)
		return opstk[--*opsp];
	else
		exit(_errmsg(0, "Stack empty!\n", NULL));
}



static void pshunop(val, unop, unopsp)
	char           *val,
	               *unop;
	int            *unopsp;
{
	/*
	    fprintf(stderr,"PSHUNOP: unopsp=%d\n",*unopsp);
	*/
	if (*unopsp < OPMAX)
		unop[(*unopsp)++] = *val;
	else
		exit(_errmsg(0, "PSHUNOP: Stack full!\n", NULL));
}



static char            popunop(unop, unopsp)
	char           *unop;
	int            *unopsp;
{
	if (*unopsp > 0)
		return unop[--*unopsp];
	else
		exit(_errmsg(0, "POPUNOP: Stack empty!\n", NULL));
}



static int getnum(val, lptr)		/* get number, parens, or unary operator */
	unsigned       *val;
	char          **lptr;
{
	int             i,
	                cnt;
	char            buf[MAXVAL];

	while (*(++*lptr) == ' ')
		;
	if (**lptr == '(')	/* expression in () needs to be isolated and */
	{			/* parsed separately */
		cnt = 1;
		i = 0;
		do		/* isolate paren data w/o parens */
		{
			buf[i++] = *(++*lptr);
			if (**lptr == '(')
				++cnt;
			else if (**lptr == ')')
				--cnt;
		} while (cnt);
		buf[i - 1] = 0;
		*val = solve(buf);	/* convert paren data */
		return NUMBER;
	}
	if (isdigit(**lptr))	/* get number */
	{
		*buf = **lptr;
		i = 0;
		while (isdigit(buf[++i] = *(++*lptr)))
			;
		buf[i] = 0;
		--*lptr;
		*val = atoi(buf);
		return NUMBER;
	}
	exit(_errmsg(0, "%d  Illegal number.\n", **lptr));
}



static int getopr(val, lptr)
	char           *val,
	              **lptr;
{
	int             i;

	while (*(++*lptr) == ' ')
		;
	i = 1;
	switch (**lptr)
	{
	case 0:
		return 0;
		break;
	case '|':		/* || and | */
	case '&':		/* && and & */
		val[0] = **lptr;
		if (*(*lptr + 1) == **lptr)
		{
			val[i++] = **lptr;
			++*lptr;
		}
		val[i] = 0;
		return OP2;
		break;
	case '^':		/* ^ */
	case '+':		/* + */
	case '-':		/* - */
	case '*':		/* * */
	case '/':		/* / */
	case '%':		/* % */
		val[0] = **lptr;
		val[1] = 0;
		return OP2;
		break;
	case '=':		/* == */
	case '!':		/* != */
		if (*(*lptr + 1) == '=')
		{
			val[0] = **lptr;
			val[1] = '=';
			val[2] = 0;
			++*lptr;
			return OP2;
		}
		exit(_errmsg(0, "%d Illegal binary operator.\n", *(*lptr + 1)));
		break;
	case '<':		/* < <= and << */
	case '>':		/* > >= and >> */
		val[0] = **lptr;
		if (*(*lptr + 1) == '=')
		{
			val[i++] = '=';
			++*lptr;
		}
		else if (*(*lptr + 1) == *val)
		{
			val[i++] = *val;
			++*lptr;
		}
		val[i] = 0;
		return OP2;
		break;
	default:
		exit(_errmsg(0, "%d Illegal binary operator.\n", **lptr));
		break;
	}
}



static int getop1(val, lptr)		/* get number, parens, or unary operator */
	char           *val,
	              **lptr;
{
	while (*(++*lptr) == ' ')
		;
	switch (**lptr)		/* get unary operator */
	{
	case '!':
	case '~':
	case '+':
	case '-':
		val[0] = **lptr;
		val[1] = 0;
		return OP1;
		break;
	}
	*(--*lptr);
	return 0;
}



static int tstnxtop(c_op, lptr)
	char           *c_op,
	              **lptr;
{
	char          **ln,
	               *l,
	                buf[MAXVAL];
	int             nxtop,
	                op1,
	                op2;

	l = *lptr;
	ln = &l;
	nxtop = getopr(buf, ln);
	if (nxtop == OP2)
	{
		/*
		        fprintf(stderr,"TSTNXTOP: c_op=|%s| buf=|%s|\n",c_op,buf);
		*/
		op1 = opval(c_op);
		op2 = opval(buf);
		if (op2 > op1)
			return TRUE;	/* nxt op is higher precedence */
		else
			return FALSE;
	}
	return FALSE;		/* Probably should be error here */
}



static int opval(val)			/* Compares two binary operators for
				 * precedence */
	char           *val;
{
	switch (*val)
	{
	case 0:
		return 0;
		break;
	case '|':		/* || and | */
		if (*(val + 1) == '|')
			return 1;
		else if (*(val + 1) == 0)
			return 3;
		exit(_errmsg(0, "%s Illegal operator\n", val));
		break;
	case '&':		/* && and & */
		if (*(val + 1) == '&')
			return 2;
		else if (*(val + 1) == 0)
			return 5;
		exit(_errmsg(0, "%s Illegal operator\n", val));
		break;
	case '^':		/* ^ */
		if (*(val + 1) == 0)
			return 4;
		exit(_errmsg(0, "%s Illegal operator\n", val));
		break;
	case '=':		/* == */
	case '!':		/* != */
		if (*(val + 1) == '=' && *(val + 2) == 0)
			return 6;
		exit(_errmsg(0, "%s Illegal operator\n", val));
		break;
	case '<':		/* < <= and << */
	case '>':		/* > >= and >> */
		if ((*(val + 1) == '=' && *(val + 2) == 0) || *(val + 1) == 0)
			return 7;
		else if (*(val + 1) == *val && *(val + 2) == 0)
			return 8;
		exit(_errmsg(0, "%s Illegal operator\n", val));
		break;
	case '+':		/* + */
	case '-':		/* - */
		return 9;
		break;
	case '*':		/* * */
	case '/':		/* / */
	case '%':		/* % */
		return 10;
		break;
	}
	exit(_errmsg(0, "%d Unrecognized operator!\n", *val));
}



static unsigned        calc1(opd, opr)
	unsigned        opd;
	char            opr;
{
	switch (opr)
	{
	case '!':
		return !opd;
		break;
	case '~':
		return ~opd;
		break;
	case '+':
		return opd;
		break;
	case '-':
		return -opd;
		break;
	}
	exit(_errmsg(0, "%d  Illegal operator.\n", opr));
}



static unsigned        calc2(opd1, opd2, opr)
	unsigned        opd1,
	                opd2;
	char           *opr;
{
	switch (*opr)
	{
	case '|':		/* || and | */
		if (*(opr + 1) == '|')
			return opd1 || opd2;
		else if (*(opr + 1) == 0)
			return opd1 | opd2;
		exit(_errmsg(0, "%s Illegal operator\n", opr));
		break;
	case '&':		/* && and & */
		if (*(opr + 1) == '&')
			return opd1 && opd2;
		else if (*(opr + 1) == 0)
			return opd1 & opd2;
		exit(_errmsg(0, "%s Illegal operator\n", opr));
		break;
	case '^':		/* ^ */
		if (*(opr + 1) == 0)
			return opd1 ^ opd2;
		exit(_errmsg(0, "%s Illegal operator\n", opr));
		break;
	case '=':		/* == */
		if (*(opr + 1) == '=' && *(opr + 2) == 0)
			return opd1 == opd2;
		exit(_errmsg(0, "%s Illegal operator\n", opr));
		break;
	case '!':		/* != */
		if (*(opr + 1) == '=' && *(opr + 2) == 0)
			return opd1 != opd2;
		exit(_errmsg(0, "%s Illegal operator\n", opr));
		break;
	case '<':		/* < <= and << */
		if (*(opr + 1) == '=' && *(opr + 2) == 0)
			return opd1 <= opd2;
		else if (*(opr + 1) == *opr && *(opr + 2) == 0)
			return opd1 << opd2;
		else if (*(opr + 1) == 0)
			return opd1 < opd2;
		exit(_errmsg(0, "%s Illegal operator\n", opr));
		break;
	case '>':		/* > >= and >> */
		if (*(opr + 1) == '=' && *(opr + 2) == 0)
			return opd1 >= opd2;
		else if (*(opr + 1) == *opr && *(opr + 2) == 0)
			return opd1 >> opd2;
		else if (*(opr + 1) == 0)
			return opd1 > opd2;
		exit(_errmsg(0, "%s Illegal operator\n", opr));
		break;
	case '+':		/* + */
		return opd1 + opd2;
		break;
	case '-':		/* - */
		return opd1 - opd2;
		break;
	case '*':		/* * */
		return opd1 * opd2;
		break;
	case '/':		/* % */
		return opd1 / opd2;
		break;
	case '%':		/* % */
		return opd1 % opd2;
		break;
	}
	exit(_errmsg(0, "%d  Illegal operator.\n", opr));
}


static int             _errmsg(err, fmt, op)
	int             err;
	char           *fmt,
	               *op;
{
	fprintf(stderr, fmt, op);

	return err;
}
