/***************************************************************************
 * mamou.c: command line parsing routines and assembler engine
 *
 * $Id$
 *
 * The Mamou Assembler - A Hitachi 6309 assembler
 *
 * (C) 2004 Boisy G. Pitre
 ***************************************************************************/

#include "mamou.h"


/* Static functions. */

static int mamou_assemble(assembler *as);
static void mamou_initialize(assembler *as);
static void mamou_deinitialize(assembler *as);

char product_name[256];
char product_copyright[256];


/*!
	@function main
	@discussion main entry point
	@param argc argument count
	@param argv argument vector
 */
int main(int argc, char **argv)
{
	char			*p;
	char			*i;
	int				j = 0;
    int				v;
	assembler		as;
	
	/* 1. Initialize our globals. */
    as.arguments = argv;

    mamou_init_assembler(&as);
 
	sprintf(product_name, "The Mamou Assembler Version %02d.%02d",
			VERSION_MAJOR, VERSION_MINOR);
	
	sprintf(product_copyright, "Copyright (C) 2004 Boisy G. Pitre");

	/* 2. Display help, if necessary. */	
	if (argc < 2)
    {
		fprintf(stderr, "%s\n", product_name);
		fprintf(stderr, "%s\n", product_copyright);
		fprintf(stderr, "\n");
		fprintf(stderr, "General options:\n");
        fprintf(stderr, " -a<sym>[=<val>] assign val to sym\n");
        fprintf(stderr, " -d        debug mode\n");
        fprintf(stderr, " -e        enhanced 6309 assembler mode\n");
		fprintf(stderr, " -ee       enhanced 6309 and X9 assembler mode\n");
        fprintf(stderr, " -i<dir>   additional include directories\n");
        fprintf(stderr, " -p        don't assemble, just parse\n");
        fprintf(stderr, " -q        quiet mode\n");
        fprintf(stderr, " -x        suppress warnings and errors\n");
        fprintf(stderr, " -y        include instruction cycle count\n");
        fprintf(stderr, " -z        suppress conditionals in assembly list output\n");
		fprintf(stderr, "Source listing options:\n");
        fprintf(stderr, " -c        show symbol cross reference table\n");
        fprintf(stderr, " -l        list file\n");
        fprintf(stderr, " -ls       source only list (no line numbers)\n");
        fprintf(stderr, " -ln       format source in 'new style' assembly\n");
        fprintf(stderr, " -lt       use tabs instead of spaces\n");
        fprintf(stderr, " -lu       force pseudo-ops to print in uppercase\n");
        fprintf(stderr, " -np       suppress 'page' pseudo output\n");
        fprintf(stderr, " -o<file>  output to file\n");
        fprintf(stderr, " -s        show symbol table\n");
		fprintf(stderr, "Assembler modes (select only one):\n");
        fprintf(stderr, " -9        OS-9/6809 (default)\n");
        fprintf(stderr, " -b        Disk BASIC\n");
		fprintf(stderr, " -r        Rom Absolute mode\n");
		fprintf(stderr, "Object generation options (select only one):\n");
        fprintf(stderr, " -tb       binary object output (default)\n");
        fprintf(stderr, " -th       hex object output\n");
        fprintf(stderr, " -ts       s-record object output\n");

        exit(1);
    }

    /* 3. Parse command line for options */
    for (j = 1; j < argc; j++)
    {
        if (*argv[j] == '-')
        {
            switch (tolower(argv[j][1]))
            {
				case '9':
                    as.o_asm_mode = ASM_OS9;
                    break;
										
                case 'a':
                    /* Symbol define */
                    p = &argv[j][2];

                    if (*p != EOS)
                    {
						i = p;

						while (*i != '=' && *i != '\0')
						{
							i++;
						}

						/* Now i points to '=' or \0 */
						if (*i == '=')
						{
							*i = '\0';
							i++;
							v = atoi(i);
						}
						else
						{
							v = 1;
						}
						
						/* Add value */
						symbol_add(&as, p, v, 0);
					}
                    break;
					
                case 'b':
                    as.o_asm_mode = ASM_DECB;
                    break;
					
                case 'c':
                    /* Cross reference output */
                    as.o_show_cross_reference = 1;
                    break;
					
                case 'd':
                    /* Debug mode */
                    as.o_debug = 1;
                    break;
					
                case 'e':
                    /* 6309 extended instruction mode */
                    as.o_cpuclass = CPU_H6309;
					if (tolower(argv[j][2]) == 'e') as.o_cpuclass = CPU_X9;
						break;	
					
                case 'i':
                    /* Include directive */
                    if (as.include_index + 1 == INCSIZE)
                    {
                        /* Reached our capacity */
                        break;
                    }
                    p = &argv[j][2];
                    if (*p == '=')
                    {
                        p++;
                    }
                    as.includes[as.include_index++] = p;
                    break;
					
                case 'l':
                    /* List file */
                    if (tolower(argv[j][2]) == 's')
                    {
                        as.o_format_only = 1;
                        as.o_pagewidth = 256;
                    }
                    if (tolower(argv[j][2]) == 't')
                    {
                        as.tabbed = 1;
                    }
                    if (tolower(argv[j][2]) == 'n')
                    {
                        as.newstyle = 1;
                    }
                    if (tolower(argv[j][2]) == 'u')
                    {
                        as.pseudoUppercase = 1;
                    }
                    as.o_show_listing = 1;
                    break;
					
                case 'o':
                    /* Output file */
                    p = &argv[j][2];
					if (*p == EOS)
					{
						strcpy(as.object_name, "mamouobj");
					}
					else
					{
						strncpy(as.object_name, p, FNAMESIZE - 1);
					}
					as.object_output = 1;
                    break;
					
                case 'p':
                    /* Parse only output */
                    as.o_cpuclass = CPU_H6309; /* So 6309 instructions come out looking good when printed */
                    as.o_do_parsing = 0;
                    break;
					
                case 'q':
                    /* Quiet mode */
                    as.o_quiet_mode = 1;
                    break;
				
				case 'r':
					/* Rom mode, like basic mode without header, footer */
                    as.o_asm_mode = ASM_ROM;
                    break;
					
                case 's':
                    /* Symbol table dump */
                    as.o_show_symbol_table = 1;
                    break;
					
                case 't':
                    /* Object type */
                    if (tolower(argv[j][2]) == 'b')
                    {
                        as.output_type = OUTPUT_BINARY;
                    }
					else if (tolower(argv[j][2]) == 'h')
                    {
                        as.output_type = OUTPUT_HEX;
                    }
					else if (tolower(argv[j][2]) == 's')
                    {
                        as.output_type = OUTPUT_SRECORD;
                    }
					else
                    {
						fprintf(stderr, "bad option\n");
                    }
                    break;
					
                case 'x':
                    /* Suppress errors and warnings */
                    as.ignore_errors = 1;
                    break;
					
                case 'y':
                    /* Cycle count (sort of works) */
                    as.f_count_cycles = 1;
                    break;
					
                case 'z':
                    as.o_show_cond = 0;
                    break;
                    
                default:
                    /* Bad option */
                    fprintf(stderr, "Unknown option\n");
                    exit(0);
            }
        }
        else if (as.file_index + 1 < MAXAFILE)
        {
			/* 1. Add the filename to the file list array. */			
            as.file_name[as.file_index++] = argv[j];
        }
    }
	
	/* 4. Call the assembler to do its work. */
	return mamou_assemble(&as);
}



