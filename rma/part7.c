/*
 * Part7.c NOTE:  this part stack-checks Compile WITHOUT the "-s" opt
 */

#include "rma.h"

/*
 * extern direct long nmbr_int; extern direct int  IsExtrn; extern direct
 * char **S_Addrs; extern direct int *d0053; extern direct char d005f; extern
 * direct char d0063, *SrcChar, *d007c;
 * 
 * extern struct symblstr d0587; extern struct ref_ent d0791[], *OptBPtr;
 */

#ifdef __STDC__

int
                do_long(void)
#else

do_long()
#endif
/* process a 32-bit operand */
{
	if (getNmbr() == 0)
	{
		return 0;
	}
	l143e();
	l1389();
	return 1;
}

#ifdef __STDC__

int
                l2369(void)
#else

l2369()
#endif
/* process a 16-bit operand */
{
	if (getNmbr() == 0)
	{
		return 0;
	}
	l143e();
	if (OptBPtr > d0791)
	{
		l1389();
	}
	else
	{
		if ((nmbr_int >= 65536) || (nmbr_int < -65536))
		{
			return e_report("value out of range");
		}
	}
	d0063 &= 0x7f;
	return 1;
}

#ifdef __STDC__

int
                l2387(void)
#else

l2387()
#endif
/* process an 8-bit operand */
{
	if (getNmbr() == 0)
	{
		return 0;
	}
	d0063 |= 0x08;
	l143e();
	if (OptBPtr > d0791)
	{
		l1389();
	}
	else
	{
		if ((nmbr_int >= 256) || (nmbr_int < -256))
		{
			return e_report("value out of range");
		}
	}
	d0063 &= 0x77;
	return 1;
}

#ifdef __STDC__

int
                getNmbr(void)
#else

getNmbr()
#endif
{
	SkipSpac();
	OptBPtr = d0791;
	if (do_math(1))
	{
		switch (*SrcChar)
		{
		case ' ':
		case '\t':
		case '\0':
		case ',':
		case ')':
		case ']':
			return 1;
		}
		e_report("bad operator");
	}
	return 0;
}

#ifdef __STDC__

int
                e_ilxtRf(void)
#else

e_ilxtRf()
#endif
{
	IsExtrn = nmbr_int = ilExtRef();
}

#ifdef __STDC__

int
                do_math(int parm)
#else

do_math(parm)
	int             parm;

#endif
{
	struct ref_ent *var1;
	long            frstnum;
	int             var3,
	                var4,
	                var5;

	if (l257e() == 0)
	{
		return 0;
	}
	while ((var5 = mathtyp(*SrcChar)) >= parm)
	{
		var4 = (*(SrcChar++));
		frstnum = nmbr_int;
		var1 = OptBPtr;
		var3 = IsExtrn;
		if (do_math(var5 + 1) == 0)
		{
			return 0;
		}
		IsExtrn |= var3;
		switch (var4)
		{
		case '+':
			nmbr_int = frstnum + nmbr_int;
			break;
		case '-':
			nmbr_int = frstnum - nmbr_int;
			while (var1 < OptBPtr)
			{
				(var1++)->ETyp ^= _HEXDIG;
			};
			break;
		case '*':
			if (IsExtrn)
			{
				e_ilxtRf();
			}
			else
			{
				nmbr_int = frstnum * nmbr_int;
			}
			break;
		case '/':
			if (IsExtrn)
			{
				e_ilxtRf();
			}
			else
			{
				if (nmbr_int == 0)
				{
					return e_report("zero division");
				}
				nmbr_int = frstnum / nmbr_int;
			}
			break;
		case '&':
			if (IsExtrn)
			{
				e_ilxtRf();
			}
			else
			{
				nmbr_int = frstnum & nmbr_int;
			}
			break;
		case '!':
			if (IsExtrn)
			{
				e_ilxtRf();
			}
			else
			{
				nmbr_int = frstnum | nmbr_int;
			}
			break;
		}
	}
	return 1;
}

#ifdef __STDC__

int
                mathtyp(int tst_char)
#else

mathtyp(tst_char)
	char            tst_char;

#endif
{
	switch (tst_char)
	{
	case '+':
	case '-':
		return 1;
	case '*':
	case '/':
		return 2;
	case '&':
	case '!':
		return 3;
	}
	return 0;		/* Default to "no match" */
}

#ifdef __STDC__

int
                l257e(void)
#else

l257e()
#endif
{
	struct ref_ent *var1;
	unsigned        var2,
	                var3;

	IsExtrn = 0;
	switch (var3 = *(SrcChar++))
	{
	case '+':
		if (l257e())
		{
			break;
		}
		return 0;
	case '-':
		var1 = OptBPtr;
		if (l257e())
		{
			nmbr_int = -nmbr_int;
			while (var1 < OptBPtr)
			{
				(var1++)->ETyp ^= NEGMASK;
			}
			break;
		}
		return 0;
	case '^':
		if (l257e())
		{
			if (IsExtrn)
			{
				e_ilxtRf();
			}
			else
			{
				nmbr_int = ~nmbr_int;
			}
			break;
		}
		return 0;
	case '(':
		if (do_math(1))
		{
			if (*SrcChar == ')')
			{
				++SrcChar;
				break;
			}
			else
			{
				e_report("parenthesis missing");
			}
		}
		return 0;
	case '*':
		if (S_Addrs == 0)
		{
			if (d0053 == 0)
			{
				return (nmbr_int = e_report("undefined org"));
			}
			else
			{
				nmbr_int = *d0053;
			}
		}
		else
		{
			nmbr_int = *(int *) S_Addrs;
			OptBPtr->ETyp = d005f;
			(OptBPtr++)->RAddr = d007c;
			IsExtrn = 1;
		}
		break;
	case '\'':
		if ((nmbr_int = *(SrcChar++)) == '\0')
		{
			--SrcChar;
			return 0;
		}
		break;
	default:
		--SrcChar;
		if (_getnum())
		{
			break;
		}
		if (MovLbl(d0587) != 0)
		{
			l1f63();
			break;
		}
		else
		{
			return (nmbr_int = e_report("bad operand"));
		}
	}
	return 1;		/* Success */
}

#ifdef __STDC__

int
                _getnum(void)
#else

_getnum()
#endif
{
	unsigned        base,
	                _passes;
	char            CTyp,
	                Dgt;

	_passes = base = 0;
	CTyp = 8;
	switch (Dgt = *SrcChar)
	{
	case '%':
		Dgt = *(++SrcChar);
		base = 2;
		CTyp = 0x10;
		break;
	case '$':
		Dgt = *(++SrcChar);
		base = 16;
		CTyp = 0x20;
		break;
	default:
		if (isdigit(Dgt))
		{
			base = 10;
		}
	}
	if (base == 0)
	{
		return 0;
	}
	nmbr_int = 0;
	while (_chcodes[Dgt = (islower(Dgt) ? _toupper(Dgt) : Dgt)] & CTyp)
	{
		nmbr_int = (nmbr_int * base) + (Dgt - ((Dgt < 'A') ? '0' : 55));
		++_passes;
		Dgt = *(++SrcChar);
	}
	return _passes;
}
