/***************************************************************************
* pseudo.c: pseudo op-code routines
*
* $Id$
*
* The Mamou Assembler - A Hitachi 6309 assembler
*
* (C) 2004 Boisy G. Pitre
***************************************************************************/

#include <string.h>
#include "mamou.h"


/*!
	@function _dts
	@discussion Generate current date/timestamp string
	@param as The assembler state structure
 */
int _dts(assembler *as)
{
	char *t;
	time_t tp;
	
	/* If we are currently in a FALSE conditional, just return. */	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
	
	tp = time(NULL);
	t = ctime(&tp);
	
	while (*t != '\n')
	{
		emit(as, *t);

		t++;
	}

	if (*as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}		
	
	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*!
	@function _dtb
	@discussion Generate current date/timestamp in byte format
	@param as The assembler state structure
 */
int _dtb(assembler *as)
{
	struct tm *t;
	time_t tp;
	
	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
	
	tp = time(NULL);
	t = localtime(&tp);
	
	emit(as, t->tm_year);
	emit(as, t->tm_mon + 1);
	emit(as, t->tm_mday);
	emit(as, t->tm_hour);
	emit(as, t->tm_min);
	emit(as, t->tm_sec);
	
	if (*as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}		
	
	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*****************************************************************************
 *
 * OS-9 ASM DIRECTIVES (MOD, EMOD)
 *
 *****************************************************************************/
#pragma mark OS-9 Asm Directives

/*!
	@function _mod
	@discussion Genereates an OS-9 module header
	@param as The assembler state structure
 */
int _mod(assembler *as)
{
#define HEADER_LEN	9
#define	_MODSYNC		0x87CD
	char *p;
	unsigned char header_check;
	int modinfo[6], i;
	int module_size, name_offset;
	char *operand;
	
	as->old_program_counter = as->program_counter = 0;
	as->data_counter = 0;
	f_record(as);	/* flush out any bytes */
	as->do_module_crc = 1;
	
	as->_crc[0] = 0xFF;
	as->_crc[1] = 0xFF;
	as->_crc[2] = 0xFF;
	
	/* Start OS-9 module */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		/* ignore this pseudo op */
		return 0;
	}
	
	if (as->pass == 1 && *as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}
	
	/* Obtain first parameter -- length of module */
	operand = strdup(as->line.optr);
	
	if ((p = strtok(operand, ",")) == NULL)
	{
		/* Error */
		error(as, "missing parameter");

		free(operand);
		
		return 0;
	}

	evaluate(as, &modinfo[0], &p, 0);

	/* Obtain rest of parameters */
	for (i = 1; i < 6; i++)
	{
		if ((p = strtok(NULL, ",")) == NULL)
		{
			/* Error */
			error(as, "missing parameter");

			free(operand);
		
			return 0;
		}

		evaluate(as, &modinfo[i], &p, 0);
	}	
	
	free(operand);
		
	header_check = 0;
	
	/* Emit sync bytes */
	eword(as, _MODSYNC);
	header_check ^= _MODSYNC >> 8;
	header_check ^= _MODSYNC & 0xFF;
	
	/* Emit module size */
	module_size = modinfo[0];
	eword(as, module_size);
	header_check ^= module_size >> 8;
	header_check ^= module_size & 0xFF;
	
	/* Emit name offset */
	name_offset = modinfo[1];
	eword(as, name_offset);
	header_check ^= name_offset >> 8;
	header_check ^= name_offset & 0xFF;
	
	/* Emit type/language and attribute/revision */
	emit(as, modinfo[2]);
	header_check ^= modinfo[2];
	
	emit(as, modinfo[3]);
	header_check ^= modinfo[3];
	
	/* Emit header check */
	emit(as, ~header_check);
	
	/* Module type specific output */
	switch (modinfo[2] & 0xF0)
	{
		case 0x10:
		case 0x20:
		case 0x30:
		case 0x40:
		case 0x50:
		case 0x60:
		case 0x70:
		case 0x80:
		case 0x90:
		case 0xA0:
		case 0xB0:
		case 0xC0:	/* Systm */
		case 0xD0:	/* FMgr */
		case 0xE0:	/* Drvr */
			/* output exec offset */
			eword(as, modinfo[4]);
			/* output storage size */
			eword(as, modinfo[5]);
			break;
		case 0xF0:	/* Desc */
			/* output fmgr name offset */
			eword(as, modinfo[4]);
			/* output drvr name offset */
			eword(as, modinfo[5]);
			break;
	}
	
	/* Re-adjust PC to undo effects of emit */
	print_line(as, 0, ' ', 0);
	
	return 0;
}


/*!
	@function _emod
	@discussion Generate OS-9 module CRC
	@param as The assembler state structure
 */
int _emod(assembler *as)
{
	/* End OS-9 module (put CRC) */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		/* ignore this pseudo op */
		return 0;
	}	
	
	f_record(as);
	as->do_module_crc = 0;
	as->_crc[0] ^= 0xff;		/* invert the CRC prior to publicising it */
	as->_crc[1] ^= 0xff;
	as->_crc[2] ^= 0xff;
	emit(as, as->_crc[0]);
	emit(as, as->_crc[1]);
	emit(as, as->_crc[2]);
	f_record(as);
	print_line(as, 0, ' ', 0);

	return 0;
}