/*!
	@function mamou_assemble
	@discussion Orchestrates the assembly process for all files
	@param as The assembler state structure
 */

int mamou_assemble(assembler *as)
{
	int ret = 0;

	/* Get the current time for future reference. */
	as->start_time = time(NULL);
	
	/* Initialize the assembler for the first pass. */
	as->pass = 1;
	
    mamou_initialize(as);

	/* For each file we have to assemble... */
    for (as->current_filename_index = 0; as->current_filename_index < as->file_index; as->current_filename_index++)
    {
		struct filestack root_file;
		
		/* Set up the structure. */
		as->current_file = &root_file;
		
		strncpy(root_file.file, as->file_name[as->current_filename_index], FNAMESIZE);
		root_file.current_line = 0;
		root_file.num_blank_lines = 0;
		root_file.num_comment_lines = 0;
		root_file.end_encountered = 0;
		
		/* Open a path to the file. */
        if (_coco_open(&(root_file.fd), root_file.file, FAM_READ) != 0)
        {
            printf("mamou: can't open %s\n", root_file.file);

            return 1;
        }

		/* Make the first pass. */		
		mamou_pass(as);

		/* Close the file. */		
		_coco_close(root_file.fd);
		
		/* Did we have more 'ifs' than 'endcs' ? */
		if (as->conditional_stack_index != 0)
		{			
			error(as, "too many IFs for ENDCs");
		}
    }

	/* If the assembly pass above yielded no errors... */
    if (as->num_errors == 0)
    {
		/********** SECOND PASS **********/
		
		/* Increment the pass. */
        as->pass++;
		
		/* Re-initialize the assembler. */
        mamou_initialize(as);
		
		/* Walk the file list again... */
        for (as->current_filename_index = 0; as->current_filename_index < as->file_index; as->current_filename_index++)
        {
			struct filestack root_file;
			
			/* Set up the structure. */
			as->current_file = &root_file;
			
			strncpy(root_file.file, as->file_name[as->current_filename_index], FNAMESIZE);
			root_file.current_line = 0;
			root_file.num_blank_lines = 0;
			root_file.num_comment_lines = 0;
			root_file.end_encountered = 0;
			
			/* Open a path to the file. */
			if (_coco_open(&(root_file.fd), root_file.file, FAM_READ) != 0)
			{
				printf("mamou: can't open %s\n", root_file.file);
				
				return 1;
			}			
			
			/* Make the second pass. */
			mamou_pass(as);			
			
			/* Close the file. */
			_coco_close(root_file.fd);
        }		

		if (as->o_asm_mode == ASM_DECB)
		{
			/* Emit Disk BASIC trailer. */		
			decb_trailer_emit(as, 0xEEAA);
		}
		
		/* Do we show the symbol table? */		
        if (as->o_show_symbol_table == 1)
        {
            printf("\f");

            symbol_dump_bucket(as->bucket);

            printf("\n");
        }
        
        if (as->o_show_cross_reference == 1)
        {
            printf("\f");
			
            symbol_cross_reference(as->bucket);
        }

        finish_outfile(as);
    }

    if ((as->o_quiet_mode == 0) && (as->o_format_only == 0))
    {
        print_summary(as);
    }

	/* Terminate the forward reference file. */
    fwd_deinit(as);

    /* Added to remove an object if there were errors in the assembly - BGP 2002/07/25 */
    if (as->num_errors != 0)
    {
		ret = 1;			/* error status */
        _coco_delete(as->object_name);
    }

	/* Deinitialize the assembler. */
    mamou_deinitialize(as);

	/* Return. */
    return ret;
}


