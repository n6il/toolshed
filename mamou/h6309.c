/***************************************************************************
* h6309.c: Hitach 6309 assembly routines
*
* $Id$
*
* The Mamou Assembler - A Hitachi 6309 assembler
*
* (C) 2004 Boisy G. Pitre
***************************************************************************/

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
	RF	  = 15,
	RT 	= 16		/* X9 extension */
} h6309_reg;

/* static functions */
static int do_gen(assembler *as, int opcode, int amode, int always_word);
static int do_indexed(assembler *as, int opcode);
static int abd_index(assembler *as, int pbyte);
static int reg_type(assembler *as, int reg);
static int addressing_mode(assembler *as);
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



/*!
	@function local_init
	@discussion Machine specific initialization
 */
void local_init(void)
{
}


/*!
	@function _inh
	@discussion Inherent instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _inh(assembler *as, int opcode)
{
	/* Emit opcode. */
	emit(as, opcode);

	/* Print the line. */	
	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}


/*!
	@function _p2inh
	@discussion Part 2 inherent instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p2inh(assembler *as, int opcode)
{
	/* Emit leading opcode. */
	emit(as, PAGE2);
	
	/* Let the primary function handle the rest. */
	return _inh(as, opcode);
}


/*!
	@function _p3inh
	@discussion Part 3 inherent instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p3inh(assembler *as, int opcode)
{
	/* Emit leading opcode. */	
	emit(as, PAGE3);

	/* Let the primary function handle the rest. */	
	return _inh(as, opcode);
}


/*!
	@function _gen
	@discussion Generic instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _gen(assembler *as, int opcode)
{
	int     amode;
	
     /* Get addressing mode. */
	amode = addressing_mode(as);
	
	/* Do general addressing */
	do_gen(as, opcode, amode, 0);
	
	/* Print the line. */
	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}


/*!
	@function _imgen
	@discussion Immediate generic instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _imgen(assembler *as, int opcode)
{
	int	result;
	int	amode;
	int	old;
	char		*p;
	
	/* Get indicated addressing mode. */
	amode = addressing_mode(as);
	
#if 0
	/* Verify immediate addressing. */
	if (amode != IMMED)
	{
		error(as, "immediate operand required");

		return 0;
	}
	
	/* skip over # */
	as->line.optr++;
#else
	/* Make use of immediate addressing # optional */
	if (*(as->line.optr) == '#')
	{
		/* skip over # */
		as->line.optr++;
	}
#endif

	evaluate(as, &result, &as->line.optr, 0);
	
	if ((hibyte(result) != 0x00) && (hibyte(result) != 0xFF))
	{
		error(as, "result >255");

		return 0;
	}

	if (*as->line.optr != ',' && *as->line.optr != ';')
	{
		error(as, "comma or semicolon required between operands");

		return 0;
	}
	
	as->line.optr++;

	if (*as->line.optr == '[')
	{
		emit(as, opcode + 0x60);
		do_indexed(as, result);
		print_line(as, 0, ' ', as->old_program_counter);

		return 0;
	}

	amode = OTHER;		/* default */
	p = as->line.optr;

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

	
	/* General addressing */
	do_gen(as, opcode, amode, 0);

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


/*!
	@function _imm
	@discussion Immediate instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */

int _imm(assembler *as, int opcode)
{
	int	result;
	int amode;  /* indicated addressing mode */

	amode = addressing_mode(as);     /* pickup indicated addressing mode */

	/* Immediate addressing ONLY. */
	if (amode != IMMED)
	{
		error(as, "immediate operand required");

		return 0;
	}

	/* skip over # */
	as->line.optr++;
	evaluate(as, &result, &as->line.optr, 0);
	emit(as, opcode);

	if ((hibyte(result) != 0x00) && (hibyte(result) != 0xFF))
	{
		error(as, "result >255");
		return 0;
	}

	emit(as, lobyte(result));
	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}


/*!
	@function _p3imm
	@discussion Part 3 immediate instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p3imm(assembler *as, int opcode)
{
	emit(as, PAGE3);

	return _imm(as, opcode);
}


/*!
	@function _rel
	@discussion Relative instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _rel(assembler *as, int opcode)
{
	int	result;
	int dist;

	/* Short relative branches */
	evaluate(as, &result, &as->line.optr, 0);
	dist = result - (as->program_counter + 2);
	emit(as, opcode);
	
	if ((dist > 127 || dist < -128) && as->pass == 2)
	{
		error(as, "branch out of range");
		emit(as, lobyte(-2));

		return 0;
	}

	emit(as, lobyte(dist));

	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}