/*****************************************************************************
 *
 * CONDITIONAL DIRECTIVES (IF, IFP1, IFP2, IFNE, IFEQ, ELSE, ENDC, etc.)
 *
 *****************************************************************************/
#pragma mark Conditional Directives

typedef enum
{
	_IF,
	_IFP1,
	_IFP2,
	_IFNE,
	_IFEQ,
	_IFGT,
	_IFGE,
	_IFLT,
	_IFLE
} conditional;

static int _generic_if(assembler *as, conditional whichone);

/*!
	@function _generic_if
	@discussion Generic conditional handler
	@param as The assembler state structure
	@param whichone The proper conditional
 */
static int _generic_if(assembler *as, conditional whichone)
{
	int	result;
	
	/* First, check to make sure we don't overflow the condition stack. */
	if (as->conditional_stack_index + 1 > CONDSTACKLEN)
	{
		/* Overflow */
		error(as, "conditional stack overflow!");
		
		return 0;
	}	

	/* Next, check the state of the current conditional. */	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		/* Current conditional false, make this one false as well. */
		
		as->conditional_stack[++as->conditional_stack_index] = 0;

		return 0;
	}
	
	/* Previous conditional true, evaluate this one */		
	if (whichone != _IFP1 && whichone != _IFP2)
	{
		evaluate(as, &result, &as->line.optr, 1);
	}

	if (as->o_show_cond == 1)
	{
		print_line(as, 0, ' ', 0);
	}

	as->conditional_stack_index++;

	switch (whichone)
	{
		case _IFEQ:
			as->conditional_stack[as->conditional_stack_index] = (result == 0);
			break;
			
		case _IFP1:
			as->conditional_stack[as->conditional_stack_index] = (as->pass == 1);
			break;

		case _IFP2:
			as->conditional_stack[as->conditional_stack_index] = (as->pass == 2);
			break;
			
		case _IF:
		case _IFNE:
			as->conditional_stack[as->conditional_stack_index] = (result != 0);
			break;
			
		case _IFGT:
			as->conditional_stack[as->conditional_stack_index] = (result > 0);
			break;
			
		case _IFGE:
			as->conditional_stack[as->conditional_stack_index] = (result >= 0);
			break;
			
		case _IFLT:
			as->conditional_stack[as->conditional_stack_index] = (result < 0);
			break;
			
		case _IFLE:
			as->conditional_stack[as->conditional_stack_index] = (result <= 0);
			break;
	}	
	
	return 0;
}


/*!
	@function _ifp1
	@discussion IF PASS = 1 conditional
	@param as The assembler state structure
 */