/*!
	@function mamou_initialize
	@discussion Initialize the assembler for each pass
	@param as The assembler state structure
 */
static void mamou_initialize(assembler *as)
{
    if (as->o_debug)
    {
        printf("Initializing for pass %u\n", (unsigned int)as->pass);
    }

	if (as->pass == 1)
	{
		/* Pass 1 initialization. */
		as->current_psect			= -1;
		as->code_segment_start			= 1;
		as->num_errors				= 0;
		as->cumulative_blank_lines  = 0;
		as->cumulative_comment_lines  = 0;
		as->cumulative_total_lines  = 0;
		as->data_counter			= 0;
		as->program_counter			= 0;
		as->pass					= 1;
		as->Ctotal					= 0;
		as->f_new_page				= 0;
		as->use_depth				= 0;
		
		as->conditional_stack_index = 0;
		as->conditional_stack[0]	= 1;

		if (as->object_name[0] != EOS)
		{
			_path_type t;
			
			if (_coco_create(&(as->fd_object), as->object_name,
				FAM_READ | FAM_WRITE,
				FAP_READ | FAP_WRITE | FAP_PREAD) != 0)
			{
				fatal("Can't create object file");
			}
			
			/* This code sets the binary file type for Disk BASIC Files - tjl 8/8/2004 */
			_coco_gs_pathtype(as->fd_object, &t);
			
			if (as->o_asm_mode == ASM_DECB && t == DECB)
			{
				decb_file_stat f;
				
				_decb_gs_fd(as->fd_object->path.decb, &f);
				
				f.file_type = 2;
				
				_decb_ss_fd(as->fd_object->path.decb, &f);
			}
		}

		fwd_init(as);		/* forward ref init */
		local_init();		/* target machine specific init. */
		
		/* Get 'include' environment variable. */
		{
			char *include;
			
			/* 1. Get defs directory environment variable. */
			
			include = getenv("MAMOU_INCLUDE");
			
			if (include != NULL)
			{
				as->includes[as->include_index++] = include;
			}
		}
	}
	else
	{
		/* Pass 2 initialization. */
		as->current_psect			= -1;
		as->cumulative_blank_lines  = 0;
		as->cumulative_comment_lines  = 0;
		as->cumulative_total_lines  = 0;
		as->data_counter    = 0;
		as->program_counter	= 0;
		as->DP				= 0;
		as->E_total			= 0;
		as->P_total			= 0;
		as->Ctotal			= 0;
		as->f_new_page		= 0;
		as->use_depth		= 0;
				
		fwd_reinit(as);

		as->conditional_stack_index = 0;
		as->conditional_stack[0] = 1;
//		as->conditional_stack[as->conditional_stack_index] = 1;
	}

    return;
}


