/*
 *	Modification history for codgen.c:
 *	24-May-83	Fixed (once and for all!!) the illegal memory
 *				reference involving FREG and/or direct-sc offsets.
 *				(Originally goto'ed (sic) dooff: to add the offset
 *				causing illegal reference to node->type.) Made
 *				addoff() function to handle this nasty bit.
 *	25-May-83	Changed around the register variables.
 *	07-Jun-83	Fixed Non-indirect lea-type references to direct page
 *				to look like "leax >a,y" instead of "leax <a".
 *	29-Aug-83	Function calls in the form of (*((int(*)())0xfffe))();
 *				did not dereference properly.
 *	26-Dec-83	Moved profexit code from gen(RETURN....) to allow
 *                       returning local storage after call to eprof.
 *	17-Apr-84	Conversion for UNIX.
 *	01-Apr-86	clear knowledge of D contents for long and float ops.
 *	17-Jun-86	fix LOADIM of ptr to static direct variables.
 *      18-Jul-05       Added UNIX def to od() to prevent printing of words
 *                      larger than 0xFFFF on systems with sizeof(int)>2 - BGP.
 */

#include "cj.h"

static void outlea(int reg);
static void addoff(int offset);
static void doref(int reg, int arg, expnode *val, int offset);

direct int datflag;

/*
 * There's no advantage in putting the following ptrs on dp
 * (ptrs always access indexed)
 */
static char *yind = ",y";
static char *spind = ",s";
static char *lbsr = "lbsr ";
static char *lbra = "lbra ";
static char *clra = "clra";
static char *unkopr = "unknown operator : ";

