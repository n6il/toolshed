/***************************************************************************
* util.c: common utility functions
*
* $Id$
*
* The Mamou Assembler - A Hitachi 6309 assembler
*
* (C) 2004 Boisy G. Pitre
***************************************************************************/

#include	"mamou.h"
#include	"os9module.h"


/*!
	@function extractfilename
	@discussion Returns the filename component of a string
	@discussion (e.g. /home/darthvader/file.c returns file.c
	@param pathlist Pointer to the pathlist to evaluate
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


/*!
	@function fatal
	@discussion Fatal error handler
	@param str String to report
 */

void fatal(char *str)
{
	fprintf(stderr, "%s\n", str);

	
	exit(-1);
}



/*!
	@function error
	@discussion Reports an error in a line.
	@param as The assembler state structure
	@param str String to report
 */

void error(assembler *as, char *str)
{
	if (as->ignore_errors == BP_TRUE)
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



/*!
	@function delim
	@discussion Determine if the character is a delimiter
	@param c Character to evaluate
 */

BP_Bool delim(BP_char c)
{
	if (any(c, " \t\n\r"))
	{
		return BP_TRUE;
	}
	
	
	return BP_FALSE;
}



/*!
	@function eol
	@discussion Determine if character is an end-of-line character
	@param c Character to evaluate
 */

BP_Bool eol(BP_char c)
{
	if (any(c, "\n\r"))
	{
		return BP_TRUE;
	}
	
	
	return BP_FALSE;
}



/*!
	@function skip_white
	@discussion Move poiner to next non-whitespace character
	@param ptr String to process
 */

BP_char *skip_white(BP_char *ptr)
{
	while (*ptr == BLANK || *ptr == TAB)
	{
		ptr++;
	}


	return ptr;
}



/*!
	@function eqword
	@discussion Emit a quad word to the code file
	@param as The assembler state structure
	@param qwd Quad word to emit
 */

void equad(assembler *as, BP_int32 qwd)
{
	eword(as, hiword(qwd));
	eword(as, loword(qwd));
	
	return;
}



/*!
	@function eword
	@discussion Emit a word to the code file
	@param as The assembler state structure
	@param wd Word to emit
 */

void eword(assembler *as, int wd)
{
	emit(as, hibyte(wd));
	emit(as, lobyte(wd));

	return;
}



/*!
	@function emit
	@discussion Emit a byte to the code file
	@param as The assembler state structure
	@param byte Byte to emit
 */

void emit(assembler *as, int byte)
{
	/* 1. Show debug output if flagged. */
	
	if (as->o_debug)
	{
		printf("Emit       %04X[%02X]\n", (unsigned int)as->program_counter, byte);
	}	

	
	/* 3. If this is pass 1... */
	
	if (as->pass == 1)
	{
		/* Is this the start of a code segment? */
		
		if (as->code_segment_start == BP_TRUE)
		{
			/* Yes.  Reset the flag. */
			
			as->code_segment_start = BP_FALSE;

			/* If this is DECB mode, then set the origination and reset size. */
			
			if (as->o_asm_mode == ASM_DECB)
			{
				as->current_psect++;
				as->psect[as->current_psect].org = as->program_counter;
				as->psect[as->current_psect].size = 0;
			}
		}

		as->psect[as->current_psect].size++;
	}
	else
	{
		if (as->code_segment_start == BP_TRUE)
		{
			as->current_psect++;
			
			decb_header_emit(as, as->psect[as->current_psect].org, as->psect[as->current_psect].size);
			
			as->code_segment_start = BP_FALSE;
		}
		
		
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


	/* 2. Increment program counter. */
	
	as->program_counter++;
	
	
	
	return;
}



/*!
	@function decb_header_emit
	@discussion Emit a Disk BASIC code segment header to the code file
	@param as The assembler state structure
	@param start Start of code
	@param size Size of code
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



/*!
	@function decb_trailer_emit
	@discussion Emit a Disk BASIC trailer to an object file
	@param as The assembler state structure
	@param exec Execution address
 */
 
void decb_trailer_emit(assembler *as, BP_uint32 exec)
{
	/* 1. If this is pass 2... */
	
	if (as->pass == 2 && as->o_asm_mode == ASM_DECB)
	{
		/* Disk BASIC trailer */
		
		as->E_bytes[as->E_total++] = 0xFF;
		
		
		/* SIZE (0) */
		
		as->E_bytes[as->E_total++] = 0;
		as->E_bytes[as->E_total++] = 0;
		
	
		/* EXEC address */

		as->E_bytes[as->E_total++] = (as->decb_exec_address & 0xFF00) >> 8;
		as->E_bytes[as->E_total++] = (as->decb_exec_address & 0x00FF);


		if (as->E_total > E_LIMIT + MAXBUF)
		{
			printf("Overflow in E_bytes array\n");
		}
		
		
		f_record(as);
	}
	
		
	return;
}



/*!
	@function f_record
	@discussion Flush one line out in Motorola S or Intel Hex format or binary
	@param as The assembler state structure
 */

void f_record(assembler *as)
{
	int i, chksum;


	/* 1. Pass 2 only. */
	
	if (as->pass == 1)
	{
		return;
	}
	
	
	/* 2. Count up accumulated code bytes. */
	
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
	else if (as->object_output == BP_TRUE)
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
	

	return;
}



char *hexstr = {"0123456789ABCDEF"};


/*!
	@function hex_out
	@discussion Output a byte in hexadecimal
	@param as The assembler state structure
	@param byte Byte to output
 */

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



/*!
	@function finish_outfile
	@discussion Close the output file
	@param as The assembler state structure
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



/*!
	@function any
	@discussion Determines of the string contains the passed character
	@param c Character to search for
	@param str String to search
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



/*!
	@function mapdn
	@discussion Converts uppercase to lowercase
	@param c Character to convert
 */

char mapdn(char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return(c + 040);
	}

	return(c);
}



/*!
	@function lobyte
	@discussion Returns the low byte of an integer
	@param i Integer to process
 */

int lobyte(int i)
{
	return(i & 0xFF);
}


/*!
	@function lobyte
	@discussion Returns the high byte of an integer
	@param i Integer to process
 */

int hibyte(int i)
{
	return((i >> 8) & 0xFF);
}



/*!
	@function loword
	@discussion Returns the low word of an integer
	@param i Integer to process
 */

int loword(BP_int32 i)
{
	return i & 0xFFFF;
}



/*!
	@function hiword
	@discussion Returns the high word of an integer
	@param i Integer to process
 */

int hiword(BP_int32 i)
{
	return (i >> 16) & 0xFFFF;
}



/*!
	@function head
	@discussion Determines if str2 is the head of str1
	@param str1 String to search
	@param str2 String to search for
 */

int head(char *str1, char *str2)
{
	while (tolower(*str1) != EOS && tolower(*str2) != EOS)
	{
		if (tolower(*str1) != tolower(*str2))
		{
			break;
		}
		str1++;
		str2++;
	}

	if (tolower(*str1) == tolower(*str2))
	{
		return(BP_TRUE);
	}

	if (*str2 == EOS)
	{
		if (any(*str1, " \t\n,+-];*"))
		{
			return BP_TRUE;
		}
	}

	
	return BP_FALSE;
}



/*!
	@function alpha
	@discussion Determins if character is a legal letter
	@param c Character to evaluate
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



/*!
	@function alphan
	@discussion Determines if character is a legal letter or digit
	@param c Character to evaluate
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



/*!
	@function numeric
	@discussion Determines if character is a legal digit
	@param c Character to evaluate
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