/*!
	@function _p2rel
	@discussion Part 2 relative instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p2rel(assembler *as, int opcode)
{
	int	result;
	int			dist;

	/* Long relative branches */
	evaluate(as, &result, &as->line.optr, 0);
	dist = result - (as->program_counter + 4);
	emit(as, PAGE2);
	emit(as, opcode);

	if ((dist > -128) && (dist < 127))
	{
		as->line.has_warning = 1;
	}
	
	eword(as, dist);
	
	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*!
	@function _p1rel
	@discussion Part 1 relative instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p1rel(assembler *as, int opcode)
{
	int	result;
	int amode;
	int dist;

	amode = addressing_mode(as);     /* pickup indicated addressing mode */

	/* lbra and lbsr */
	if (amode == IMMED)
	{
		as->line.optr++; /* kludge for C compiler */
	}

	evaluate(as, &result, &as->line.optr, 0);
	dist = result - (as->program_counter + 3);

	
	if ((dist > -128) && (dist < 127))
	{
		as->line.has_warning = 1;
	}
	
	emit(as, opcode);
	eword(as, dist);

	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}



/*!
	@function _noimm
	@discussion Non immediate instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _noimm(assembler *as, int opcode)
{
	int amode;

	amode = addressing_mode(as);     /* pickup indicated addressing mode */

	if (amode == IMMED)
	{
		error(as, "immediate addressing illegal");

		return 0;
	}
	
	do_gen(as, opcode, amode, 0);
	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}


/*!
	@function _p2noimm
	@discussion Part 2 non-immediate instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p2noimm(assembler *as, int opcode)
{
	emit(as, PAGE2);
	
	return _noimm(as, opcode);
}


/*!
	@function _p3noimm
	@discussion Part 3 non-immediate instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p3noimm(assembler *as, int opcode)
{
	emit(as, PAGE3);
	
	return _noimm(as, opcode);
}


/*!
	@function _pxgen
	@discussion Part X generic instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
static int _pxgen(assembler *as, int opcode, int amode)
{
	int	result;
	
	if ((amode == IMMED) || (amode == IMMED8))
	{
		emit(as, opcode);
		as->line.optr++;
		evaluate(as, &result, &as->line.optr, 0);

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

	do_gen(as, opcode, amode, 0);

	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}


/*!
	@function _ldqgen
	@discussion LDQ generic instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _ldqgen(assembler *as, int opcode)
{
	int amode;

	amode = addressing_mode(as);
	
	if (amode == IMMED)
	{
		int result;

		as->line.optr++;
		evaluate(as, &result, &as->line.optr, 0);
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


/*!
	@function _p2gen
	@discussion Part 2 generic instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p2gen(assembler *as, int opcode)
{
	int amode;

	emit(as, PAGE2);
	amode = addressing_mode(as);     /* pickup indicated addressing mode */

	return _pxgen(as, opcode, amode);
}


/*!
	@function _p3gen
	@discussion Part 3 generic instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p3gen(assembler *as, int opcode)
{
	int amode;

	emit(as, PAGE3);
	amode = addressing_mode(as);     /* pickup indicated addressing mode */

	return _pxgen(as, opcode, amode);
}


/*!
	@function _p3gen8
	@discussion Part 3 generic 8-bit instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p3gen8(assembler *as, int opcode)
{
	int amode;

	emit(as, PAGE3);
	
	amode = addressing_mode(as);     /* pickup indicated addressing mode */

	if (amode == IMMED)
	{
		amode = IMMED8;
	}
	
	return _pxgen(as, opcode, amode);
}


