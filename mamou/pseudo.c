#include <string.h>


#include "mamou.h"


/*
 * nam - specify a name for header printing
 */
 
int _nam(assembler *as)
{
	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	/* 2. Ignore this directive in any file but the root (main) file. */
	
	if (as->use_depth > 0)
	{
		return 0;
	}


	/* 3. Test for presence of a label and error if found. */
	
	if (*as->line->label != EOS)
	{
		error(as, "label not allowed");

		return 0;
	}
	
	
	/* 4. Copy the name into our global space for later use. */
	
	strncpy(as->name_header, as->line->optr, NAMLEN-1);

	print_line(as, 0, ' ', 0);


	return 0;
}



/*
 * ttl - specify a title for printing
 */
 
int _ttl(assembler *as)
{
	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	/* 2. Ignore this directive in any file but the root (main) file. */

	if (as->use_depth > 0)
	{
		return 0;
	}


	/* 3. Test for presence of a label and error if found */
	
	if (*as->line->label != EOS)
	{
		error(as, "label not allowed");

		return 0;
	}

	
	/* 4. Copy the title into our global space for later use. */
	
	strncpy(as->title_header, as->line->optr, TTLLEN - 1);

	print_line(as, 0, ' ', 0);


	return 0;
}


/*
 * rmb - reserve memory bytes
 */
int _rmb(assembler *as)
{
	BP_int32	result;

	
	as->P_force = 1;
	
	as->rm_encountered = BP_TRUE;


	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	if (evaluate(as, &result, &as->line->optr, 0))
	{
		f_record(as);     /* flush out bytes */

		if (as->o_asm_mode == ASM_OS9)
		{
			if (*as->line->label != EOS)
			{
				symbol_add(as, as->line->label, as->data_counter, 0);
			}
			
			print_line(as, 0, 'D', as->data_counter);
			as->data_counter +=  result;
		}
		else
		{
			if (*as->line->label != EOS)
			{
				symbol_add(as, as->line->label, as->program_counter, 0);
			}
			
			print_line(as, 0, ' ', as->program_counter);
			as->program_counter +=  result;
		}
	}
	else
	{
		error(as, "Undefined as->line->operand during Pass One");
	}
	return 0;
}


/*
 * zmb - zero memory bytes
 */