int _ifp1(assembler *as)
{
	return _generic_if(as, _IFP1);
}


/*!
	@function _ifp1
	@discussion IF PASS = 2 conditional
	@param as The assembler state structure
 */
int _ifp2(assembler *as)
{
	return _generic_if(as, _IFP2);
}


/*!
	@function _ifeq
	@discussion IF result = 0 conditional
	@param as The assembler state structure
 */
int _ifeq(assembler *as)
{
	return _generic_if(as, _IFEQ);
}


/*!
	@function _ifne
	@discussion IF result != 0 conditional
	@param as The assembler state structure
 */
int _ifne(assembler *as)
{
	return _generic_if(as, _IFNE);
}


/*!
	@function _iflt
	@discussion IF result < 0 conditional
	@param as The assembler state structure
 */
int _iflt(assembler *as)
{
	return _generic_if(as, _IFLT);
}


/*!
	@function _ifle
	@discussion IF result <= 0 conditional
	@param as The assembler state structure
 */
int _ifle(assembler *as)
{
	return _generic_if(as, _IFLE);
}


/*!
	@function _ifgt
	@discussion IF result > 0 conditional
	@param as The assembler state structure
 */
int _ifgt(assembler *as)
{
	return _generic_if(as, _IFGT);
}


/*!
	@function _ifge
	@discussion IF result >= 0 conditional
	@param as The assembler state structure
 */
int _ifge(assembler *as)
{
	return _generic_if(as, _IFGE);
}


/*!
	@function _endc
	@discussion End conditional
	@param as The assembler state structure
 */
int _endc(assembler *as)
{
	if (as->conditional_stack_index == 0)
	{
		/* Conditional underflow */
		error(as, "ENDC without a conditional IF!");

		return 0;
	}

	as->conditional_stack_index--;

	if (as->o_show_cond == 1)
	{
		print_line(as, 0, ' ', 0);
	}
	
	return 0;
}


/*!
	@function _else
	@discussion Change current conditional
	@param as The assembler state structure
 */
int _else(assembler *as)
{
	/* If the previous conditional was false... */
	if (as->conditional_stack_index > 0 && as->conditional_stack[as->conditional_stack_index - 1] == 0)
	{
		/* Then ignore this one */
		return 0;
	}
	
	if (as->o_show_cond == 1)
	{
		print_line(as, 0, ' ', 0);
	}

	/* Invert the sense of the conditional */
	as->conditional_stack[as->conditional_stack_index] = !as->conditional_stack[as->conditional_stack_index];

	if (as->o_show_cond == 1)
	{
		print_line(as, 0, ' ', 0);
	}

	return 0;
}


/*****************************************************************************
 *
 * ALIGNMENT DIRECTIVES
 *
 *****************************************************************************/
#pragma mark Alignment Directives

/*!
	@function _align
	@discussion Align program counter on passed boundary
	@param as The assembler state structure
 */
int _align(assembler *as)
{
	int	result;
	
	as->P_force = 1;
	
	as->code_segment_start = 1;
	
	/* 1. If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
	
	if (evaluate(as, &result, &as->line.optr, 0))
	{
		int			whats_left;
		
		f_record(as);     /* flush out bytes */
		
		whats_left = result - (as->program_counter % result);

		while (whats_left-- > 0)
		{
			emit(as, 0);
		}

		print_line(as, 0, ' ', as->old_program_counter);
	}
	else
	{
		error(as, "undefined operand during pass one");
	}
	
	return 0;
}


/*!
	@function _even
	@discussion Align PC on even boundary
	@param as The assembler state structure
 */
int _even(assembler *as)
{
	as->P_force = 1;
	
	as->code_segment_start = 1;
	
	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
		
	f_record(as);     /* flush out bytes */
		
	if (as->program_counter % 2 == 1)
	{
		emit(as, 0);
	}

	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*!
	@function _odd
	@discussion Align PC on odd boundary
	@param as The assembler state structure
 */
int _odd(assembler *as)
{
	as->P_force = 1;
	
	as->code_segment_start = 1;	
	
	/* If we are currently in a FALSE conditional, just return. */	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}	
	
	f_record(as);     /* flush out bytes */
	
	if (as->program_counter % 2 == 0)
	{
		emit(as, 0);
	}

	print_line(as, 0, ' ', as->old_program_counter);	
	
	return 0;
}