void gen(int op, int rtype, int arg, expnode *val)
{
	register expnode	*ptr;
	register symnode	*sptr;
	int					reg;
	int					temp, value;

	if (op == LONGOP)
	{
		dolongs(rtype, arg);
		return;
	}
	
	if (op == DBLOP)
	{
		dofloats(rtype, arg);
		return;
	}
	
	switch(op)
	{
	case PUSH:
		fprintf(code, " pshs %c\n", regname(rtype));
		if ((sp -= 2) < maxpush)
		{
			maxpush = sp;
		}
		return;
	case JMPEQ:
		gen(COMPARE, XREG, CONST, arg);
		arg = rtype;
		rtype = EQ;
	case CNDJMP:
		ot("lb");
		os(getrel(rtype));
		label(arg);
		return;
	case RETURN:
		ol("puls u,pc\n");
		return;
	case AND:
	case OR:
	case XOR:
		trouble(op, arg, val);
		return;
	case TIMES:
		mwsyscall("ccmult");
		return;
	case UDIV:
		mwsyscall("ccudiv");
		return;
	case DIV:
		mwsyscall("ccdiv");
		return;
	case SHL:
		mwsyscall("ccasl");
		return;
	case SHR:
		mwsyscall("ccasr");
		return;
	case USHR:
		mwsyscall("cclsr");
		return;
	case UMOD:
		mwsyscall("ccumod");
		return;
	case MOD:
		mwsyscall("ccmod");
		return;
	case NEG:
		ol("nega\n negb\n sbca #0");
		return;
	case COMPL:
		ol("coma\n comb");
		return;
	case GOTO:
		ot("leax ");
		od(-sp);
		os(spind);
		nl();
		/* all branches are long; assembly code optimizer fixes them up */
	case JMP:
		ot(lbra);
		label(rtype);
		return;
	case LABEL:
		ot(lbra);
		label(arg = getlabel());
		label(rtype);
		ot("leas ");
		od(sp);
		os(",x\n");
		label(arg);
		return;
	case CALL:
		callflag = 4;
#ifdef DEBUG
#if 0
		fflush(stdout);
		fprintf(stderr, "arg=%04x op=%d val=%04x\n",
			arg, ((int*)arg)->op, ((int*)arg)->value);
#endif
#endif
		if ((ptr = (expnode *) arg)->op == NAME && (sptr = ptr->val.sp)) {
			ot(lbsr);
			nlabel(sptr->sname, 0);
		} else {
			ot("jsr ");
			deref(rtype, arg, 0);
			nl();
		}
		return;
	case CTOI:
		ol("sex");
		return;
	case LTOI:
		ot("ld");
		doref('d', rtype, arg, 2);
		return;
	case IDOUBLE:
		ol("aslb\n rola");
		return;
	case HALVE:
		ol("asra\n rorb");
		return;
	case UHALVE:
		ol("lsra\n rorb");
		return;
	case YREG:
		ot("ldy ");
		goto dooff;
	case UREG:
		ot("ldu ");
dooff:
		od(rtype);
		os(spind);
		nl();
		return;
	case LEAX:
		ot("leax ");
		switch (rtype) {
		case NODE:
			reg =	(reg = ((expnode *)arg)->op) == YIND ? 'y' :
					reg == UIND ? 'u' : 'x';
			fprintf(code, "%d,%c\n", ((expnode *)arg)->val.num, reg);
			break;
		case DREG:
			fprintf(code, "d,%c\n", regname(arg));
			break;
		}
		return;
	}

	reg = regname(rtype);
	if (arg == NODE) {
		if (val->op == CTOI) {
			gen(op, DREG, NODE, val->left);
			switch (op) {
			case LOAD:
				ol("sex");
				break;
			case RSUB:
			case PLUS:
				ol("adca #0");
				break;
			case MINUS:
				ol("sbca #0");
				break;
			}
			val->op = DREG;
			return;
		}
#ifdef DEBUG
#if 0
		fprintf(stderr,
			"val=%04x val->type=%04x op=%04x reg=%04x val->op=%04x rhs=%04x\n",
			val, val->type, op, reg, val->op, ((int*)val->value)->type);
#endif
#endif
		if (val->type == CHAR && op != LOADIM && reg != 'x')
			reg = 'b';
	}

	switch (op) {
	case LOAD:
		if (arg ==NODE) {
			value = val->val.num;
			switch (temp = val->op) {
			case XREG:
			case YREG:
			case UREG:
				if (rtype != DREG) {
					outlea(reg);
					fprintf(code, "%d,%c\n", value, regname(temp));
				} else {
					transfer(regname(temp), 'd');
					if (value)
						gen(PLUS, DREG, CONST, value);
				}
				return;
			case DREG:
				if (rtype != DREG)
					transfer('d', reg);
				return;
			case STRING:
				outstr(reg, val->val. num);
				return;
			case CONST:
				arg = CONST;
				val = (expnode *) value;
			}
		}
		if (rtype == DREG && arg == CONST && val == 0) {
			ol("clra\n clrb");
			return;
		}
		ot("ld");
		goto simple;
	case COMPARE:
		if (arg == CONST && val == 0) {
			fprintf(code, " st%c -2,s\n", reg);
			return;
		}
		ot("cmp");
		if (reg == 'b')
			reg = 'd';
		goto simple;
	case STORE:
		if (arg == NODE && isreg(temp = val->op)) {
			if (temp != rtype)
				transfer(reg, regname(temp));
			return;
		}
		ot("st");
		goto simple;
	case MINUS:
		ot("sub");
		goto simple;
	case RSUB:
		gen(NEG, 0, 0, NULL);
	case PLUS:
		ot("add");
simple:
		doref(reg, arg, val, 0);
		return;
	case LOADIM:
		if (arg == NODE) {
			switch (temp = val->val.sp->storage) {
			case DIRECT:
			case EXTERND:
			case STATICD:
				outlea(reg);
				ob('>');
				if (temp == STATICD)
					olbl(val->val.sp->offset);
				else
					on(val->val.sp->sname);
				addoff(val->modifier);
				os(yind);
				nl();
				return;
			}
		}
		ot("lea");
		doref(reg, arg, val, 0);
		return;
	case EXG:
		fprintf(code, " exg %c,%c\n", reg, regname(arg));
		return;
	case LEA:
		outlea(reg);
		switch (arg) {
		case DREG:
			os("d,");
			goto leaexit;
		case CONST:
			od(val);
			ob(',');
leaexit:
			ob(reg);
			nl();
			return;
		default:
			error("LEA arg");
			break;
		}
		return;
	default:
		error(unkopr);
	}
}

char regname(int r)
{
	switch (r) {
	case DREG:
		return 'd';
	case XREG:
		return 'x';
	case YREG:
		return 'y';
	case UREG:
		return 'u';
	default:
		return ' ';
	}
}

void transfer(int r1, int r2)
{
	fprintf(code," tfr %c,%c\n", r1, r2);
}