/*!
	@function _rtor
	@discussion Register-to-register instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _rtor(assembler *as, int opcode)
{
	int src, srcsz;
	int dst, dstsz;

	/* tfr and exg */
	emit(as, opcode);
	
	src = regnum(as);
	
	while (alpha(*as->line.optr) || (*as->line.optr == '0'))
	{
		as->line.optr++;
	}
	
	if (src == ERR)
	{
		error(as, "register name required");
		emit(as, 0);
		return 0;
	}
	
	if (*as->line.optr++ != ',')
	{
		error(as, "missing ,");
		emit(as, 0);
		return 0;
	}
	
	dst = regnum(as);
	
	while (alpha(*as->line.optr))
	{
		as->line.optr++;
	}
	
	if (dst == ERR)
	{
		error(as, "register name required");
		emit(as, 0);

		return 0;
	}
	
	if (src == RPCR || dst == RPCR)
	{
		error(as, "PCR illegal here");
		emit(as, 0);

		return 0;
	}

	if (src == RT) src = RPCR;	/* _DIRTY_ hack for T register */
	if (dst == RT) dst = RPCR;	
	
	if (dst == RZERO)
	{
		error(as, "destination zero register is illegal");

		return 0;
	}

	srcsz = ((src & 8) && (src != RPCR)) ? 8 : 16;
	dstsz = ((dst & 8) && (dst != RPCR)) ? 8 : 16;
	if ((src == RZERO) && (dstsz == 8)) srcsz = 8;

	if ((srcsz != dstsz) && (opcode == 30)) /* EXG disallows R16->R8 */
	{
		error(as, "register size mismatch");
		emit(as, 0);

		return 0;
	}

	if (*as->line.optr && (*as->line.optr != BLANK) && (*as->line.optr != TAB))
	{
		error(as, "invalid trailing text");

		return 0;
	}

	emit(as, (src << 4) + dst);
	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*!
	@function _p2rtor
	@discussion Part 2 register-to-register instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p2rtor(assembler *as, int opcode)
{
	emit(as, PAGE2);
	
	return _rtor(as, opcode);
}


/*!
	@function _p3rtor
	@discussion Part 3 register-to-register instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _p3rtor(assembler *as, int opcode)
{
	int src;
	int dst;
	int form = 0;

	src = regnum(as);

	while (alpha(*as->line.optr) || (*as->line.optr == '0'))
	{
		as->line.optr++;
	}

	if (src == ERR)
	{
		error(as, "register name required");

		return 0;
	}

	switch (*as->line.optr)
	{
		case '+':
			form = 1;
			as->line.optr++;
			break;

		case '-':
			form = 2;
			as->line.optr++;
			break;
	    
		case ',':
			break;

		default:
			error(as, "invalid text");
			return 0;
	}

	if (*as->line.optr++ != ',')
	{
		error(as, "missing ,");
		emit(as, 0);

		return 0;
	}

	dst = regnum(as);
	while (alpha(*as->line.optr))
	{
		as->line.optr++;
	}

	if (dst == ERR)
	{
		error(as, "register name required");

		return 0;
	}

	if (src == RPCR || dst == RPCR)
	{
		error(as, "PCR illegal here");

		return 0;
	}

	if (dst == RZERO)
	{
		error(as, "destination zero register is illegal");

		return 0;
	}

	if ((dst > 4) || ((src > 4) && (src != RZERO)))
	{
		error(as, "invalid register");

		return 0;
	}

	switch (*as->line.optr)
	{
		case '+':
			if (form == 0)
			{
				form = 4;
				as->line.optr++;
			} else if (form == 1)
			{
				as->line.optr++;
			}
			else
			{
				error(as, "unexpected trailing '+'");
				return 0;
			}
			break;
	    
		case '-':
			if (form == 2)
			{
				as->line.optr++;
			}
			else
			{
				error(as, "unexpected trailing '-'");
				return 0;
			}
			break;
	    
	  default:
		  if (form == 1)
		  {
			  form = 3;
		  }
		  else
		  {
			  error(as, "expected addressing mode '+' or '-'");

			  return 0;
		  }
		  break;
	}

	if (*as->line.optr && (*as->line.optr != BLANK) && (*as->line.optr != TAB))
	{
		error(as, "invalid trailing text");

		return 0;
	}

	emit(as, PAGE3);
	emit(as, opcode + form - 1);
	emit(as, (src << 4) + dst);
	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}


/*!
	@function _indexed
	@discussion Indexed instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _indexed(assembler *as, int opcode)
{
	int amode;

	amode = addressing_mode(as);     /* pickup indicated addressing mode */

	/* indexed addressing only */

#if 0
	if (*as->line.optr == '#')
	{
		as->line.optr++;         /* kludge city */
		amode = IND;
	}