/*****************************************************************************
*
* PAGING DIRECTIVES (NAM, TTL, PAGE, etc.)
*
*****************************************************************************/
#pragma mark Paging Directives

/*!
	@function nam
	@discussion Specify a name for header printing
	@param as The assembler state structure
 */ 
int _nam(assembler *as)
{
	/* If we are currently in a FALSE conditional, just return. */	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	/* Ignore this directive in any file but the root (main) file. */	
	if (as->use_depth > 0)
	{
		return 0;
	}

	/* Test for presence of a label and error if found. */	
	if (*as->line.label != EOS)
	{
		error(as, "label not allowed");

		return 0;
	}
		
	/* Copy the name into our global space for later use. */	
	strncpy((char *)as->name_header, as->line.optr, NAMLEN-1);

	print_line(as, 0, ' ', 0);

	return 0;
}


/*!
	@function _ttl
	@discussion Specify a title for printing
	@param as The assembler state structure
 */ 
int _ttl(assembler *as)
{
	/* If we are currently in a FALSE conditional, just return. */	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	/* Ignore this directive in any file but the root (main) file. */
	if (as->use_depth > 0)
	{
		return 0;
	}

	/* Test for presence of a label and error if found */	
	if (*as->line.label != EOS)
	{
		error(as, "label not allowed");

		return 0;
	}
	
	/* Copy the title into our global space for later use. */
	strncpy((char *)as->title_header, as->line.optr, TTLLEN - 1);
	
	print_line(as, 0, ' ', 0);

	return 0;
}


/*!
	@function _page
	@discussion New page
	@param as The assembler state structure
 */
int _page(assembler *as)
{
	as->P_force = 0;
	as->f_new_page = 1;
	
	if (as->pass == 2)
	{
		if (as->o_show_listing == 1)  
		{
			if (as->o_format_only == 1)
			{
				printf("* ");
			}
			else
			{
				printf("\f");
			}
	
			printf("%-10s", extractfilename(as->file_name[as->file_index -1]));
			printf("                                   ");
			printf("page %3u\n", (unsigned int)as->page_number++);
		}
	}

	print_line(as, 0, ' ', 0);
	
	return 0;
}


/*!
	@function _fill
	@discussion Fill memory bytes
	@param as The assembler state structure
 */
int _fill(assembler *as)
{
	int		fill;
	int		result;

	as->P_force = 1;

	/* If we are currently in a FALSE conditional, just return. */	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	evaluate(as, &result, &as->line.optr, 0);

	fill = result;

	if (*as->line.optr++ != ',')
	{
		error(as, "bad fill");
	}
	else
	{
		as->line.optr = skip_white(as->line.optr);

		evaluate(as, &result, &as->line.optr, 0);

		if (result < 0)
		{
			error(as, "illegal value for fill");

			return 0;
		}

		while (result--)
		{
			emit(as, fill);

			/* In order to not overflow our emit buffer, we flush
			 * every MAXBUF bytes.
			 */
			if (result % MAXBUF == 0)
			{
				f_record(as);     /* flush out bytes so far */
			}
		}

		print_line(as, 0, ' ', as->old_program_counter);
	}

	if (*as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}		
	
	return 0;
}


/*!
	@function _fcc
	@discussion Fill constant characters
	@param as The assembler state structure
 */