void dolongs(int op, int arg)
{
	switch (op) {
	case STACK:
		gen(LOAD, DREG, XIND, 2);
		gen(PUSH, DREG, 0, NULL);
		gen(LOAD, DREG, XIND, NULL);
		gen(PUSH, DREG, 0, NULL);
		break;
	case TEST:
		ol("lda 0,x\n ora 1,x\n ora 2,x\n ora 3,x");
		break;
	case MOVE:
		lcall("_lmove");
		sp -= 2;
		break;
	case PLUS:
		lcall("_ladd");
		break;
	case MINUS:
		lcall("_lsub");
		break;
	case TIMES:
		lcall("_lmul");
		break;
	case DIV:
		lcall("_ldiv");
		break;
	case MOD:
		lcall("_lmod");
		break;
	case AND:
		lcall("_land");
		break;
	case OR:
		lcall("_lor");
		break;
	case XOR:
		lcall("_lxor");
		break;
	case SHL:
		lcall("_lshl");
		sp -= 2;
		break;
	case SHR:
		lcall("_lshr");
		sp -= 2;
		break;
	case EQ:
	case NEQ:
	case GEQ:
	case LEQ:
	case GT:
	case LT:
		lcall("_lcmpr");
		break;
	case NEG:
		lcall("_lneg");
		sp-=4;
		break;
	case COMPL:
		lcall("_lcompl");
		sp -= 4;
		break;
	case ITOL:
		lcall("_litol");
		sp -= 4;
		break;
	case UTOL:
		lcall("_lutol");
		sp -= 4;
		break;
	case INCBEF:
	case INCAFT:
		lcall("_linc");
		sp -= 4;
		break;
	case DECBEF:
	case DECAFT:
		lcall("_ldec");
		sp -= 4;
		break;
	case LCONST:
		getcon(arg, 2);
		break;
	default:
		error("codgen - longs");
		os(unkopr);
		od(op);
		nl();
		break;
	}
	setdreg(NULL);
}

void dofloats(int op, int arg)
{
	switch (op) {
	case FCONST:
		getcon(arg, 4);
		break;
	case STACK:
		fcall("_dstack");
		sp -= 8;
		break;
	case TEST:
		fprintf(code, " lda %c,x\n", arg == FLOAT ? '3' : '7');
		break;
	case MOVE:
		fcall(arg == FLOAT ? "_fmove" : "_dmove");
		sp += 2;
		break;
	case PLUS:
		fcall("_dadd");
		sp += 8;
		break;
	case MINUS:
		fcall("_dsub");
		sp += 8;
		break;
	case TIMES:
		fcall("_dmul");
		sp += 8;
		break;
	case DIV:
		fcall("_ddiv");
		sp += 8;
		break;
	case EQ:
	case NEQ:
	case GEQ:
	case LEQ:
	case GT:
	case LT:
		fcall("_dcmpr");
		sp += 8;
		break;
	case NEG:
		fcall("_dneg");
		break;
	case INCBEF:
	case INCAFT:
		fcall(arg == FLOAT ? "_finc" : "_dinc");
		break;
	case DECBEF:
	case DECAFT:
		fcall(arg == FLOAT ? "_fdec" : "_ddec");
		break;
	case DTOF:
		fcall("_dtof");
		break;
	case FTOD:
		fcall("_ftod");
		break;
	case LTOD:
		fcall("_ltod");
		break;
	case ITOD:
		fcall("_itod");
		break;
	case UTOD:
		fcall("_utod");
		break;
	case DTOL:
		fcall("_dtol");
		break;
	case DTOI:
		fcall("_dtoi");
		break;
	default:
		error("codgen - floats");
		os(unkopr);
		od(op);
		nl();
	}
	setdreg(NULL);
}

void getcon(int *p, int n)
{
	int	temp;

	ot("bsr ");
	label(temp = getlabel());
	defcon(p, n);
	olbl(temp);
	ol("puls x");
}

void defcon(INTTYPE *p, int n)
{
	defword();
	if (n == 1)
		od((int)p);
	else if (p == NULL) {
		while (--n > 0)
			os("0,");
		ob('0');
	} else {
		while (--n > 0) {
			od(*p++);
			ob(',');
		}
		od(*p);
	}
	nl();
}

void mwsyscall(char *s)
{
	ot(lbsr);
	ol(s);
	sp += 2;
}

void lcall(char *s)
{
	ot(lbsr);
	ol(s);
	sp += 4;
}

void fcall(char *s)
{
	ot(lbsr);
	ol(s);
}

void trouble(int op, int arg, expnode *val)
{
	char	*s;
	int		cflag, temp;

	if (arg == NODE) {
		cflag = (val->type == CHAR);
		arg = val->op;
		val = (expnode *) val->val.sp;
	} else
		cflag = 0;

	switch (op) {
	case AND:
		s = "and";
		break;
	case OR:
		s = "or";
		break;
	case XOR:
		s = "eor";
		break;
	}
	switch (arg) {
	case NAME:
	case YIND:
	case UIND:
	case XIND:
		if (cflag) {
			if (op == AND)
				ol(clra);
			ot(s);
			doref('b', arg, val, 0);
		} else {
			ot(s);
			doref('a', arg, val, 0);
			ot(s);
			doref('b', arg, val, 1);
		}
		break;
	case CONST:
		switch (temp = ((int) val >> 8) & 0xff) {
		case 0:
			if (op == AND)
				ol(clra);
			break;
		case 0xff:
			if (op == AND)
				break;
			if (op == XOR) {
				ol("coma");
				break;
			}
		default:
			ot(s);
			doref('a', CONST, temp, 0);
		}
		switch (temp = (int) val & 255) {
		case 0:
			if (op == AND)
				ol("clrb");
			break;
		case 0xff:
			if (op == AND)
				break;
			if (op == XOR) {
				ol("comb");
				break;
			}
		default:
			ot(s);
			doref('b', CONST, temp, 0);
		}
		break;
	case STACK:
		fprintf(code, " %sa ,s+\n %sb ,s+\n", s, s);
		sp += 2;
		break;
	default:
		error("compiler trouble");
		break;
	}
}