#endif

	if (amode != IND)
	{
		error(as, "indexed addressing required");

		return 0;
	}

	do_indexed(as, opcode);
	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*!
	@function _rlist
	@discussion Register list instruction handler used by puls/pshs
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _rlist(assembler *as, int opcode)
{
	int pbyte;
	int j;
	
	/* pushes and pulls */
	if (*as->line.operand == EOS)
	{
		error(as, "register list required");

		return 0;
	}

	emit(as, opcode);
	pbyte = 0;

	do
	{
		j = regnum(as);
		
		/* check for valid registers which can be used in push/pull operations */
		if (!(j == RPC || j == RU || j == RY || j == RX || j == RDP || j == RD || j == RA || j == RB ||  j == RCC))
		{
			error(as, "illegal register name");
		}
#if 0
		else if (j == RS && (opcode == 52))
		{
			error(as, "can't push S on S");
		}
		else if (j == RU && (opcode == 54))
		{
			error(as, "can't push U on U");
		}
		else if (j == RS && (opcode == 53))
		{
			error(as, "can't pull S from S");
		}
		else if (j == RU && (opcode == 55))
		{
			error(as, "can't pull U from U");
		}
#endif
		else
		{
			pbyte |= _regs[j];
			as->cumulative_cycles += rcycl[j];
		}
		while(*as->line.optr != EOS && alpha(*as->line.optr))
		{
			as->line.optr++;
		}
	} while(*as->line.optr++ == ',');

	emit(as, lobyte(pbyte));
	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}


/*!
	@function _longimm
	@discussion Long immediate instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _longimm(assembler *as, int opcode)
{
	int result;
	int amode;

	amode = addressing_mode(as);     /* pickup indicated addressing mode */

	if (amode == IMMED)
	{
		emit(as, opcode);
		as->line.optr++;
		evaluate(as, &result, &as->line.optr, 0);
		eword(as, result);
	}
	else
	{
		do_gen(as, opcode, amode, 0);
	}

	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*!
	@function _grp2
	@discussion Generic instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _grp2(assembler *as, int opcode)
{
	int result;
	int amode;

	amode = addressing_mode(as);     /* pickup indicated addressing mode */

	if (amode == IND)
	{
		/* Indexed mode (i.e. $5,y) */
		do_indexed(as, opcode + 0x60);
		print_line(as, 0, ' ', as->old_program_counter);
		return 0;
	}
	else
	if (amode == INDIR)
	{
		/* Indrect mode (i.e. [$FFFE]) */
		as->line.optr++;
		emit(as, opcode + 0x60);
		emit(as, IPBYTE);
		evaluate(as, &result, &as->line.optr, 0);
		eword(as, result);
		as->cumulative_cycles += 7;

		if (*as->line.optr == ']')
		{
			as->line.optr++;
			print_line(as, 0, ' ', as->old_program_counter);
			return 0;
		}

		error(as, "missing ']'");

		return 0;
	}

	/* Evaluate result */
	evaluate(as, &result, &as->line.optr, 0);
	
	/* Check for inconsistency in force mode and DP */
	if (as->line.force_byte == 1 && hibyte(result) != as->DP)
	{
		error(as, "DP out of range");
		
		return 0;
	}

	if (as->line.force_word == 1 || hibyte(result) != as->DP)
	{
		if ((hibyte(result) == as->DP))
		{
			as->line.has_warning = 1;
		}
		
		emit(as, opcode + 0x70);
		eword(as, result);
		as->cumulative_cycles += 3;
	}
	else
	{
		if (hibyte(result) != as->DP)
		{
			error(as, "DP out of range");

			return 0;
		}
		
		emit(as, opcode);		
		emit(as, lobyte(result));
		
		as->cumulative_cycles += 2;
	}
	
	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}


/*!
	@function _sys
	@discussion System call instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _sys(assembler *as, int opcode)
{
	int	result;

	/* system call */
	emit(as, PAGE2);
	emit(as, opcode);
	evaluate(as, &result, &as->line.optr, 0);
	emit(as, lobyte(result));
	print_line(as, 0, ' ', as->old_program_counter);

	return 0;
}