/*!
	@function mamou_deinitialize
	@discussion Deinitializes the assembler
	@param as The assembler state structure
 */
static void mamou_deinitialize(assembler *as)
{
    if (as->o_debug)
    {
        printf("Deinitializing\n");
    }

    return;
}


/*!
	@function mamou_pass
	@discussion Makes one pass through the source
	@param as The assembler state structure
 */
void mamou_pass(assembler *as)
{
	u_int		size = MAXBUF - 1;
	char		input_line[1024];
	
	/* 1. If debug mode is on, show output. */
    if (as->o_debug)
    {
        printf("\n------");
        printf("\nPass %u", (unsigned int)as->pass);
        printf("\n------\n");
    }
	
	/* 2. While we haven't encountered 'end' and there are more lines to read... */
	while (as->current_file->end_encountered == 0 && _coco_readln(as->current_file->fd, input_line, &size) == 0)
	{
		char *p = strchr(input_line, 0x0D);

		size = MAXBUF - 1;

		if (p != NULL)
		{
#ifdef _WIN32
			p++;
			*p = 0x0A;
#else
			*p = 0x0A;
#endif
			p++;
			*p = '\0';
		}

		as->current_file->current_line++;
		as->P_force = 0;	/* No force unless bytes emitted */
		as->f_new_page = 0;
	
		mamou_parse_line(as, input_line);
		
		if (as->line.type == LINETYPE_SOURCE && as->o_do_parsing == 1)
		{
			process(as);
		}
		else
		{
			print_line(as, 0, ' ', 0);
			
			/* Keep track of the number of blank and comment lines. */			
			if (as->line.type == LINETYPE_BLANK)
			{
				as->current_file->num_blank_lines++;
				as->cumulative_blank_lines++;
			}
			else
			{
				as->current_file->num_comment_lines++;
				as->cumulative_comment_lines++;
			}
		}
		
		/* Keep track of the total number of lines so far. */
		as->cumulative_total_lines++;
	
		as->P_total = 0;	/* reset byte count */

		as->cumulative_cycles = 0;	/* and per instruction cycle count */
	}

    f_record(as);

	return;
}


/*!
	@function mamou_parse_line
	@discussion Splits the input line into label, opcode, operand and comment.
	@discussion This function also finds the mnemonic entry (pseudo or real)
	@discussion from the appropriate mnemonic table.
	@param as The assembler state structure
	@param input_line A pointer to the line to parse
 */