int _zmb(assembler *as)
{
	BP_int32	result;

	as->P_force = 1;

	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	if (evaluate(as, &result, &as->line->optr, 0))
	{
		if (result < 0)
		{
			error(as, "Illegal value for zmb");
			return 0;
		}

		while (result--)
		{
			emit(as, 0);

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
		error(as, "Undefined as->line->operand during Pass One");
	}
	if (*as->line->label != EOS)
	{
		symbol_add(as, as->line->label, as->old_program_counter, 0);
	}		
	return 0;
}



/*
 * fill - fill memory bytes
 */
int _fill(assembler *as)
{
	BP_int32		fill;
	BP_int32		result;

	
	as->P_force = 1;

	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	evaluate(as, &result, &as->line->optr, 0);

	fill = result;

	if (*as->line->optr++ != ',')
	{
		error(as, "Bad fill");
	}
	else
	{
		as->line->optr = skip_white(as->line->optr);

		evaluate(as, &result, &as->line->optr, 0);

		if (result < 0)
		{
			error(as, "Illegal value for fill");

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

	if (*as->line->label != EOS)
	{
		symbol_add(as, as->line->label, as->old_program_counter, BP_FALSE);
	}		

	
	return 0;;
}



/*
 * fcb - form constant bytes
 */

int _fcb(assembler *as)
{
	BP_int32	result;


	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	do
	{
		as->line->optr = skip_white(as->line->optr);

		evaluate(as, &result, &as->line->optr, 0);

		if (result > 0xFF)
		{
			if (as->line->force_byte == BP_FALSE)
			{
				error(as, "Value truncated");
			}

			result = lobyte(result);
		}

		emit(as, result);
	}

	while (*as->line->optr++ == ',');

	if (*as->line->label != EOS)
	{
		symbol_add(as, as->line->label, as->old_program_counter, BP_FALSE);
	}		

	print_line(as, 0, ' ', as->old_program_counter);

	
	return 0;
}



/*
 * fdb - form double bytes
 */
int _fdb(assembler *as)
{
	BP_int32	result;


	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	do
	{
		as->line->optr = skip_white(as->line->optr);
		evaluate(as, &result, &as->line->optr,0 );
		eword(as, result);
	} while (*as->line->optr++ == ',');

	if (*as->line->label != EOS)
	{
		symbol_add(as, as->line->label, as->old_program_counter, 0);
	}		

	print_line(as, 0, ' ', as->old_program_counter);

	
	return 0;
}



/*
 * fcc - form constant characters
 */
int _fcc(assembler *as)
{
	char fccdelim;

	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	if (*as->line->operand == EOS)
	{
		return 0;
	}
	
	fccdelim = *as->line->optr++;

	while (*as->line->optr != EOS && *as->line->optr != fccdelim)
	{
		emit(as, *as->line->optr++);
	}

	if (*as->line->optr == fccdelim)
	{
		as->line->optr++;
	}
	else
	{
		error(as, "Missing Delimiter");
		return 0;
	}

	if (*as->line->label != EOS)
	{
		symbol_add(as, as->line->label, as->old_program_counter, 0);
	}		

	print_line(as, 0, ' ', as->old_program_counter);

	
	return 0;
}



/*
 * fcz - form constant characters with a nul byte at end
 */
int _fcz(assembler *as)
{
	char fccdelim;
	
	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}
	
	
	if (*as->line->operand == EOS)
	{
		return 0;
	}
	
	fccdelim = *as->line->optr++;
	
	while (*as->line->optr != EOS && *as->line->optr != fccdelim)
	{
		emit(as, *as->line->optr++);
	}
	
	emit(as, EOS);
	
	if (*as->line->optr == fccdelim)
	{
		as->line->optr++;
	}
	else
	{
		error(as, "Missing Delimiter");
		return 0;
	}
	
	if (*as->line->label != EOS)
	{
		symbol_add(as, as->line->label, as->old_program_counter, 0);
	}		
	
	print_line(as, 0, ' ', as->old_program_counter);
	
	
	return 0;
}



/*
 * fcs - form constant string
 */

int _fcs(assembler *as)
{
	char fccdelim;

	
	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == BP_FALSE)
	{
		return 0;
	}


	if (*as->line->operand == EOS)
	{
		return 0;
	}


	/* Get delimiter character. */
	
	fccdelim = *as->line->optr++;

	while (*as->line->optr != EOS && *as->line->optr != fccdelim)
	{
		/* Look ahead to the next char. */
		
		if (*(as->line->optr + 1) != fccdelim)
		{
			emit(as, *as->line->optr++);
		}
		else
		{
			/* Next char is fccdelim, so set hi bit of last char. */
			
			emit(as, *as->line->optr + 128);
			as->line->optr++;
		}
	}

	if (*as->line->optr == fccdelim)
	{
		as->line->optr++;
	}
	else
	{
		error(as, "Missing Delimiter");
		return 0;
	}
	if (*as->line->label != EOS)
	{
		symbol_add(as, as->line->label, as->old_program_counter, 0);
	}		
	print_line(as, 0, ' ', as->old_program_counter);
	return 0;
}



/*
 * org: set origin for PC (Disk BASIC mode) or DATA (OS-9 mode)
 */

int _org(assembler *as)
{
	BP_int32	result;
	BP_char		emit_char = ' ' ;
	

	as->P_force = 1;

	as->rm_encountered = BP_TRUE;
	
		
	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	if (evaluate(as, &result, &as->line->optr, 0) == BP_TRUE)
	{
		if (as->o_asm_mode == ASM_DECB)
		{
			/* 1. Disk BASIC BIN file -- set program counter to result. */
			
			as->program_counter = result;
		}
		else
		{
			/* 1. OS-9 mode... our emit character is 'D'. */
			
			emit_char = 'D';
		}

		as->data_counter = result;
		
		f_record(as);     /* flush out any bytes */
	}
	else
	{
		error(as, "Undefined operand during Pass One");
		return 0;
	}

	print_line(as, 0, emit_char, as->data_counter);
	
	
	return 0;
}


/*
 * equ - equate label to an expression
 */
int _equ(assembler *as)
{
	BP_int32	result;


	as->P_force = 1;

	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	if (*as->line->label == EOS)
	{
		error(as, "EQU requires label");
		return 0;
	}
	if (evaluate(as, &result, &as->line->optr, 0))
	{
		symbol_add(as, as->line->label, result, 0);
		as->old_program_counter = result;        /* override normal */
	}
	else
	{
		error(as, "Undefined as->line->operand during Pass One");
		return 0;
	}
	print_line(as, 0, ' ', result);
	return 0;
}


/*
 * set - set label to an expression
 */
int _set(assembler *as)
{
	BP_int32	result;


	as->P_force = 1;

	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	if (*as->line->label == EOS)
	{
		error(as, "SET requires label");
		return 0;
	}
	if (evaluate(as, &result, &as->line->optr, 0))
	{
		symbol_add(as, as->line->label, result, 1);
		as->old_program_counter = result;        /* override normal */
	}
	print_line(as, 0, ' ', result);
	return 0;
}


/*
 * opt - set assembler options
 */
int _opt(assembler *as)
{
	char *Opt = as->line->operand;
	BP_Bool opt_state = BP_TRUE;
	

	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	/* test for presence of a label and error if found */
	if (*as->line->label != EOS)
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

	/* does a minus precede the option? */
	if (*Opt == '-')
	{
		Opt++;
		opt_state = BP_FALSE;
	}

	/* parse the option */
	switch (tolower(*Opt))
	{
		case 'b':	/* Disk BASIC compatible mode */
			as->o_asm_mode = opt_state;
			break;
						
		case 'c':	/* conditional assembly in listing */
			as->Opt_C = opt_state;
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
			if (opt_state == BP_FALSE)
			{
				strncpy(as->object_name, as->line->optr + 1, FNAMESIZE - 1);
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


/*
 * page - new page
 */
int _page(assembler *as)
{
	as->P_force = 0;
	as->f_new_page = BP_TRUE;
	
	if (as->pass == 2)
	{
		if (as->o_show_listing == BP_TRUE)  
		{
			if (as->o_format_only == BP_TRUE)
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


/*
 * unsupported pseudo op
 */
int _null_op(assembler *as)
{
	as->P_force = 0;
	return 0;
}


/*
 * ifp1 - if pass == 1 conditional
 */
int _ifp1(assembler *as)
{
	BP_int32	result;


	if (as->Opt_C == BP_TRUE)
	{
		print_line(as, 0, ' ', 0);
	}
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		/* prev. conditional false, ignore this one */
		result = 0;
	}
	else
	{
		/* prev. conditional true, evaluate this one */
		if (as->pass == 1)
		{
			result = 1;
		}
		else
		{
			result = 0;
		}
	}
	as->conditional_stack_index++;
	if (as->conditional_stack_index > CONDSTACKLEN)
	{
		/* overflow */
		error(as, "Conditional stack overflow!");
		return 0;
	}
	/* determine if current pass is ok */
	as->conditional_stack[as->conditional_stack_index] = result;
	return 0;
}


/*
 * ifp2 - if pass == 2 conditional
 */
int _ifp2(assembler *as)
{
	BP_int32	result;


	if (as->Opt_C == BP_TRUE)
	{
		print_line(as, 0, ' ', 0);
	}
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		/* prev. conditional false, ignore this one */
		result = 0;
	}
	else
	{
		/* prev. conditional true, evaluate this one */
		if (as->pass == 2)
		{
			result = 1;
		}
		else
		{
			result = 0;
		}
	}
	as->conditional_stack_index++;
	if (as->conditional_stack_index > CONDSTACKLEN)
	{
		/* overflow */
		error(as, "Conditional stack overflow!");
		return 0;
	}
	/* determine if current pass is ok */
	as->conditional_stack[as->conditional_stack_index] = result;
	return 0;
}


/*
 * ifeq - if expression == 0 conditional
 */
int _ifeq(assembler *as)
{
	BP_int32	result;


	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		/* prev. conditional false, ignore this one */
		result = 1;
	}
	else
	{
		/* prev. conditional true, evaluate this one */
		evaluate(as, &result, &as->line->optr, 1);
		if (as->Opt_C == BP_TRUE)
		{
			print_line(as, 0, ' ', 0);
		}
	}
	as->conditional_stack_index++;
	if (as->conditional_stack_index > CONDSTACKLEN)
	{
		/* overflow */
		error(as, "Conditional stack overflow!");
		return 0;
	}
	if (result == 0)
	{
		as->conditional_stack[as->conditional_stack_index] = 1;
	}
	else
	{
		as->conditional_stack[as->conditional_stack_index] = 0;
	}
	return 0;
}



/*
 * ifne - if expression != 0 conditional
 */
int _ifne(assembler *as)
{
	BP_int32	result;


	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{	
		/* prev. conditional false, ignore this one */
		result = 0;
	}
	else
	{
		/* prev. conditional true, evaluate this one */
		evaluate(as, &result, &as->line->optr, 1);
		if (as->Opt_C == BP_TRUE)
		{
			print_line(as, 0, ' ', 0);
		}
	}
	as->conditional_stack_index++;
	if (as->conditional_stack_index > CONDSTACKLEN)
	{
		/* overflow */
		error(as, "Conditional stack overflow!");
		return 0;
	}
	if (result != 0)
	{
		as->conditional_stack[as->conditional_stack_index] = 1;
	}
	else
	{
		as->conditional_stack[as->conditional_stack_index] = 0;
	}
	return 0;
}


/*
 * iflt - if expression < 0 conditional
 */
int _iflt(assembler *as)
{
	BP_int32	result;


	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{	
		/* prev. conditional false, ignore this one */
		result = 0;
	}
	else
	{
		/* prev. conditional true, evaluate this one */
		evaluate(as, &result, &as->line->optr, 1);
		if (as->Opt_C == BP_TRUE)
		{
			print_line(as, 0, ' ', 0);
		}
	}
	as->conditional_stack_index++;
	if (as->conditional_stack_index > CONDSTACKLEN)
	{
		/* overflow */
		error(as, "Conditional stack overflow!");
		return 0;
	}
	if (result < 0)
	{
		as->conditional_stack[as->conditional_stack_index] = 1;
	}
	else
	{
		as->conditional_stack[as->conditional_stack_index] = 0;
	}
	return 0;
}


/*
 * ifle - if expression <= 0 conditional
 */
int _ifle(assembler *as)
{
	BP_int32	result;


	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{	
		/* prev. conditional false, ignore this one */
		result = 0;
	}
	else
	{
		/* prev. conditional true, evaluate this one */
		evaluate(as, &result, &as->line->optr, 1);
		if (as->Opt_C == BP_TRUE)
		{
			print_line(as, 0, ' ', 0);
		}
	}
	as->conditional_stack_index++;
	if (as->conditional_stack_index > CONDSTACKLEN)
	{
		/* overflow */
		error(as, "Conditional stack overflow!");
		return 0;
	}
	if (result <= 0)
	{
		as->conditional_stack[as->conditional_stack_index] = 1;
	}
	else
	{
		as->conditional_stack[as->conditional_stack_index] = 0;
	}
	return 0;
}


/*
 * ifgt - if expression > 0 conditional
 */
int _ifgt(assembler *as)
{
	BP_int32	result;


	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{	
		/* prev. conditional false, ignore this one */
		result = 0;
	}
	else
	{
		/* prev. conditional true, evaluate this one */
		evaluate(as, &result, &as->line->optr, 1);
		if (as->Opt_C == BP_TRUE)
		{
			print_line(as, 0, ' ', 0);
		}
	}
	as->conditional_stack_index++;
	if (as->conditional_stack_index > CONDSTACKLEN)
	{
		/* overflow */
		error(as, "Conditional stack overflow!");
		return 0;
	}
	if (result > 0)
	{
		as->conditional_stack[as->conditional_stack_index] = 1;
	}
	else
	{
		as->conditional_stack[as->conditional_stack_index] = 0;
	}
	return 0;
}


/*
 * ifge - if expression >= 0 conditional
 */
int _ifge(assembler *as)
{
	BP_int32	result;


	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{	
		/* prev. conditional false, ignore this one */
		result = 0;
	}
	else
	{
		/* prev. conditional true, evaluate this one */
		evaluate(as, &result, &as->line->optr, 1);
		if (as->Opt_C == BP_TRUE)
		{
			print_line(as, 0, ' ', 0);
		}
	}
	as->conditional_stack_index++;
	if (as->conditional_stack_index > CONDSTACKLEN)
	{
		/* overflow */
		error(as, "Conditional stack overflow!");
		return 0;
	}
	if (result >= 0)
	{
		as->conditional_stack[as->conditional_stack_index] = 1;
	}
	else
	{
		as->conditional_stack[as->conditional_stack_index] = 0;
	}
	return 0;
}


/*
 * endc - end conditional
 */
int _endc(assembler *as)
{
	as->conditional_stack_index--;
	if (as->conditional_stack_index < 0)
	{
		/* underflow */
		error(as, "endc without a conditional if!");
		return 0;
	}
	if (as->Opt_C == BP_TRUE)
	{
		print_line(as, 0, ' ', 0);
	}
	return 0;
}


/*
 * else - change current conditional
 */
int _else(assembler *as)
{
	/* if the previous conditional was false... */
	if (as->conditional_stack_index > 0 && as->conditional_stack[as->conditional_stack_index-1] == 0)
	{
		/* ...then ignore this one */
		return 0;
	}
	/* invert the sense of the conditional */
	if (as->Opt_C == BP_TRUE)
	{
		print_line(as, 0, ' ', 0);
	}
	as->conditional_stack[as->conditional_stack_index] = !as->conditional_stack[as->conditional_stack_index];
	if (as->Opt_C == BP_TRUE)
	{
		print_line(as, 0, ' ', 0);
	}
	return 0;
}


/*
 * mod - generate OS-9 header module
 */
int _mod(assembler *as)
{
#define HEADER_LEN	9
#define	_MODSYNC		0x87CD
	char *p;
	unsigned char header_check;
	BP_int32 modinfo[6], i;
	int module_size, name_offset;

	as->old_program_counter = as->program_counter = 0;
	as->data_counter = 0;
	f_record(as);	/* flush out any bytes */
	as->do_module_crc = BP_TRUE;

	as->_crc[0] = 0xFF;
	as->_crc[1] = 0xFF;
	as->_crc[2] = 0xFF;

	/* Start OS-9 module */
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		/* ignore this pseudo op */
		return 0;
	}

	if (as->pass == 1 && *as->line->label != EOS)
	{
		symbol_add(as, as->line->label, as->old_program_counter, 0);
	}

	/* obtain first parameter -- length of module */
	if ((p = strtok(as->line->optr, ",")) == NULL)
	{
		/* error */
		error(as, "Missing parameter");
		return 0;
	}
	evaluate(as, &modinfo[0], &p, 0);

	/* obtain rest of parameters */
	for (i = 1; i < 6; i++)
	{
		if ((p = strtok(NULL, ",")) == NULL)
		{
			/* error */
			error(as, "Missing parameter");
			return 0;
		}
		evaluate(as, &modinfo[i], &p, 0);
	}	

	header_check = 0;

	/* emit sync bytes */
	eword(as, _MODSYNC);
	header_check ^= _MODSYNC >> 8;
	header_check ^= _MODSYNC & 0xFF;

	/* emit module size */
	module_size = modinfo[0];
	eword(as, module_size);
	header_check ^= module_size >> 8;
	header_check ^= module_size & 0xFF;

	/* emit name offset */
	name_offset = modinfo[1];
	eword(as, name_offset);
	header_check ^= name_offset >> 8;
	header_check ^= name_offset & 0xFF;

	/* emit type/language and attribute/revision */
	emit(as, modinfo[2]);
	header_check ^= modinfo[2];

	emit(as, modinfo[3]);
	header_check ^= modinfo[3];

	/* emit header check */
	emit(as, ~header_check);

	/* module type specific output */
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

	/* re-adjust PC to undo effects of emit */
	print_line(as, 0, ' ', 0);
	return 0;
}


/*
 * emod - generate OS-9 module CRC
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
	as->do_module_crc = BP_FALSE;
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


/*
 * setdp - set direct page (motorola mode)
 */
int _setdp(assembler *as)   /* TODO! */
{
	BP_int32	newdp;


	as->P_force = 1;

	evaluate(as, &newdp, &as->line->optr, 0);
	as->DP = newdp;

	as->data_counter = newdp;
	f_record(as);
	print_line(as, 0, 'D', as->data_counter);
	return 0;
}


/*
 * spc - add spaces to printed output
 */
int _spc(assembler *as)	/* TODO! */
{
	int lines = atoi(as->line->operand);

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


/*
 * use - include other assembly or definition files
 */
int _use(assembler *as)
{
	error_code ec = 0;

	
	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	/* 2. Test for presence of a label and return error if found. */

	if (*as->line->label != EOS)
	{
		error(as, "label not allowed");
	}

	
	/* 3. Print the line. */
	
	print_line(as, 0, ' ', 0);

	{
		struct filestack use_file, *prev_file;
		BP_char		path[FNAMESIZE];
		int			i = 0;
		
		
		/* 1. Set up the structure. */

		prev_file = as->current_file;
		
		as->current_file = &use_file;
		
		strncpy(use_file.file, as->line->optr, FNAMESIZE);

		use_file.current_line = 0;
		use_file.num_blank_lines = 0;
		use_file.num_comment_lines = 0;
		use_file.end_encountered = BP_FALSE;
		
		
		/* 2. Open a path to the file. */
		
		strncpy(path, use_file.file, FNAMESIZE);

		do
		{
			ec = _coco_open(&(use_file.fd), path, FAM_READ);

			if (ec != 0 && i < as->include_index)
			{
				/* 1. Try any alternate include directories. */
			
				strcpy(path, as->includes[i]);
				strcat(path, "/");
				strcat(path, use_file.file);				
			}
		} while (ec != 0 && i++ < as->include_index);
		
		if (ec == 0)
		{
			/* 1. Make the first pass. */
			
			as->use_depth++;
			
			mamou_pass(as);
			
			
			/* 2. Close the file. */
			
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


/*
 * end - stop further processesing in this pass
 */
int __end(assembler *as)
{
	/* 1. If we are currently in a FALSE conditional, just return. */
	
	if (as->conditional_stack[as->conditional_stack_index] == 0)
	{
		return 0;
	}


	/* test for presence of a label and error if found */
	if (*as->line->label != EOS)
	{
		error(as, "label not allowed");
		return 0;
	}
	else
	{
		/* If we are in pass 2 and this is DECB mode, evaluate the operand,
		 * if any, for the EXEC address.
		 */

		if (as->pass == 2 && as->o_asm_mode == ASM_DECB)
		{
			evaluate(as, &as->decb_exec_address, &as->line->optr, 0);
		}
		
		print_line(as, 0, ' ', 0);

		as->current_file->end_encountered = BP_TRUE;
	}

	return 0;
}
