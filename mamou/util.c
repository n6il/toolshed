

#include	"mamou.h"
#include	"os9module.h"


/*
 * Returns the filename component of a string
 *
 * (e.g.  /home/darthvader/file.c returns file.c)
 */
char *extractfilename(char *pathlist)
{
	char *f;

	f = strrchr(pathlist, '/');
	if (f == NULL)
	{
		f = pathlist;
	}
	else
	{
		f++;
	}

	return(f);
}


void hexout(assembler *as, int byte);
#if 0
void imageinit(void);
int white(char c);
#endif

/*
 *      fatal --- fatal error handler
 */
void fatal(char *str)
{
	fprintf(stderr, "%s\n", str);
	exit(-1);
}


/*
 *      error --- error in a line
 *                      print line number and error
 */
void error(assembler *as, char *str)
{
	if (as->SuppressFlag == 1)
	{
		return;
	}

	if (as->current_line == 0)
	{
		print_header(as);
	}
	as->current_line++;
	printf("\n***** Error: %s\n", str);
	print_line(as, 1, 'E', as->old_program_counter);
	as->num_errors++;

	return;
}


/*
 *      delim --- check if character is a delimiter
 */

BP_Bool delim(BP_char c)
{
	if (any(c, " \t\n\r"))
	{
		return BP_TRUE;
	}
	
	
	return BP_FALSE;
}



/*
 *      eol --- check if character is eol
 */

BP_Bool eol(BP_char c)
{
	if (any(c, "\n\r"))
	{
		return BP_TRUE;
	}
	
	
	return BP_FALSE;
}


/*
 *      skip_white --- move pointer to next non-whitespace char
 */
BP_char *skip_white(BP_char *ptr)
{
	while (*ptr == BLANK || *ptr == TAB)
	{
		ptr++;
	}


	return ptr;
}



/*
 *      eword --- emit a word to code file
 */
void eword(assembler *as, int wd)
{
	emit(as, hibyte(wd));
	emit(as, lobyte(wd));

	return;
}



/*
 * emit: emit a byte to code file
 */
 
void emit(assembler *as, int byte)
{
	/* 1. Show debug output if flagged. */
	
	if (as->o_debug)
	{
		printf("Emit       %04X[%02X]\n", (unsigned int)as->program_counter, byte);
	}	


	/* 2. Increment program counter. */
	
	as->program_counter++;


	/* 3. If this is pass 2... */
	
	if (as->pass == 1)
	{
		as->orgs[as->current_org].size++;
	}
	else
	{
		if (as->P_total < P_LIMIT)
		{
			as->P_bytes[as->P_total++] = byte;
		}

		as->E_bytes[as->E_total++] = byte;


		if (as->E_total > E_LIMIT + MAXBUF)
		{
			printf("Overflow in E_bytes array\n");
		}
	}

	
	return;
}



/*
 * decb_header_emit: emit code segment header to code file
 */
 
void decb_header_emit(assembler *as, BP_uint32 start, BP_uint32 size)
{
	/* 1. If this is pass 2... */
	
	if (as->pass == 2 && as->o_asm_mode == ASM_DECB)
	{
		/* Disk BASIC Preamble */
		
		as->E_bytes[as->E_total++] = 0;
		
		
		/* Segment size */
		
		as->E_bytes[as->E_total++] = (size & 0xFF00) >> 8;
		as->E_bytes[as->E_total++] = (size & 0x00FF);


		/* ORG address */
		
		as->E_bytes[as->E_total++] = (start & 0xFF00) >> 8;
		as->E_bytes[as->E_total++] = (start & 0x00FF);


		if (as->E_total > E_LIMIT + MAXBUF)
		{
			printf("Overflow in E_bytes array\n");
		}
		
		
		f_record(as);
	}
	
		
	return;
}



/*
 * decb_trailer_emit: emit trailer to object file
 */
 
void decb_trailer_emit(assembler *as, BP_uint32 exec)
{
	/* 1. If this is pass 2... */
	
	if (as->pass == 2 && as->o_asm_mode == ASM_DECB)
	{
		/* Disk BASIC trailer */
		
		as->E_bytes[as->E_total++] = 0xFF;
		
		
		/* EXEC address */
		
		as->E_bytes[as->E_total++] = (as->orgs[0].org & 0xFF00) >> 8;
		as->E_bytes[as->E_total++] = (as->orgs[0].org & 0x00FF);


		if (as->E_total > E_LIMIT + MAXBUF)
		{
			printf("Overflow in E_bytes array\n");
		}
		
		
		f_record(as);
	}
	
		
	return;
}



/*
 *      f_record --- flush one line out in Motorola S or Intel Hex format
 *			or binary
 */