void mamou_parse_line(assembler *as, char *input_line)
{
    char *ptrfrm = input_line;
    char *ptrto = as->line.label;
	
	/* 1. Initialize line structure. */
	as->line.has_warning = 0;
	as->line.optr = as->line.Op;
	as->line.force_word = 0;
	as->line.force_byte = 0;
    *as->line.label = EOS;
    *as->line.Op = EOS;
    *as->line.operand = EOS;
    *as->line.comment = EOS;	
	
	/* 2. First, check to see if this line has the 5 byte numerical field that
	 * is associated with EDTASM source files.
	 */	
	if (
		numeric(*(ptrfrm + 0)) == 1 &&
		numeric(*(ptrfrm + 1)) == 1 &&
		numeric(*(ptrfrm + 2)) == 1 &&
		numeric(*(ptrfrm + 3)) == 1 &&
		numeric(*(ptrfrm + 4)) == 1 &&
		*(ptrfrm + 5) == ' '
		)
	{
		ptrfrm += 6;
		input_line += 6;
	}
	
	/* 3. Check to see if this is a blank line. */
	while (isspace(*ptrfrm)) ptrfrm++;
	
	if (*ptrfrm == '\n' || *ptrfrm == EOS)
	{
		*ptrto = EOS;
		
		as->line.type = LINETYPE_BLANK;
		
		return;
	}
	
	/* 4. Reanchor pointer to start of line. */
	ptrfrm = input_line;

    /* 5. Check for comment characters. */
	if (*ptrfrm == '*' || *ptrfrm == ';' || *ptrfrm == '#')
    {
        strcpy(as->line.comment, input_line);

        ptrto = as->line.comment;
		
        while (!eol(*ptrto))
        {
            ptrto++;
        }
		
        *ptrto = EOS;
		
		as->line.type = LINETYPE_COMMENT;
		
        return;
    }
	
    while (delim(*ptrfrm) == 0)
    {
        *ptrto++ = *ptrfrm++;
    }
	
    if (ptrto > as->line.label && *--ptrto != ':')
    {
        ptrto++;     /* allow trailing : */
    }
	
    *ptrto = EOS;
	
	
	/* Skip over whitespace. */
    ptrfrm = skip_white(ptrfrm);
	
    ptrto = as->line.Op;
	
    while (delim(*ptrfrm) == 0)
    {
        *ptrto++ = mapdn(*ptrfrm++);
    }
	
    *ptrto = EOS;
	
    ptrfrm = skip_white(ptrfrm);
	
	
    /* Look up the mnemonic */
    if (mne_look(as, as->line.Op, &as->line.mnemonic) == 0)
    {
		/* Does this op code have a parameter? */
        if ((as->line.mnemonic.type == OPCODE_PSEUDO  && as->line.mnemonic.opcode.pseudo->info != HAS_NO_OPERAND) ||
			(as->line.mnemonic.type == OPCODE_H6309 && 
            (as->line.mnemonic.opcode.h6309->class != INH && as->line.mnemonic.opcode.h6309->class != P2INH && as->line.mnemonic.opcode.h6309->class != P3INH))
		)
        {
			/* Yes, it does. */
            ptrto = as->line.operand;

            if (as->line.mnemonic.type == OPCODE_PSEUDO && as->line.mnemonic.opcode.pseudo->info == HAS_OPERAND_WITH_DELIMITERS)
            {
                char fccdelim;

				/* Check for nul operand */				
				if (*ptrfrm == EOS || eol(*ptrfrm))
				{
					if (as->pass == 2)
					{
						error(as, "operand required");
					}
				}
				else
				{
					/* Pseudo opcode with delimiter bounded data (i.e. fcc, fcs). */				
					fccdelim = *ptrfrm;

					do
					{
						*ptrto++ = *ptrfrm++;
					} while (*ptrfrm != EOS && *ptrfrm != fccdelim);
				
					*ptrto++ = *ptrfrm++;
				}
            }
            else if (as->line.mnemonic.type == OPCODE_PSEUDO && as->line.mnemonic.opcode.pseudo->info == HAS_OPERAND_WITH_SPACES)
            {
                /* Pseudo opcode with spaces in the operand. */				
				/* Check for nul operand */				
				if (*ptrfrm == EOS || eol(*ptrfrm))
				{
					if (as->pass == 2)
					{
						error(as, "operand required");
					}
				}
				else
				{
					do
					{
						*ptrto++ = *ptrfrm++;
					} while (*ptrfrm != EOS && !eol(*ptrfrm));
				}
            }
            else
            {
				/* Pseudo or real opcode with regular operand */                
				while (delim(*ptrfrm) == 0)
                {
                    *ptrto++ = *ptrfrm++;
                }
            }
			
            *ptrto = EOS;
			
            ptrfrm = skip_white(ptrfrm);
        }
    }
		
	/* Point to the line's comment field and suck up the remainder of the line as a comment. */	
    ptrto = as->line.comment;
	
    while (!eol(*ptrfrm))
    {
        *ptrto++ = *ptrfrm++;
    }
	
    *ptrto = EOS;

    /* If debug mode is on, print the line information. */	
    if (as->o_debug)
    {
        printf("\n");
        printf("Label      %s\n", as->line.label);
        printf("Op         %s\n", as->line.Op);
        printf("Operand    %s\n", as->line.operand);
    }
	
	as->line.type = LINETYPE_SOURCE;
	
	return;
}