/*!
	@function do_gen
	@discussion General addressing instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
static int do_gen(assembler *as, int opcode, int mode, int always_word)
{
	int	result;

	if (mode == IMMED)
	{
		/* Immediate addressing mode (i.e. #$123) */
		as->line.optr++;
		emit(as, opcode);
		
		/* Evaluate the result. */
		evaluate(as, &result, &as->line.optr, 0);
		
		if (*as->line.optr != EOS)
		{
			error(as, "bad operand");
			
			return 0;
		}
		
		/* If the result is > 255, return error. */
		if ((hibyte(result) != 0x00) && (hibyte(result) != 0xFF))
		{
			error(as, "result >255");

			return 0;
		}
		
		/* Emit the low byte result. */
		emit(as, lobyte(result));

		return 0;
	}
	else
	if (mode == IND)
	{
		/* Indexed mode (i.e. $5,y) */		
		do_indexed(as, opcode + 0x20);

		return 0;
	}
	else
	if (mode == INDIR)
	{
		/* Indirect mode (i.e. [$FFFE] */
		as->line.optr++;

		emit(as, opcode + 0x20);
		emit(as, IPBYTE);

		/* Evaluate. */
		evaluate(as, &result, &as->line.optr, 0);

		/* Emit word. */
		eword(as, result);

		as->cumulative_cycles += 7;

		if (*as->line.optr == ']')
		{
			as->line.optr++;

			return 0;
		}

		error(as, "missing ']'");

		return 0;
	}
	else
	if (mode == OTHER)
	{
		/* Evaluate result */
		evaluate(as, &result, &as->line.optr, 0);
		
		if (as->line.force_byte == 1)
		{
			/* Case #1: < has been prepended to expression */
			
			/* If we are still in pass 1, ignore DP check as there may
			 * be a forward reference
			 */
			if (as->pass > 1 && hibyte(result) != as->DP)
			{
				error(as, "DP out of range");
			}
			else
			{
				emit(as, opcode + 0x10);
				emit(as, lobyte(result));

				as->cumulative_cycles += 2;
			}
			
			return 0;
		}
		else
		if (as->line.force_word == 1)
		{
			/* Case #2: > has been prepended to expression */
			
			/* If we are still in pass 1, ignore DP check as there may
			 * be a forward reference
			 */
			if (as->pass > 1 && hibyte(result) == as->DP)
			{
				as->line.has_warning = 1;
			}
			
			emit(as, opcode + 0x30);
			eword(as, result);

			as->cumulative_cycles += 3;
			
			return 0;
		}
		else
		{
			/* Case #3: Ambiguous... look to as->DP for guidance. */
			if (hibyte(result) == as->DP)
			{
				emit(as, opcode + 0x10);

				emit(as, lobyte(result));
				
				as->cumulative_cycles += 2;
			}
			else
			{
				emit(as, opcode + 0x30);

				eword(as, result);
				
				as->cumulative_cycles += 3;
			}
			
			return 0;
		}
	}
	else
	{
		error(as, "unknown addressing mode");

		return 0;
	}
}


