
#include "mamou.h"


/* 6309 register names */

typedef enum _h6309_reg
{
	RD    = 0,
	RX    = 1,
	RY    = 2,
	RU    = 3,
	RS    = 4,
	RPC   = 5,
	RW    = 6,
	RV    = 7,
	RA    = 8,
	RB    = 9,
	RCC	  = 10,
	RDP	  = 11,
	RZERO = 12,
	RPCR  = 13,
	RE    = 14,
	RF	  = 15
} h6309_reg;


/* static functions */
static int do_gen(assembler *as, int opcode, int amode, BP_Bool always_word);
static int do_indexed(assembler *as, int opcode);
static int abd_index(assembler *as, int pbyte);
static int rtype(assembler *as, int reg);
static int set_mode(assembler *as);
static h6309_reg regnum(assembler *as);

#define PAGE2	0x10
#define PAGE3	0x11
#define IPBYTE	0x9F	/* extended indirect postbyte */
#define SWI     0x3F

/* convert tfr/exg reg number into psh/pul format */
int     _regs[] = { 6,16,32,64,64,128,0,0,2,4,1,8,0};
int     rcycl[]= { 2,2, 2, 2, 2, 2,  0,0,1,1,1,1,0};

/* addressing modes */
#define IMMED   0       /* immediate */
#define IND     1       /* indexed */
#define INDIR   2       /* indirect */
#define OTHER   3       /* NOTA */
#define IMMED8  4       /* immediate - but 8 bytes*/



/*
 *      local_init --- machine specific initialization
 */
 
void local_init(void)
{
}


#if 0
/*
 *      do_op --- process mnemonic
 *
 *	Called with the base opcode and it's class. as->optr points to
 *	the beginning of the operand field.
 */