/*!
	@function process
	@discussion Process an assembled line
	@param as The assembler state structure
 */
void process(assembler *as)
{	
	/* Setup `old' program counter */	
    as->old_program_counter = as->program_counter;
 	
	/* Point to beginning of operand field */
    as->line.optr = as->line.operand;
	
	/* Determine if we are in a FALSE conditional */
    if (as->conditional_stack[as->conditional_stack_index] == 0)
    {
        /* We are... ignore this line unless it's an PSEUDO op */
        if (
			as->line.mnemonic.type == OPCODE_PSEUDO
//			(as->line.mnemonic.opcode.pseudo->class == IF || as->line.mnemonic.opcode.pseudo->class == ELSE || as->line.mnemonic.opcode.pseudo->class == ENDC)
		)
        {
			/* Call the pseudo func */
			as->line.mnemonic.opcode.pseudo->func(as);
        }
		
        return;
    }

	/* At this point we are in a TRUE conditional */
    if (*as->line.Op == EOS)
    {
        /* no mnemonic */
        if (*as->line.label != EOS)
        {
            symbol_add(as, as->line.label, as->program_counter, 0);

            print_line(as, 0, ' ', 0);
        }
    }
    else if (as->line.mnemonic.type == OPCODE_UNKNOWN) // && as->pass > 1)
	{
		error(as, "unrecognized mnemonic");
	}
    else if (as->line.mnemonic.type == OPCODE_PSEUDO)
    {
        as->line.mnemonic.opcode.pseudo->func(as);
		
        if (as->E_total >= E_LIMIT)
        {
            f_record(as);
        }
    }
    else
    {
        if (*as->line.label)
        {
            symbol_add(as, as->line.label, as->program_counter, 0);
        }
        if (as->f_count_cycles)
        {
            as->cumulative_cycles = as->line.mnemonic.opcode.h6309->cycles;
            if (as->o_cpuclass >= CPU_H6309)
            {
                as->cumulative_cycles--;
            }
        }
        as->line.mnemonic.opcode.h6309->func(as, as->line.mnemonic.opcode.h6309->opcode);
		
        if (as->E_total >= E_LIMIT)
        {
            f_record(as);
        }
        
        if (as->f_count_cycles)
        {
            as->Ctotal += as->cumulative_cycles;
        }
    }
}


/*!
	@function mamou_init_assembler
	@discussion Initializes an instance of the assembler to default values
	@param as The assembler state structure
 */
void mamou_init_assembler(assembler *as)
{
	memset(as, 0, sizeof(assembler));
	
    as->output_type = OUTPUT_BINARY;
    as->pass			= 1;		/* Current pass #               */
    as->page_number		= 2;		/* page number */
    as->o_show_cond			= 1;		/* show conditionals in listing */
    as->o_page_depth	= 66;
    as->o_show_error	= 1;
    as->o_pagewidth		= 80;
    as->_crc[0]			= 0xFF;
    as->_crc[1]			= 0xFF;
    as->_crc[2]			= 0xFF;
    as->o_do_parsing	= 1;
    as->current_page	= 1;
    as->header_depth	= 3;
    as->footer_depth	= 3;
    as->o_asm_mode		= ASM_OS9;
	as->newstyle		= 0;

    return;
}