void f_record(assembler *as)
{
	int i, chksum;


	/* 1. Pass 2 only. */
	
	if (as->pass == 2 && as->object_output == BP_TRUE)
	{
		as->code_bytes += as->E_total;
		
		if (as->E_total == 0)
		{
			as->E_pc = as->program_counter;

			return;
		}

		chksum =  as->E_total;    /* total bytes in this record */
		chksum += lobyte(as->E_pc);
		chksum += as->E_pc >> 8;


		/* Compute module CRC. */

		if (as->do_module_crc == BP_TRUE && as->pass == 2)
		{
			_os9_crc_compute(as->E_bytes, as->E_total, as->_crc);
		}


		/* S-Record and Hex files: record header preamble. */

		if (as->output_type == OUTPUT_BINARY && as->object_output == BP_TRUE)
		{
			int		size = as->E_total;
			
			_coco_write(as->fd_object, as->E_bytes, &size);
		}
		else
		{
			int size;
			
			if (as->output_type == OUTPUT_HEX && as->object_output == BP_TRUE)
			{
				size = 1;
				
				_coco_write(as->fd_object, ":", &size);
				
				hexout(as, as->E_total);        /* byte count  */
				hexout(as, 0);		/* Output 00 */
			}
			else if (as->output_type == OUTPUT_SRECORD && as->object_output == BP_TRUE) 		/* S record file */
			{
				size = 2;
				
				chksum += 3;

				_coco_write(as->fd_object, "S1", &size);
				
				hexout(as, as->E_total + 3);      /* byte count +3 */
			}

			hexout(as, as->E_pc >> 8);        	/* high byte of PC */
			hexout(as, lobyte(as->E_pc));		/* low byte of PC */


			for (i = 0; i < as->E_total; i++)
			{
				chksum += lobyte(as->E_bytes[i]);
				hexout(as, lobyte(as->E_bytes[i]));	/* data byte */
			}

			/* ones or twos complement checksum then output it */

			chksum =~ chksum;
			if (as->output_type == OUTPUT_HEX)
			{
				chksum++;
			}

			hexout(as, lobyte(chksum));

			size = 1;
			
			_coco_write(as->fd_object, "\n", &size);
		}
		
		as->E_pc = as->program_counter;
		as->E_total = 0;
	}
	
	
	return;
}



char *hexstr = {"0123456789ABCDEF"};


void hexout(assembler *as, int byte)
{
	if (as->object_output == BP_TRUE)
	{
		int size = 2;
		BP_char tmp[8];
		
		byte = lobyte(byte);
		sprintf(tmp, "%c%c", hexstr[byte >> 4], hexstr[byte & 017]);

		_coco_write(as->fd_object, tmp, &size);
	}


	return;
}


#if 0
/*
 * Init the memory image.
 */
void imageinit(void)
{
	int i;
	for (i = 0; i <= 65535; i++)
	{
		Memmap[i] = 0;
	}

	return;
}
#endif



/*
 * Terminate the output file.
 */
void finish_outfile(assembler *as)
{
	int size;
	
	
	if (as->object_output == BP_FALSE)
	{
		return;
	}

	if (as->output_type == OUTPUT_BINARY)	/* dump the binary bytes to the object file */
	{
	}
	else if (as->output_type == OUTPUT_HEX)
	{
		size = 12;
		
		_coco_write(as->fd_object, ":00000001FF\n", &size);
	}
	else
	{
		size = 11;
		
		_coco_write(as->fd_object, "S9030000FC\n", &size);
	}

	_coco_close(as->fd_object);


	return;
}



/*
 *      any --- does str contain c?
 */
BP_Bool any(BP_char c, BP_char *str)
{
	while (*str != EOS)
	{
		if (*str++ == c)
		{
			return BP_TRUE;
		}
	}


	return BP_FALSE;
}



/*
 *      mapdn --- convert A-Z to a-z
 */
char mapdn(char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return(c + 040);
	}
	return(c);
}


/*
 *      lobyte --- return low byte of an int
 */
int lobyte(int i)
{
	return(i & 0xFF);
}


/*
 *      hibyte --- return high byte of an int
 */
int hibyte(int i)
{
	return((i >> 8) & 0xFF);
}


/*
 *      head --- is str2 the head of str1?
 *      ATD: ??? should be using strncasecmp... the code will get smaller
 */
int head(char *str1, char *str2)
{
	while (*str1 != EOS && *str2 != EOS)
	{
		if (*str1 != *str2)
		{
			break;
		}
		str1++;
		str2++;
	}
	if (*str1 == *str2)
	{
		return(BP_TRUE);
	}
	if (*str2 == EOS)
	{
		if (any(*str1, " \t\n,+-];*"))
		{
			return(BP_TRUE);
		}
	}
	return BP_FALSE;
}



/*
 *      alpha --- is character a legal letter
 */
BP_Bool alpha(BP_char c)
{
	if (c <= 'z' && c >= 'a')
	{
		return BP_TRUE;
	}
	if (c <= 'Z' && c >= 'A')
	{
		return BP_TRUE;
	}
	if (c == '_')
	{
		return BP_TRUE;
	}
	if (c == '.')
	{
		return BP_TRUE;
	}


	return BP_FALSE;
}



/*
 *      alphan --- is character a legal letter or digit
 */

BP_Bool alphan(BP_char c)
{
	if (alpha(c))
	{
		return BP_TRUE;
	}

	if (numeric(c))
	{
		return BP_TRUE;
	}

	if (c == '$' || c == '@')
	{
		return BP_TRUE;      /* allow imbedded $ */
	}
	
	
	return BP_FALSE;
}



/*
 * numeric: is character a legal digit?
 */

BP_Bool numeric(BP_char c)
{
	if (c <= '9' && c >= '0')
	{
		return BP_TRUE;
	}

	
	return BP_FALSE;
}


#if 0
/*
 *	white --- is character whitespace?
 */
int white(char c)
{
	if (c == TAB || c == BLANK || c == '\n')
	{
		return(BP_TRUE);
	}
	return BP_FALSE;
}
#endif



#ifdef _UCC
/*  strucmp.c

    strcmp(s, t) returns > 0, = 0,  or < 0  when s > t, s = t,  or s < t
    according  to  the  ordinary  lexicographical  order.
*/

int strcasecmp(char *s, char *t)
{
	while (toupper(*s) == toupper(*t))  {
		if (*s++ == 0) 
		    return 0;
		t++;
	}
	return (toupper(*s)-toupper(*t));
}
#endif