static void doref(int reg, int arg, expnode *val, int offset)
{
    ob(reg);
    ob(' ');
    deref(arg, val, offset);
    nl();
}


static void addoff(int offset)
{
    if (offset) {
        if (offset > 0)
        	ob('+');
        od(offset);
    }
}

void deref(int arg, int val, int offset)
{
	register expnode	*node;
	register symnode	*sn;
    int					sc;

    if (arg & INDIRECT)
    	ob('[');

    switch (arg & NOTIND) {
    case NODE:
        node = (expnode *) val;
        if (node->op == AMPER) {
            ob('#');
            deref(NODE, node->left, offset);
            return;
        }
        deref(node->op, node->val.sp, offset + node->modifier);
        return;
    case CONST:
        ob('#');
        od(val);
        break;
    case FREG:
        os("_flacc");
        addoff(offset);
        os(yind);
        break;
    case NAME:
        if ((sn = (symnode *) val) == NULL)
        	od(offset);
        else {
            switch (sc = sn->storage) {
            case AUTO:
                od(val = sn->offset-sp+offset);
                os(spind);
                break;
            case STATICD:
                if (!datflag)
                	ob((arg & INDIRECT) ? '>' : '<');
            case STATIC:
                olbl(sn->offset);
                goto dooff;
            case DIRECT:
            case EXTERND:
                if (!datflag)
                	ob((arg & INDIRECT) ? '>' : '<');
            case EXTERN:
            case EXTDEF:
                on(sn->sname);
dooff:
                addoff(offset);
                if (arg & INDIRECT ||
                 (!datflag && (sc != DIRECT && sc != EXTERND && sc != STATICD)))
                {
                	os(isfunction(sn->type) ? ",pcr" : yind);
                }
                break;
            default:
                error("storage error");
                break;
            }
        }
        break;
    case XIND:
    case YIND:
    case UIND:
        od(val += offset);
        ob(',');
        switch (arg & NOTIND) {
        case XIND:
        	ob('x');
        	break;
        case YIND:
        	ob('y');
        	break;
        case UIND:
        	ob('u');
        	break;
        }
        break;
    case STACK:
       os(spind);
       os("++");
       sp += 2;
       break;
    default:
       error("dereference");
    }

    if (arg & INDIRECT)
    	ob(']');
}

char *getrel(int op)
{
    switch (op) {
        default:    error("rel op");
        case EQ:    return "eq ";
        case NEQ:   return "ne ";
        case LEQ:   return "le ";
        case LT:    return "lt ";
        case GEQ:   return "ge ";
        case GT:    return "gt ";
        case ULEQ:  return "ls ";
        case ULT:   return "lo ";
        case UGEQ:  return "hs ";
        case UGT:   return "hi ";
    }
}

void ot(char *s)
{
    putc(' ',code);
    os(s);
}

void ol(char *s)
{
    ot(s);
    nl();
}

void nl(void)
{
    putc('\n', code);
}

void ob(int b)
{
    putc(b, code);
}

void os(char *s)
{
    fprintf(code, s);
}

void od(int16_t n)
{
/* Added UNIX def to prevent printing of words larger than 0xFFFF on systems
 * with sizeof(int)>2 - BGP.
 */
#ifdef UNIXY
    fprintf(code, "%d", n & 0xFFFF);
#else
    fprintf(code, "%d", n);
#endif
}

void label(int n)
{
    olbl(n);
    lastst = 0;
    nl();
}

void olbl(int n)
{
    ob(UNIQUE);
    od(n);
}

void on(char *s)
{
    fprintf(code, "%.8s", s);
}

int modstk(int nsp)
{
    register int	x;

    if ((x = nsp - sp)) {
        ot("leas ");
        od(x);
        os(spind);
        nl();
    }
    return nsp;
}

void nlabel(char *ptr, int scope)
{
    on(ptr);
    if (scope)
    	ob(':');
    nl();
}

static void outlea(int reg)
{
    fprintf(code, " lea%c ", reg);
}

void outstr(int reg, int l)
{
    outlea(reg);
    fprintf(code, "%c%d,pcr\n", UNIQUE, l);
}