int _fcc(assembler *as)
{
	char fccdelim;

	/* If we are currently in a FALSE conditional, just return. */	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	if (*as->line.operand == EOS)
	{
		return 0;
	}
	
	fccdelim = *as->line.optr++;

	while (*as->line.optr != EOS && *as->line.optr != fccdelim)
	{
		emit(as, *as->line.optr++);
	}

	if (*as->line.optr == fccdelim)
	{
		as->line.optr++;
	}
	else
	{
		error(as, "missing delimiter");

		return 0;
	}

	if (*as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}		

	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*!
	@function _fcz
	@discussion Fill constant characters with a nul byte at end
	@param as The assembler state structure
 */
int _fcz(assembler *as)
{
	char fccdelim;
	
	/* If we are currently in a FALSE conditional, just return. */	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
	
	if (*as->line.operand == EOS)
	{
		return 0;
	}
	
	fccdelim = *as->line.optr++;
	
	while (*as->line.optr != EOS && *as->line.optr != fccdelim)
	{
		emit(as, *as->line.optr++);
	}
	
	emit(as, EOS);
	
	if (*as->line.optr == fccdelim)
	{
		as->line.optr++;
	}
	else
	{
		error(as, "missing delimiter");

		return 0;
	}
	
	if (*as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}		
	
	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*!
	@function _fcs
	@discussion Fill constant string
	@param as The assembler state structure
 */
int _fcs(assembler *as)
{
	char fccdelim;

	/* If we are currently in a FALSE conditional, just return. */

	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	if (*as->line.operand == EOS)
	{
		return 0;
	}

	/* Get delimiter character. */
	fccdelim = *as->line.optr++;

	while (*as->line.optr != EOS && *as->line.optr != fccdelim)
	{
		/* Look ahead to the next char. */
		if (*(as->line.optr + 1) != fccdelim)
		{
			emit(as, *as->line.optr++);
		}
		else
		{
			/* Next char is fccdelim, so set hi bit of last char. */
			emit(as, *as->line.optr + 128);
			as->line.optr++;
		}
	}

	if (*as->line.optr == fccdelim)
	{
		as->line.optr++;
	}
	else
	{
		error(as, "missing delimiter");

		return 0;
	}

	if (*as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}		

	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}



/*!
	@function _fcr
	@discussion Fill constant string with CR and nul at end
	@param as The assembler state structure
 */
int _fcr(assembler *as)
{
	char fccdelim;
	
	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
	
	if (*as->line.operand == EOS)
	{
		return 0;
	}
	
	/* Get delimiter character. */
	fccdelim = *as->line.optr++;
	
	while (*as->line.optr != EOS && *as->line.optr != fccdelim)
	{
		emit(as, *as->line.optr++);
	}
	
	if (*as->line.optr == fccdelim)
	{
		emit(as, 0x0D);
		emit(as, 0x00);

		as->line.optr++;
	}
	else
	{
		error(as, "missing delimiter");
		
		return 0;
	}
	
	if (*as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}		
	
	print_line(as, 0, ' ', as->old_program_counter);
	
	return 0;
}


/*!
	@function _org
	@discussion Set origin for PC (Disk BASIC mode) or DATA (OS-9 mode)
	@param as The assembler state structure
 */
int _org(assembler *as)
{
	int	result;
	char		emit_char = ' ' ;
	
	as->P_force = 1;

	as->code_segment_start = 1;
		
	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	if (evaluate(as, &result, &as->line.optr, 0) == 1)
	{
		if ((as->o_asm_mode == ASM_DECB) || (as->o_asm_mode == ASM_ROM))
		{
			/* Disk BASIC BIN file or ROM -- set program counter to result. */
			as->program_counter = result;
		}
		else
		{
			/* OS-9 mode... our emit character is 'D'. */
			emit_char = 'D';
		}

		as->data_counter = result;
		
		f_record(as);     /* flush out any bytes */
	}
	else
	{
		error(as, "undefined operand during pass one");

		return 0;
	}

	print_line(as, 0, emit_char, as->data_counter);
	
	return 0;
}


/*!
	@function _equ
	@discussion Equate a label to an expression
	@param as The assembler state structure
 */
int _equ(assembler *as)
{
	int	result;

	as->P_force = 1;

	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	if (*as->line.label == EOS)
	{
		error(as, "label required");

		return 0;
	}

	if (evaluate(as, &result, &as->line.optr, 0))
	{
		symbol_add(as, as->line.label, result, 0);
		as->old_program_counter = result;        /* override normal */
	}
	else
	{
		error(as, "undefined operand during pass one");

		return 0;
	}

	print_line(as, 0, ' ', result);
	
	return 0;
}


/*!
	@function _set
	@discussion Set label to an expression
	@param as The assembler state structure
 */
int _set(assembler *as)
{
	int	result;

	as->P_force = 1;

	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	if (*as->line.label == EOS)
	{
		error(as, "label required");

		return 0;
	}

	if (evaluate(as, &result, &as->line.optr, 0))
	{
		symbol_add(as, as->line.label, result, 1);
		as->old_program_counter = result;        /* override normal */
	}

	print_line(as, 0, ' ', result);
	
	return 0;
}


/*!
	@function _opt
	@discussion Set assembler options
	@param as The assembler state structure
 */
int _opt(assembler *as)
{
	char *Opt = as->line.operand;
	int opt_state = 1;

	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	/* Test for presence of a label and error if found. */
	if (*as->line.label != EOS)
	{
		error(as, "label not allowed");

		return 0;
	}

	if (as->use_depth > 0)
	{
		/* OPTs are ignored in files brought in by
		 * the 'use' directive, but are still printed.
		 */
		print_line(as, 0, ' ', 0);

		return 0;
	}

	as->P_force = 0;

	/* Does a minus precede the option? */
	if (*Opt == '-')
	{
		Opt++;
		opt_state = 0;
	}

	/* Parse the option */
	switch (tolower(*Opt))
	{
		case 'b':	/* Disk BASIC compatible mode */
			as->o_asm_mode = opt_state;
			break;
						
		case 'c':	/* conditional assembly in listing */
			as->o_show_cond = opt_state;
			break;

		case 'd':	/* page depth */
			as->o_page_depth = atoi(Opt + 1);
			break;

		case 'e':	/* error messages */
			as->o_show_error = opt_state;
			break;

		case 'f':	/* use form feed */
			as->Opt_F = opt_state;
			break;

		case 'g':	/* generate all constant lines */
			as->Opt_G = opt_state;
			break;

		case 'l':	/* listing */
			as->o_show_listing = opt_state;
			break;

		case 'n':	/* narrow listing */
			as->Opt_N = opt_state;
			break;

		case 'o':	/* object file name */
			if (opt_state == 0)
			{
				strncpy(as->object_name, as->line.optr + 1, FNAMESIZE - 1);
			}
			else
			{
				as->object_name[0] = EOS;
			}
			break;

		case 's':	/* generate symbol table */
			as->o_show_symbol_table = opt_state;
			break;

		case 'w':	/* page width */
			as->o_pagewidth = atoi(Opt + 1);
			break;

		default:
			error(as, "opt list");
			break;
	}

	print_line(as, 0, ' ', 0);

	return 0;
}


/*!
	@function _null_op
	@discussion Unsupported pseudo-op
	@param as The assembler state structure
 */
int _null_op(assembler *as)
{
	as->P_force = 0;
	
	return 0;
}


/*!
	@function _setdp
	@discussion Set direct page
	@param as The assembler state structure
 */
int _setdp(assembler *as)   /* TODO! */
{
	int	newdp;

	as->P_force = 1;

	evaluate(as, &newdp, &as->line.optr, 0);
	as->DP = newdp;

	as->data_counter = newdp;
	f_record(as);
	print_line(as, 0, 'D', as->data_counter);

	return 0;
}


/*!
	@function _spc
	@discussion Add spaces to printed output
	@param as The assembler state structure
 */
int _spc(assembler *as)	/* TODO! */
{
	int lines = atoi(as->line.operand);

	if (lines == 0)
	{
		/* put out one line */
	}
	else
	{
		/* put out 'lines' lines */
	}

	return 0;
}


/*!
	@function use
	@discussion Include other assembly or definition files
	@param as The assembler state structure
 */
int _use(assembler *as)
{
	error_code ec = 0;

	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	/* Test for presence of a label and return error if found. */
	if (*as->line.label != EOS)
	{
		error(as, "label not allowed");
	}

	
	/* Print the line. */
	print_line(as, 0, ' ', 0);

	{
		struct filestack use_file, *prev_file;
		char		path[FNAMESIZE];
		u_int		i = 0;
		
		/* Set up the structure. */
		prev_file = as->current_file;
		
		as->current_file = &use_file;
		
		strncpy(use_file.file, as->line.optr, FNAMESIZE);

		use_file.current_line = 0;
		use_file.num_blank_lines = 0;
		use_file.num_comment_lines = 0;
		use_file.end_encountered = 0;
		
		/* Open a path to the file. */
		strncpy(path, use_file.file, FNAMESIZE);

		do
		{
			ec = _coco_open(&(use_file.fd), path, FAM_READ);

			if (ec != 0 && i < as->include_index)
			{
				/* Try any alternate include directories. */
				strcpy(path, as->includes[i]);
				strcat(path, "/");
				strcat(path, use_file.file);				
			}
		} while (ec != 0 && i++ < as->include_index);
		
		if (ec == 0)
		{
			/* Make the first pass. */
			as->use_depth++;
			
			mamou_pass(as);
			
			/* Close the file. */
			_coco_close(use_file.fd);

			as->use_depth--;
		}
		else
		{
			printf("mamou: can't open %s\n", use_file.file);
		}	

		as->current_file = prev_file;			
	}
	
	return 0;
}


/*!
	@function __end
	@discussion Stop further processing in this pass
	@param as The assembler state structure
 */
int __end(assembler *as)
{
	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}

	/* Test for presence of a label and error if found. */
	if (*as->line.label != EOS)
	{
		error(as, "label not allowed");

		return 0;
	}
	else
	{
		/* If we are in pass 2 and this is DECB mode, evaluate the operand,
		 * if any, for the EXEC address.
		 */
		if (as->pass == 2 && as->o_asm_mode == ASM_DECB && *as->line.optr != EOS)
		{
			evaluate(as, (int *)&as->decb_exec_address, &as->line.optr, 0);
		}
		
		print_line(as, 0, ' ', 0);

		as->current_file->end_encountered = 1;
	}

	return 0;
}



/*****************************************************************************
*
* RESERVE MEMORY STORAGE (RMB, etc.)
*
*****************************************************************************/
#pragma mark Reserve Memory Storage

static int _reserve_memory(assembler *as, int size);


/*!
	@function _reserve_memory
	@discussion Reserve memory
	@param as The assembler state structure
 */
static int _reserve_memory(assembler *as, int size)
{
	int	result;
	
	as->P_force = 1;
	
	as->code_segment_start = 1;
	
	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
	
	if (evaluate(as, &result, &as->line.optr, 0))
	{
		f_record(as);     /* flush out bytes */
		
		if (as->o_asm_mode == ASM_OS9)
		{
			if (*as->line.label != EOS)
			{
				symbol_add(as, as->line.label, as->data_counter, 0);
			}
			
			print_line(as, 0, 'D', as->data_counter);

			as->data_counter +=  result * size;
		}
		else
		{
			if (*as->line.label != EOS)
			{
				symbol_add(as, as->line.label, as->program_counter, 0);
			}
			
			print_line(as, 0, ' ', as->program_counter);

			as->program_counter +=  result * size;
		}
	}
	else
	{
		error(as, "undefined operand during pass one");
	}
	
	return 0;
}


/*!
	@function _rmb
	@discussion Reserve memory bytes
	@param as The assembler state structure
 */
int _rmb(assembler *as)
{
	return _reserve_memory(as, 1);
}


/*!
	@function _rmd
	@discussion Reserve memory double bytes
	@param as The assembler state structure
 */
int _rmd(assembler *as)
{
	return _reserve_memory(as, 2);
}


/*!
	@function _rmq
	@discussion Reserve memory quad bytes
	@param as The assembler state structure
 */
int _rmq(assembler *as)
{
	return _reserve_memory(as, 4);
}


/*****************************************************************************
 *
 * FILL CONSTANT DATA
 *
 *****************************************************************************/
#pragma mark Fill Constant Data

static int _fill_constant(assembler *as, int size);


/*!
	@function _fill_constant
	@discussion Fill constant data
	@param as The assembler state structure
 */
int _fill_constant(assembler *as, int size)
{
	int	result;
	
	/* If we are currently in a FALSE conditional, just return. */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
	
	as->line.optr = skip_white(as->line.optr);
	
	do
	{
		evaluate(as, &result, &as->line.optr, 0);
		
		switch (size)
		{
			case 1:
				if (result > 0xFF && as->line.force_byte == 0)
				{
					error(as, "value truncated");
				}
				result = lobyte(result);
				emit(as, result);
				break;
				
			case 2:
				if (result > 0xFFFF && as->line.force_byte == 0)
				{
					error(as, "value truncated");
				}
				eword(as, result);
				break;
				
			case 4:
				equad(as, result);
				break;
		}
	}
	while (*as->line.optr++ == ',');
		
	if (*as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}		
		
	print_line(as, 0, ' ', as->old_program_counter);
		
	return 0;
}


/*!
	@function _fcb
	@discussion Fill constant bytes
	@param as The assembler state structure
 */
int _fcb(assembler *as)
{
	return _fill_constant(as, 1);
}


/*!
	@function _fdb
	@discussion Fill double bytes
	@param as The assembler state structure
 */
int _fdb(assembler *as)
{
	return _fill_constant(as, 2);
}


/*!
	@function _fqb
	@discussion Fill quad bytes
	@param as The assembler state structure
 */
int _fqb(assembler *as)
{
	return _fill_constant(as, 4);
}


/*****************************************************************************
 *
 * FILL CONSTANT DATA WITH VALUE
 *
 *****************************************************************************/
#pragma mark Fill Constant Data With Value

static int _fill_constant_with_value(assembler *as, int size, int value);


/*!
	@function fill_constant_with_value
	@discussion Fill constant space with value
	@param as The assembler state structure
 */
static int _fill_constant_with_value(assembler *as, int size, int value)
{
	int	result;
	
	as->P_force = 1;
	
	/* If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
	
	if (evaluate(as, &result, &as->line.optr, 0))
	{
		if (result < 0)
		{
			error(as, "illegal value");

			return 0;
		}
		
		while (result--)
		{
			switch (size)
			{
				case 1:
					emit(as, value);
					break;

				case 2:
					eword(as, value);
					break;
					
				case 4:
					equad(as, value);
					break;
			}
					
			/* In order to not overflow our emit buffer, we flush
			 * every MAXBUF bytes.
			 */
			if (result % MAXBUF == 0)
			{
				f_record(as);     /* flush out bytes so far */
			}
		}

		print_line(as, 0, ' ', as->old_program_counter);
	}
	else
	{
		error(as, "undefined operand during pass one");
	}

	if (*as->line.label != EOS)
	{
		symbol_add(as, as->line.label, as->old_program_counter, 0);
	}		
	
	return 0;
}


/*!
	@function _zmb
	@discussion Zero memory bytes
	@param as The assembler state structure
 */
int _zmb(assembler *as)
{
	return _fill_constant_with_value(as, 1, 0);
}


/*!
	@function _zmd
	@discussion Zero memory double bytes
	@param as The assembler state structure
 */
int _zmd(assembler *as)
{
	return _fill_constant_with_value(as, 2, 0);
}


/*!
	@function _zmq
	@discussion Zero memory quad bytes
	@param as The assembler state structure
 */
int _zmq(assembler *as)
{
	return _fill_constant_with_value(as, 4, 0);
}