void do_op(int opcode, int class)
int opcode;	/* base opcode */
int class;	/* mnemonic class */
{
	int     dist;   /* relative branch distance */
	int     src,dst;/* source and destination registers */
	int     pbyte;  /* postbyte value */
	int     amode;  /* indicated addressing mode */
	int	j;
	int	result;

	amode = set_mode();     /* pickup indicated addressing mode */

	switch (class)
	{
#endif


int _inh(assembler *as, int opcode)
{
	/* 1. Emit opcode. */
	
	emit(as, opcode);

	print_line(as, 0, ' ', as->old_program_counter);


	return 0;
}



/* Page 2 inherent */

int _p2inh(assembler *as, int opcode)
{
	emit(as, PAGE2);
	

	return _inh(as, opcode);
}



/* Page 3 inherent */

int _p3inh(assembler *as, int opcode)
{
	emit(as, PAGE3);


	return _inh(as, opcode);
}



int _gen(assembler *as, int opcode)
{
	int     amode;  /* indicated addressing mode */

	amode = set_mode(as);     /* pickup indicated addressing mode */

	/* general addressing */
	do_gen(as, opcode, amode, BP_FALSE);
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _imgen(assembler *as, int opcode)
{
	BP_int32	result;
	int amode;  /* indicated addressing mode */
	int old;
	register char *p;

	amode = set_mode(as);     /* pickup indicated addressing mode */

	/* immediate addressing */
	if (amode != IMMED)
	{
		error(as, "Immediate as->operand Required");
		return 0;
	}
	as->optr++;
	evaluate(as, &result, &as->optr, 0);
	if ((hibyte(result) != 0x00) && (hibyte(result) != 0xFF))
	{
		error(as, "Result >255");

		return 0;
	}

	if (*as->optr++ != ',')
	{
		error(as, "Comma required between operands");
		return 0;
	}

	while(*as->optr == ' ') as->optr++;

	if (*as->optr == '#')
	{
		error(as, "Immediate Addressing Illegal");
		return 0;
	}
	if (*as->optr == '[')
	{
		emit(as, opcode + 0x60);
		do_indexed(as, result);
		print_line(as, 0, ' ', as->old_program_counter);
		return 0;
	}

	amode = OTHER;		/* default */
	p = as->optr;
	while (*p != EOS && *p != BLANK && *p != TAB)
	{
		/* any , before break */
		if (*p == ',')
		{
			amode = IND;    /* indexed addressing */
			break;
		}
		p++;
	}

	old = as->E_total;
	emit(as, lobyte(result));
	/* general addressing */
	do_gen(as, opcode, amode, BP_FALSE);

	/* Fix up output */
	as->E_bytes[old] = as->E_bytes[old + 1];
	as->E_bytes[old + 1] = result;
	as->P_bytes[0] = as->P_bytes[1];
	as->P_bytes[1] = result;

	if ((as->P_bytes[0] & 0xf0) == 0x10)
	{
		as->E_bytes[old] &= 0x0f;
		as->P_bytes[0] &= 0x0f;
	}
	else
	{
		as->E_bytes[old] |= 0x40;
		as->P_bytes[0] |= 0x40;
	}

	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _imm(assembler *as, int opcode)
{
	BP_int32	result;
	int amode;  /* indicated addressing mode */

	amode = set_mode(as);     /* pickup indicated addressing mode */

	/* immediate addressing */
	if (amode != IMMED)
	{
		error(as, "Immediate Operand Required");
		return 0;
	}
	as->optr++;
	evaluate(as, &result, &as->optr, 0);
	emit(as, opcode);
	if ((hibyte(result) != 0x00) && (hibyte(result) != 0xFF))
	{
		error(as, "Result >255");
		return 0;
	}
	emit(as, lobyte(result));
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _p3imm(assembler *as, int opcode)
{
	emit(as, PAGE3);
	return _imm(as, opcode);
}



int _rel(assembler *as, int opcode)
{
	BP_int32	result;
	int dist;

	/* short relative branches */
	evaluate(as, &result, &as->optr, 0);
	dist = result - (as->program_counter + 2);
	emit(as, opcode);
	if ((dist > 127 || dist < -128) && as->pass == 2)
	{
		error(as, "Branch out of Range");
		emit(as, lobyte(-2));
		return 0;
	}
	emit(as, lobyte(dist));
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _p2rel(assembler *as, int opcode)
{
	BP_int32	result;
	int dist;

	/* long relative branches */
	evaluate(as, &result, &as->optr, 0);
	dist = result - (as->program_counter + 4);
	emit(as, PAGE2);
	emit(as, opcode);

	if ((dist > -128) && (dist < 127)) as->allow_warnings = BP_TRUE;
	eword(as, dist);
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _p1rel(assembler *as, int opcode)
{
	BP_int32	result;
	int amode;
	int dist;

	amode = set_mode(as);     /* pickup indicated addressing mode */
	/* lbra and lbsr */
	if (amode == IMMED)
	{
		as->optr++; /* kludge for C compiler */
	}
	evaluate(as, &result, &as->optr, 0);
	dist = result - (as->program_counter + 3);
	if ((dist > -128) && (dist < 127)) as->allow_warnings = BP_TRUE;
	emit(as, opcode);
	eword(as, dist);
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _noimm(assembler *as, int opcode)
{
	int amode;

	amode = set_mode(as);     /* pickup indicated addressing mode */

	if (amode == IMMED)
	{
		error(as, "Immediate Addressing Illegal");
		return 0;
	}
	
	do_gen(as, opcode, amode, BP_FALSE);
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _noimm_16(assembler *as, int opcode)
{
	int amode;

	amode = set_mode(as);     /* pickup indicated addressing mode */

	if (amode == IMMED)
	{
		error(as, "Immediate Addressing Illegal");
		return 0;
	}
	
	do_gen(as, opcode, amode, BP_TRUE);
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _p2noimm(assembler *as, int opcode)
{
	emit(as, PAGE2);
	return _noimm(as, opcode);
}



int _p3noimm(assembler *as, int opcode)
{
	emit(as, PAGE3);
	return _noimm(as, opcode);
}



static int _pxgen(assembler *as, int opcode, int amode)
{
	BP_int32	result;

	if ((amode == IMMED) || (amode == IMMED8))
	{
		emit(as, opcode);
		as->optr++;
		evaluate(as, &result, &as->optr, 0);
		if (amode == IMMED)
		{
			eword(as, result);
		}
		else
		{
			emit(as, lobyte(result));
		}
		print_line(as, 0, ' ', as->old_program_counter);
		return 0;
	}
	do_gen(as, opcode, amode, BP_FALSE);
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _ldqgen(assembler *as, int opcode)
{
	int amode;

	amode = set_mode(as);
	if (amode == IMMED)
	{
	  BP_int32 result;

	  evaluate(as, &result, &as->optr, 0);
	  emit(as, 0xcd);
	  emit(as, (result >> 24) & 0xff);
	  emit(as, (result >> 16) & 0xff);
	  emit(as, (result >> 8) & 0xff);
	  emit(as, result & 0xff);
	  print_line(as, 0, ' ', as->old_program_counter);
	  return 0;
	}

	return _p2gen(as, 0xcc);
}



int _p2gen(assembler *as, int opcode)
{
	int amode;

	emit(as, PAGE2);
	amode = set_mode(as);     /* pickup indicated addressing mode */
	return _pxgen(as, opcode, amode);
}



int _p3gen(assembler *as, int opcode)
{
	int amode;

	emit(as, PAGE3);
	amode = set_mode(as);     /* pickup indicated addressing mode */
	return _pxgen(as, opcode, amode);
}



int _p3gen8(assembler *as, int opcode)
{
	int amode;

	emit(as, PAGE3);
	amode = set_mode(as);     /* pickup indicated addressing mode */
	if (amode == IMMED)
	{
		amode = IMMED8;
	}
	return _pxgen(as, opcode, amode);
}



int _rtor(assembler *as, int opcode)
{
	int src;
	int dst;

	/* tfr and exg */
	emit(as, opcode);
	src = regnum(as);
	while (alpha(*as->optr) || (*as->optr == '0'))
	{
		as->optr++;
	}
	if (src == ERR)
	{
		error(as, "Register Name Required");
		emit(as, 0);
		return 0;
	}
	if (*as->optr++ != ',')
	{
		error(as, "Missing ,");
		emit(as, 0);
		return 0;
	}
	dst = regnum(as);
	while (alpha(*as->optr))
	{
		as->optr++;
	}
	if (dst == ERR)
	{
		error(as, "Register Name Required");
		emit(as, 0);
		return 0;
	}
	if (src == RPCR || dst == RPCR)
	{
		error(as, "PCR illegal here");
		emit(as, 0);
		return 0;
	}
	if (dst == RZERO)
	{
		error(as, "Destination Zero register is illegal");
		return 0;
	}

	if ((src != RZERO) &&
	    ((src < 8 && dst >= 8) ||
	    (src >= 8 && dst < 8)))
	{
		error(as, "Register Size Mismatch");
		emit(as, 0);
		return 0;
	}
	if (*as->optr && (*as->optr != BLANK) && (*as->optr != TAB))
	{
		error(as, "Invalid trailing text");
		return 0;
	}

	emit(as, (src << 4) + dst);
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _p2rtor(assembler *as, int opcode)
{
	emit(as, PAGE2);
	return _rtor(as, opcode);
}



int _p3rtor(assembler *as, int opcode)
{
	int src;
	int dst;
	int form = 0;

	src = regnum(as);
	while (alpha(*as->optr) || (*as->optr == '0'))
	{
		as->optr++;
	}
	if (src == ERR)
	{
		error(as, "Register Name Required");
		return 0;
	}

	switch (*as->optr)
	  {
	  case '+':
	    form = 1;
	    as->optr++;
	    break;

	  case '-':
	    form = 2;
	    as->optr++;
	    break;
	    
	  case ',':
	    break;

	  default:
	    error(as, "Invalid text");
	    return 0;
	  }

	if (*as->optr++ != ',')
	{
		error(as, "Missing ,");
		emit(as, 0);
		return 0;
	}

	dst = regnum(as);
	while (alpha(*as->optr))
	{
		as->optr++;
	}
	if (dst == ERR)
	{
		error(as, "Register Name Required");
		return 0;
	}
	if (src == RPCR || dst == RPCR)
	{
		error(as, "PCR illegal here");
		return 0;
	}
	if (dst == RZERO)
	{
		error(as, "Destination Zero register is illegal");
		return 0;
	}

	if ((dst > 4) || ((src > 4) && (src != RZERO)))
	{
		error(as, "Invalid Register");
		return 0;
	}

	switch (*as->optr)
	  {
	  case '+':
	    if (form == 0) {
	      form = 4;
	      as->optr++;
	    } else if (form == 1) {
	      as->optr++;
	    } else {
	      error(as, "Unexpected trailing '+'");
	      return 0;
	    }
	    break;
	    
	  case '-':
	    if (form == 2) {
	      as->optr++;
	    } else {
	      error(as, "Unexpected trailing '-'");
	      return 0;
	    }
	    break;
	    
	  default:
	    if (form == 1) {
	      form = 3;
	    } else {
	      error(as, "Expected addressing mode '+' or '-'");
	      return 0;
	    }
	    break;

	  }

	if (*as->optr && (*as->optr != BLANK) && (*as->optr != TAB))
	{
		error(as, "Invalid trailing text");
		return 0;
	}

	emit(as, PAGE3);
	emit(as, opcode + form - 1);
	emit(as, (src << 4) + dst);
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _indexed(assembler *as, int opcode)
{
	int amode;

	amode = set_mode(as);     /* pickup indicated addressing mode */
	/* indexed addressing only */
	if (*as->optr == '#')
	{
		as->optr++;         /* kludge city */
		amode = IND;
	}
	if (amode != IND)
	{
		error(as, "Indexed Addressing Required");
		return 0;
	}
	do_indexed(as, opcode);
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _rlist(assembler *as, int opcode)
{
	int pbyte;
	int j;

	/* pushes and pulls */
	if (*as->operand == EOS)
	{
		error(as, "Register List Required");
		return 0;
	}
	emit(as, opcode);
	pbyte = 0;
	do
	{
		j = regnum(as);
		if (j == ERR || j == RPCR)
		{
			error(as, "Illegal Register Name");
		}
#if 0
		else if (j == RS && (opcode == 52))
		{
			error(as, "Can't Push S on S");
		}
		else if (j == RU && (opcode == 54))
		{
			error(as, "Can't Push U on U");
		}
		else if (j == RS && (opcode == 53))
		{
			error(as, "Can't Pull S from S");
		}
		else if (j == RU && (opcode == 55))
		{
			error(as, "Can't Pull U from U");
		}
#endif
		else
		{
			pbyte |= _regs[j];
			as->cumulative_cycles += rcycl[j];
		}
		while(*as->optr != EOS && alpha(*as->optr))
		{
			as->optr++;
		}
	} while(*as->optr++ == ',');
	emit(as, lobyte(pbyte));
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _longimm(assembler *as, int opcode)
{
	BP_int32 result;
	int amode;

	amode = set_mode(as);     /* pickup indicated addressing mode */
	if (amode == IMMED)
	{
		emit(as, opcode);
		as->optr++;
		evaluate(as, &result, &as->optr, 0);
		eword(as, result);
	}
	else
	{
		do_gen(as, opcode, amode, BP_FALSE);
	}
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _grp2(assembler *as, int opcode)
{
	BP_int32 result;
	int amode;

	amode = set_mode(as);     /* pickup indicated addressing mode */

	if (amode == IND)
	{
		do_indexed(as, opcode + 0x60);
		print_line(as, 0, ' ', as->old_program_counter);
		return 0;
	}
	else if (amode == INDIR)
	{
		as->optr++;
		emit(as, opcode + 0x60);
		emit(as, IPBYTE);
		evaluate(as, &result, &as->optr, 0);
		eword(as, result);
		as->cumulative_cycles += 7;
		if (*as->optr == ']')
		{
			as->optr++;
			print_line(as, 0, ' ', as->old_program_counter);
			return 0;
		}
		error(as, "Missing ']'");
		return 0;
	}
	evaluate(as, &result, &as->optr, 0);
	if (as->force_word)
	{
		emit(as, opcode + 0x70);
		eword(as, result);
		as->cumulative_cycles += 3;
	}
	else if (as->force_byte)
	{
		if (hibyte(result) != as->DP)
		{
			error(as, "as->DP out of range");
			return 0;
		}
		emit(as, opcode);
		emit(as, lobyte(result));
		as->cumulative_cycles += 2;
	}
	else if (hibyte(result) == as->DP)
	{
		emit(as, opcode);
		emit(as, lobyte(result));
		as->cumulative_cycles += 2;
	}
	else
	{
		emit(as, opcode + 0x70);
		eword(as, result);
		as->cumulative_cycles += 3;
	}
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _grp2_16(assembler *as, int opcode)
{
	BP_int32 result;
	int amode;

	amode = set_mode(as);     /* pickup indicated addressing mode */

	if (amode == IND)
	{
		do_indexed(as, opcode + 0x60);
		print_line(as, 0, ' ', as->old_program_counter);
		return 0;
	}
	else if (amode == INDIR)
	{
		as->optr++;
		emit(as, opcode + 0x60);
		emit(as, IPBYTE);
		evaluate(as, &result, &as->optr, 0);
		eword(as, result);
		as->cumulative_cycles += 7;
		if (*as->optr == ']')
		{
			as->optr++;
			print_line(as, 0, ' ', as->old_program_counter);
			return 0;
		}
		error(as, "Missing ']'");
		return 0;
	}
	evaluate(as, &result, &as->optr, 0);

	{
		emit(as, opcode + 0x70);
		eword(as, result);
		as->cumulative_cycles += 3;
	}
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



int _sys(assembler *as, int opcode)
{
	BP_int32	result;

	/* system call */
	emit(as, PAGE2);
	emit(as, opcode);
	evaluate(as, &result, &as->optr, 0);
	emit(as, lobyte(result));
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



/*
 *      do_gen --- process general addressing mode stuff
 */
static int do_gen(assembler *as, int op, int mode, BP_Bool always_word)
{
	BP_int32	result;

	if (mode == IMMED)
	{
		as->optr++;
		emit(as, op);
		evaluate(as, &result, &as->optr, 0);
		if ((hibyte(result) != 0x00) && (hibyte(result) != 0xFF))
		{
			error(as, "Result >255");

			return 0;
		}
		emit(as, lobyte(result));

		return 0;
	}
	else if (mode == IND)
	{
		do_indexed(as, op + 0x20);

		return 0;
	}
	else if (mode == INDIR)
	{
		as->optr++;
		emit(as, op + 0x20);
		emit(as, IPBYTE);
		evaluate(as, &result, &as->optr, 0);
		eword(as, result);
		as->cumulative_cycles += 7;
		if (*as->optr == ']')
		{
			as->optr++;
			return 0;
		}
		error(as, "Missing ']'");
		return 0;
	}
	else if (mode == OTHER)
	{
		evaluate(as, &result, &as->optr, 0);
		if (as->force_word || always_word == BP_TRUE)
		{
			if ((hibyte(result) == as->DP)) as->allow_warnings = BP_TRUE;

			emit(as, op + 0x30);
			eword(as, result);
			as->cumulative_cycles += 3;
			return 0;
		}
		if (as->force_byte)
		{
			emit(as, op + 0x10);
			if (hibyte(result) != as->DP)
			{
				error(as, "as->DP out of range");
				return 0;
			}
			emit(as, lobyte(result));
			as->cumulative_cycles += 2;
			return 0;
		}
		if (hibyte(result) == as->DP)
		{
			emit(as, op + 0x10);
			emit(as, lobyte(result));
			as->cumulative_cycles += 2;
			return 0;
		}
		else
		{
			emit(as, op + 0x30);
			eword(as, result);
			as->cumulative_cycles += 3;
			return 0;
		}
	}
	else
	{
		error(as, "Unknown Addressing Mode");
		return 0;
	}
}



/*
 *      do_indexed --- handle all weird stuff for indexed addressing
 */
static int do_indexed(assembler *as, int op)
{
	int     pbyte;
	int     j,k;
	int     predec,pstinc;
	BP_int32	result;

	as->cumulative_cycles += 2;    /* indexed is always 2+ base cycle count */
	predec = 0;
	pstinc = 0;
	pbyte = 128;
	emit(as, op);
	if (*as->optr == '[')
	{
		pbyte |= 0x10;    /* set indirect bit */
		as->optr++;
		if (!any((char)']', as->optr))
		{
			error(as, "Missing ']'");
		}
		as->cumulative_cycles += 3;    /* indirection takes this much longer */
	}
	j = regnum(as);
	if (j == RA)
	{
		as->cumulative_cycles++;
		abd_index(as, pbyte + 6);
		return 0;
   	}
	if (j == RB)
	{
		as->cumulative_cycles++;
		abd_index(as, pbyte + 5);
		return 0;
	}
	if (j == RD)
	{
		as->cumulative_cycles += 4;
		abd_index(as, pbyte + 11);
		return 0;
	}
	if (j == RE && as->h6309 == 1)
	{
		as->cumulative_cycles++;
		abd_index(as, pbyte + 7);
		return 0;
   	}
	if (j == RF && as->h6309 == 1)
	{
		as->cumulative_cycles++;
		abd_index(as, pbyte + 10);
		return 0;
	}
	if (j == RW && as->h6309 == 1)
	{
		as->cumulative_cycles += 4;
		abd_index(as, pbyte + 14);
		return 0;
	}

	evaluate(as, &result, &as->optr, 0);
	as->optr++;
	while (*as->optr == '-')
	{
		predec++;
		as->optr++;
	}
	j = regnum(as);
	while (alpha(*as->optr))
	{
		as->optr++;
	}
	while (*as->optr == '+')
	{
		pstinc++;
		as->optr++;
	}
	if (j == RPC || j == RPCR)
	{
#if 0
		int as->force_word = NO;
#endif

		if (as->force_byte == BP_FALSE)
		{
			as->force_word = BP_TRUE;
		}
		if (pstinc || predec)
		{
			error(as, "Auto Inc/Dec Illegal on PC");
			return 0;
		}

		/* PC or PCR addressing */
		if (as->force_word)
		{
			emit(as, pbyte + 13);
			eword(as, result - (as->program_counter + 2));
			as->cumulative_cycles += 5;
			return 0;
		}
		if (as->force_byte)
		{
			emit(as, pbyte + 12);
			/* as->allow_warnings */
			emit(as, lobyte(result - (as->program_counter + 1)));
			as->cumulative_cycles++;
			return 0;
		}
		k = result - (as->program_counter + 2);
		if (k >= -128 && k <= 127)
		{
			emit(as, pbyte + 12);
			emit(as, lobyte(result - (as->program_counter + 1)));
			as->cumulative_cycles++;
			return 0;
		}
		else
		{
			emit(as, pbyte + 13);
			eword(as, result - (as->program_counter + 2));
			as->cumulative_cycles += 5;
			return 0;
		}
	}
	if (predec || pstinc)
	{
		if (result != 0)
		{
			error(as, "Offset must be Zero");
			return 0;
		}
		if (predec > 2 || pstinc > 2)
		{
			error(as, "Auto Inc/Dec by 1 or 2 only");
			return 0;
		}
		if ((predec == 1 && (pbyte & 0x10) != 0) ||
			(pstinc == 1 && (pbyte & 0x10) != 0))
		{
			error(as, "No Auto Inc/Dec by 1 for Indirect");
			return 0;
		}
		if (predec && pstinc)
		{
			error(as, "Can't do both!");
			return 0;
		}

		j = rtype(as, j);
		if (j < 0x100)
		{
			if (predec)
			{
				pbyte += predec + 1;
			}
			if (pstinc)
			{
				pbyte += pstinc - 1;
			}
		  
			pbyte += j;
			emit(as, pbyte);
			as->cumulative_cycles += 1 + predec + pstinc;
			return 0;
		}

		if ((predec != 2) && (pstinc != 2))
		{
			error(as, "Only ,--W and ,W++ allowed for W indexing");
			return 0;
		}

		/* handle ,W++  ,--W */
		if (pbyte & 0x10)  /* [,W++] */
		{
		  if (predec == 2) {
		    emit(as, 0xf0);
		    as->cumulative_cycles += 6;
		    return 0;
		  } else if (pstinc == 2) {
		    emit(as, 0xd0);
		    as->cumulative_cycles += 6;
		    return 0;
		  }
		}
		else		/* ,W++ */
		{
		  if (predec == 2) {
		    emit(as, 0xef);
		    as->cumulative_cycles += 2;
		    return 0;
		  } else if (pstinc == 2) {
		    emit(as, 0xcf);
		    as->cumulative_cycles += 2;
		    return 0;
		  }
		}
	}
	j = rtype(as, j);
	if (j != 0x100) {
	  pbyte += j;
	  if (as->force_word)
	    {
	      if ((hibyte(result) == 0)) as->allow_warnings = BP_TRUE;
	      emit(as, pbyte + 0x09);
	      eword(as, result);
	      as->cumulative_cycles += 4;
	      return 0;
	    }
	  if (as->force_byte)
	    {
	      emit(as, pbyte + 0x08);
	      if (result <-128 || result >127)
		{
		  error(as, "value out of range 2");
		  return 0;
		}
	      if ((result >= -16) && (result <= 15) && ((pbyte & 16) == 0)) {
		as->allow_warnings = BP_TRUE;
	      }
	      emit(as, lobyte(result));
	      as->cumulative_cycles++;
	      return 0;
	    }
	  if (result == 0)
	    {
	      emit(as, pbyte + 0x04);
	      return 0;
	    }
	  if ((result >= -16) && (result <= 15) && ((pbyte & 16) == 0))
	    {
	      pbyte &= 127;
	      pbyte += result & 31;
	      emit(as, pbyte);
	      as->cumulative_cycles++;
	      return 0;
	    }
	  if (result >= -128 && result <= 127)
	    {
	      emit(as, pbyte + 0x08);
	      emit(as, lobyte(result));
	      as->cumulative_cycles++;
	      return 0;
	    }
	  emit(as, pbyte + 0x09);
	  eword(as, result);
	  as->cumulative_cycles += 4;
	  return 0;
	} else {		/* ,W  n,W [n,W] */

	  if (as->force_byte) {
	    error(as, "Byte indexing is invalid for W");
	    return 0;
	  }

	  if (pbyte & 0x10 && as->h6309 == 1) {	/* [,W] */
	    if (as->force_word || (result != 0)) {
	      emit(as, 0xb0);
	      eword(as, result);
	      as->cumulative_cycles += 6;
	      return 0;
	    }

	    emit(as, 0x90);
	    return 0;
	  } else {		/* ,W */
	    if (as->force_word || (result != 0) && as->h6309 == 1) {
	      emit(as, 0xaf);
	      eword(as, result);
	      as->cumulative_cycles += 3;
	      return 0;
	    }

	    emit(as, 0x8f);
	    return 0;
	  }
	}
}



/*
 *      abd_index --- a,b or d indexed
 */
static int abd_index(assembler *as, int pbyte)
{
	int     k;

	as->optr += 2;
	k = regnum(as);
	k = rtype(as, k);
	if (k == 0x100)
	{
		error(as, "Cannot use W for register indirect");
		return 0;
	}
	pbyte += k;
	emit(as, pbyte);
	return 0;
}



/*
 *      rtype --- return register type in post-byte format
 */
static int rtype(assembler *as, int r)
{
	switch(r)
	{
		case RX:
			return(0x00);
		case RY:
			return(0x20);
		case RU:
			return(0x40);
		case RS:
			return(0x60);
		case RW:
		 	if (as->h6309 == 1) return(0x100);
	}
	error(as, "Illegal Register for Indexed");
	return 0;
}



/*
 *      set_mode --- determine addressing mode from operand field
 */
static int set_mode(assembler *as)
{
	register char *p;

	if (*as->operand == '#')
	{
		return(IMMED);          /* immediate addressing */
	}
	p = as->operand;
	while (*p != EOS && *p != BLANK && *p != TAB)
	{
		/* any , before break */
		if (*p == ',')
		{
			return(IND);    /* indexed addressing */
		}
		p++;
	}
	if (*as->operand == '[')
	{
		return(INDIR);          /* indirect addressing */
	}
	return(OTHER);                  /* NOTA */
}



/*
 *      regnum --- return register number of *as->optr
 */
static h6309_reg regnum(assembler *as)
{
	if (head(as->optr, "D"))
	{
		return(RD);
	}
	if (head(as->optr, "d"))
	{
		return(RD);
	}
	if (head(as->optr, "X"))
	{
		return(RX);
	}
	if (head(as->optr, "x"))
	{
		return(RX);
	}
	if (head(as->optr, "Y"))
	{
		return(RY);
	}
	if (head(as->optr, "y"))
	{
		return(RY);
	}
	if (head(as->optr, "U"))
	{
		return(RU);
	}
	if (head(as->optr, "u"))
	{
		return(RU);
	}
	if (head(as->optr, "S"))
	{
		return(RS);
	}
	if (head(as->optr, "s"))
	{
		return(RS);
	}
	if (head(as->optr, "PC"))
	{
		return(RPC);
	}
	if (head(as->optr, "pc"))
	{
		return(RPC);
	}
	if (head(as->optr, "W") && as->h6309 == 1)
	{
		return(RW);
	}
	if (head(as->optr, "w") && as->h6309 == 1)
	{
		return(RW);
	}
	if (head(as->optr, "V") && as->h6309 == 1)
	{
		return(RV);
	}
	if (head(as->optr, "v") && as->h6309 == 1)
	{
		return(RV);
	}

	if (head(as->optr, "PCR"))
	{
		return(RPCR);
	}
	if (head(as->optr, "pcr"))
	{
		return(RPCR);
	}
	if (head(as->optr, "A"))
	{
		return(RA);
	}
	if (head(as->optr, "a"))
	{
		return(RA);
	}
	if (head(as->optr, "B"))
	{
		return(RB);
	}
	if (head(as->optr, "b"))
	{
		return(RB);
	}
	if (head(as->optr, "CC"))
	{
		return(RCC);
	}
	if (head(as->optr, "cc"))
	{
		return(RCC);
	}
	if (head(as->optr, "DP"))
	{
		return(RDP);
	}
	if (head(as->optr, "dp"))
	{
		return(RDP);
	}
	if (head(as->optr, "0") && as->h6309 == 1)
	{
		return(RZERO);
	}
	if (head(as->optr, "Z") && as->h6309 == 1)
	{
		return(RZERO);
	}
	if (head(as->optr, "z") && as->h6309 == 1)
	{
		return(RZERO);
	}
	if (head(as->optr, "E") && as->h6309 == 1)
	{
		return(RE);
	}
	if (head(as->optr, "e") && as->h6309 == 1)
	{
		return(RE);
	}
	if (head(as->optr, "F") && as->h6309 == 1)
	{
		return(RF);
	}
	if (head(as->optr, "f") && as->h6309 == 1)
	{
		return(RF);
	}
	return(ERR);
}