/*!
	@function do_indexed
	@discussion Weird index addressing instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
static int do_indexed(assembler *as, int opcode)
{
	int     pbyte;
	int     j,k;
	int     predec,pstinc;
	int	result, noOffset = 0;

	as->cumulative_cycles += 2;    /* indexed is always 2+ base cycle count */
	predec = 0;
	pstinc = 0;
	pbyte = 128;
	emit(as, opcode);

	if (*as->line.optr == '[')
	{
		pbyte |= 0x10;    /* set indirect bit */
		as->line.optr++;
		if (!any((char)']', as->line.optr))
		{
			error(as, "missing ']'");
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

	if (j == RE && (as->o_cpuclass >= CPU_H6309))
	{
		as->cumulative_cycles++;
		abd_index(as, pbyte + 7);

		return 0;
   	}

	if (j == RF && (as->o_cpuclass >= CPU_H6309))
	{
		as->cumulative_cycles++;
		abd_index(as, pbyte + 10);

		return 0;
	}

	if (j == RW && (as->o_cpuclass >= CPU_H6309))
	{
		as->cumulative_cycles += 4;
		abd_index(as, pbyte + 14);

		return 0;
	}

	/* check if operand's first char is ',' */
	if (*as->line.optr == ',')
	{
		noOffset = 1;
		result = 0;
	}
	else
	{
		evaluate(as, &result, &as->line.optr, 0);
	}
	as->line.optr++;

	while (*as->line.optr == '-')
	{
		predec++;
		as->line.optr++;
	}

	j = regnum(as);

	while (alpha(*as->line.optr))
	{
		as->line.optr++;
	}

	while (*as->line.optr == '+')
	{
		pstinc++;
		as->line.optr++;
	}

	if (j == RPC || j == RPCR)
	{
		if (as->line.force_byte == 0)
		{
			as->line.force_word = 1;
		}
		if (pstinc || predec)
		{
			error(as, "auto inc/dec illegal on PCR");

			return 0;
		}
		
		/* PC or PCR addressing */
		if (as->line.force_word)
		{
			emit(as, pbyte + 13);
			eword(as, result - (as->program_counter + 2));
			as->cumulative_cycles += 5;

			return 0;
		}

		if (as->line.force_byte)
		{
			emit(as, pbyte + 12);

			/* as->line.has_warning */
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
			error(as, "offset must be zero");

			return 0;
		}

		if (predec > 2 || pstinc > 2)
		{
			error(as, "auto inc/dec by 1 or 2 only");

			return 0;
		}

		if ((predec == 1 && (pbyte & 0x10) != 0) ||
			(pstinc == 1 && (pbyte & 0x10) != 0))
		{
			error(as, "no auto inc/dec by 1 for indirect");

			return 0;
		}

		if (predec && pstinc)
		{
			error(as, "can't do both!");

			return 0;
		}

		j = reg_type(as, j);

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
			error(as, "only ,--W and ,W++ allowed for W indexing");

			return 0;
		}

		/* handle ,W++  ,--W */
		if (pbyte & 0x10)  /* [,W++] */
		{
			if (predec == 2)
			{
				emit(as, 0xf0);
				as->cumulative_cycles += 6;

				return 0;
			}
			else if (pstinc == 2)
			{
				emit(as, 0xd0);
				as->cumulative_cycles += 6;

				return 0;
			}
		}
		else		/* ,W++ */
		{
			if (predec == 2)
			{
				emit(as, 0xef);
				as->cumulative_cycles += 2;

				return 0;
			}
			else if (pstinc == 2)
			{
				emit(as, 0xcf);
				as->cumulative_cycles += 2;

				return 0;
			}
		}
	}

	j = reg_type(as, j);

	if (j != 0x100)
	{
		pbyte += j;

		if (as->line.force_word)
	    {
			if ((hibyte(result) == 0))
			{
				as->line.has_warning = 1;
			}
			emit(as, pbyte + 0x09);
			eword(as, result);
			as->cumulative_cycles += 4;

			return 0;
		}

		if (as->line.force_byte)
	    {
			emit(as, pbyte + 0x08);
			if (result <- 128 || result > 127)
			{
				/* it is permissible to specify a larger range, we just
				 * flag it with a warning and downgrade the value
				 */
#if 0
				error(as, "value out of range 2");
				return 0;
#else
				as->line.has_warning = 1;
#endif
			}

			if ((result >= -16) && (result <= 15) && ((pbyte & 16) == 0))
			{
				as->line.has_warning = 1;
			}

			emit(as, lobyte(result));
			as->cumulative_cycles++;

			return 0;
		}

		if (result == 0 && noOffset == 1)
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
	}
	else
	{
		/* ,W  n,W [n,W] */
		if (as->line.force_byte)
		{
			error(as, "byte indexing is invalid for W");

			return 0;
		}

		if (pbyte & 0x10 && (as->o_cpuclass >= CPU_H6309))
		{
			/* [,W] */
			if (as->line.force_word || (result != 0))
			{
				emit(as, 0xb0);
				eword(as, result);
				as->cumulative_cycles += 6;
				return 0;
			}

			emit(as, 0x90);

			return 0;
		}
		else
		{		
			/* ,W */
			if ((as->line.force_word || (result != 0)) && 
				(as->o_cpuclass >= CPU_H6309))
			{
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


/*!
	@function abd_index
	@discussion A, B or D indexed
	@param as The assembler state structure
	@param pbyte Post byte
 */
static int abd_index(assembler *as, int pbyte)
{
	int     k;

	as->line.optr += 2;
	k = regnum(as);
	k = reg_type(as, k);

	if (k == 0x100)
	{
		error(as, "cannot use W for register indirect");

		return 0;
	}

	pbyte += k;
	emit(as, pbyte);

	return 0;
}


/*
 * reg_type: return register type in post-byte format
 */

static int reg_type(assembler *as, int r)
{
	switch (r)
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
		 	if (as->o_cpuclass >= CPU_H6309)
			{
				return(0x100);
			}
	}

	error(as, "illegal register for indexed");
	
	return 0;
}


/*!
	@function addressing_mode
	@discussion Determine addressing mode from operand field
	@param as The assembler state structure
 */
static int addressing_mode(assembler *as)
{
	char *p;

	if (*as->line.operand == '#')
	{
		return(IMMED);          /* immediate addressing */
	}
	
	p = as->line.operand;
	
	while (*p != EOS && *p != BLANK && *p != TAB)
	{
		/* Any , before break? */

		if (*p == ',')
		{
			return(IND);    /* indexed addressing */
		}

		p++;
	}

	if (*as->line.operand == '[')
	{
		return(INDIR);          /* indirect addressing */
	}
	
	return(OTHER);                  /* NOTA */
}


/*!
	@function regnum
	@discussion Return register number of *as->line.optr
	@param as The assembler state structure
 */
static h6309_reg regnum(assembler *as)
{
	if (head(as->line.optr, "D") == 1)
	{
		return(RD);
	}

	if (head(as->line.optr, "X") == 1)
	{
		return(RX);
	}

	if (head(as->line.optr, "Y") == 1)
	{
		return(RY);
	}

	if (head(as->line.optr, "U") == 1)
	{
		return(RU);
	}

	if (head(as->line.optr, "S") == 1)
	{
		return(RS);
	}

	if (head(as->line.optr, "PC") == 1)
	{
		return(RPC);
	}

	if (head(as->line.optr, "PCR") == 1)
	{
		return(RPCR);
	}

	if (head(as->line.optr, "A") == 1)
	{
		return(RA);
	}

	if (head(as->line.optr, "B") == 1)
	{
		return(RB);
	}

	if (head(as->line.optr, "CC") == 1)
	{
		return(RCC);
	}

	if (head(as->line.optr, "DP") == 1)
	{
		return(RDP);
	}

	if (as->o_cpuclass >= CPU_H6309)
	{
		if (head(as->line.optr, "E") == 1)
		{
			return(RE);
		}
		
		if (head(as->line.optr, "F") == 1)
		{
			return(RF);
		}
		
		if (head(as->line.optr, "W") == 1)
		{
			return(RW);
		}
		
		if (head(as->line.optr, "V") == 1)
		{
			return(RV);
		}
		
		if (head(as->line.optr, "0") == 1 ||
			head(as->line.optr, "Z") == 1)
		{
			return(RZERO);
		}
		
		if (head(as->line.optr, "Z") == 1)
		{
			return(RZERO);
		}		

		if ((head(as->line.optr, "T") == 1) && (as->o_cpuclass>=CPU_X9))
		{	
			return(RT);
		}
	}
			
	return(ERR);
}


/*!
	@function _bitgen
	@discussion Bitfield instruction handler
	@param as The assembler state structure
	@param opcode The op-code value to emit
 */
int _bitgen(assembler *as, int opcode)
{
	int r, src, dst, addr;
	
	/* get register number */
	r = regnum(as);

	/* check for legal register */
	if (r != RA && r != RB && r != RCC)
	{
		error(as, "illegal register");
		
		return 1;
	}
	
	/* skip over register name */
	while (alpha(*as->line.optr))
	{
		as->line.optr++;
	}
	
	/* skip over delimiter */
	as->line.optr++;
	
	/* capture source bit */
	evaluate(as, &src, &as->line.optr, 0);
	
	/* skip over delimiter */
	as->line.optr++;
	
	/* capture destination bit */
	evaluate(as, &dst, &as->line.optr, 0);

	/* if src or dst bit > 7, error */
	if (src > 7 || dst > 7)
	{
		error(as, "illegal bit number");
		return 1;
	}
	
	/* skip over delimiter */
	as->line.optr++;
	
	/* capture 8-bit address (DP offsetted) */
	evaluate(as, &addr, &as->line.optr, 0);

	emit(as, PAGE3);
	emit(as, opcode);

	/* emit encoded byte */
	{
		char b;
		
		switch (r)
		{
			case RA:
				b = 1 << 6;
				break;
				
			case RB:
				b = 2 << 6;
				break;
			
			case RCC:
				b = 0 << 6;
				break;
		}
		
		b |= (dst - 1) << 3;
		b |= (src - 1);
		
		emit(as, b);
	}
	
	/* emit direct page address */
	emit(as, addr);
		
	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


