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

static void mamou_assemble(assembler *as);
static void mamou_initialize(assembler *as);
static void mamou_deinitialize(assembler *as);

char product_name[256];
char product_copyright[256];


/*
 * main:
 *
 * The main entry point.
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

    init_globals(&as);

 
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
        fprintf(stderr, " -lt       use tabs instead of spaces\n");
        fprintf(stderr, " -np       suppress 'page' pseudo output\n");
        fprintf(stderr, " -o<file>  output to file\n");
        fprintf(stderr, " -s        show symbol table\n");
		fprintf(stderr, "Assembler modes (select only one):\n");
        fprintf(stderr, " -9        OS-9/6809 (default)\n");
        fprintf(stderr, " -b        Disk BASIC\n");
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
                    /* 1. Assembly define specification */
					
                    p = &argv[j][2];

                    if (*p != EOS)
                    {
						i = p;

						while (*i != '=' && *i != '\0')
						{
							i++;
						}

						/* now i points to '=' or \0 */

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
						
						/* add value */
						symbol_add(&as, p, v, 0);
					}
                    break;
					
                case 'b':
                    as.o_asm_mode = ASM_DECB;
                    break;
					
                case 'c':
                    /* cross reference output */
                    as.o_show_cross_reference = BP_TRUE;
                    break;
					
                case 'd':
                    /* debug mode */
                    as.o_debug = BP_TRUE;
                    break;
					
                case 'e':
                    /* 6309 extended instruction mode */
                    as.o_h6309 = BP_TRUE;
                    break;	
					
                case 'i':
                    /* include directive */
                    if (as.include_index + 1 == INCSIZE)
                    {
                        /* reached our capacity */
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
                    /* list file */
                    if (tolower(argv[j][2]) == 's')
                    {
                        as.o_format_only = BP_TRUE;
                        as.o_pagewidth = 256;
                    }
                    if (tolower(argv[j][2]) == 't')
                    {
                        as.tabbed = BP_TRUE;
                    }
                    as.o_show_listing = BP_TRUE;
                    break;
					
                case 'o':
                    /* output file */
                    p = &argv[j][2];
					strncpy(as.object_name, p, FNAMESIZE - 1);
					as.object_output = BP_TRUE;
                    break;
					
                case 'p':
                    /* parse only output */
                    as.o_do_parsing = BP_FALSE;
                    break;
					
                case 'q':
                    /* quiet mode */
                    as.o_quiet_mode = BP_TRUE;
                    break;
					
                case 's':
                    /* symbol table dump */
                    as.o_show_symbol_table = BP_TRUE;
                    break;
					
                case 't':
                    /* object type */
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
                    /* suppress errors and warnings */
                    as.SuppressFlag = BP_TRUE;
                    break;
					
                case 'y':
                    /* cycle count (sort of works) */
                    as.f_count_cycles = BP_TRUE;
                    break;
					
                case 'z':
                    as.Opt_C = BP_FALSE;
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

	mamou_assemble(&as);
	
	
	/* 5. Return status. */
	
	return 0;
}



/*
 * mamou_assemble:
 *
 * This is the heart of the assembly process.
 */

void mamou_assemble(assembler *as)
{
	/* 1. Initialize the assembler for the first pass. */
		
	as->pass = 1;
	
    mamou_initialize(as);


	/* 2. For each file we have to assemble... */
	
    for (as->current_filename_index = 0; as->current_filename_index < as->file_index; as->current_filename_index++)
    {
		struct filestack root_file;
		
		
		/* 1. Set up the structure. */
		
		as->current_file = &root_file;
		
		strncpy(root_file.file, as->file_name[as->current_filename_index], FNAMESIZE);
		root_file.current_line = 0;
		root_file.num_blank_lines = 0;
		root_file.num_comment_lines = 0;
		root_file.end_encountered = BP_FALSE;
		
		
		/* 2. Open a path to the file. */
		
        if (_coco_open(&(root_file.fd), root_file.file, FAM_READ) != 0)
        {
            printf("mamou: can't open %s\n", root_file.file);

            return;
        }


		/* 3. Make the first pass. */
		
		mamou_pass(as);

		
		/* 4. Close the file. */
		
		_coco_close(root_file.fd);
    }


	/* 3. If the assembly pass above yielded no errors... */
	
    if (as->num_errors == 0)
    {
		/********** SECOND PASS **********/
		
		
		/* 1. Increment the pass. */
		
        as->pass++;
		
		
		/* 2. Re-initialize the assembler. */
		
        mamou_initialize(as);


		/* 3. If this is a DECB .BIN file, emit the initial header. */
		
		if (as->o_asm_mode == ASM_DECB && as->orgs[as->current_org].size > 0)
		{
			decb_header_emit(as, as->orgs[as->current_org].org, as->orgs[as->current_org].size);
		}
		
		as->current_org++;
		
		
		/* 4. Walk the file list again... */
		
        for (as->current_filename_index = 0; as->current_filename_index < as->file_index; as->current_filename_index++)
        {
			struct filestack root_file;
			
			
			/* 1. Set up the structure. */
			
			as->current_file = &root_file;
			
			strncpy(root_file.file, as->file_name[as->current_filename_index], FNAMESIZE);
			root_file.current_line = 0;
			root_file.num_blank_lines = 0;
			root_file.num_comment_lines = 0;
			root_file.end_encountered = BP_FALSE;
			
			
			/* 2. Open a path to the file. */
			
			if (_coco_open(&(root_file.fd), root_file.file, FAM_READ) != 0)
			{
				printf("mamou: can't open %s\n", root_file.file);
				
				return;
			}
			
			
			/* 3. Make the first pass. */
			
			mamou_pass(as);
			
			
			/* 4. Close the file. */
			
			_coco_close(root_file.fd);
        }
		

		/* 5. Emit Disk BASIC trailer. */
		
		if (as->o_asm_mode == ASM_DECB)
		{
			decb_trailer_emit(as, 0xEEAA);
		}

		
		/* 6. Do we show the symbol table? */
		
        if (as->o_show_symbol_table == BP_TRUE)
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


    if ((as->o_quiet_mode == BP_FALSE) && (as->o_format_only == BP_FALSE))
    {
        print_summary(as);
    }


	/* Terminate the forward reference file. */

    fwd_deinit(as);


    /* Added to remove an object if there were errors in the assembly - BGP 2002/07/25 */

    if (as->num_errors != 0)
    {
        _coco_delete(as->object_name);
    }


	/* 1. Deinitialize the assembler. */
	
    mamou_deinitialize(as);


	/* Return. */

    return;
}



static void mamou_initialize(assembler *as)
{
    if (as->o_debug)
    {
        printf("Initializing for pass %u\n", (unsigned int)as->pass);
    }

	if (as->pass == 1)
	{
		/* Pass 1 initialization. */

		as->num_errors				= 0;
		as->cumulative_blank_lines  = 0;
		as->cumulative_comment_lines  = 0;
		as->cumulative_total_lines  = 0;
		as->data_counter			= 0;
		as->program_counter			= 0;
		as->pass					= 1;
		as->Ctotal					= 0;
		as->f_new_page				= BP_FALSE;
		as->use_depth				= 0;
		
		as->conditional_stack_index = 0;
		as->conditional_stack[0]	= 1;

		as->current_org = 0;
		as->orgs[as->current_org].org = 0;
		as->orgs[as->current_org].size = 0;

		if (as->object_name[0] != EOS)
		{
			if (_coco_create(&(as->fd_object), as->object_name,
				FAM_READ | FAM_WRITE,
				FAP_READ | FAP_WRITE | FAP_PREAD) != 0)
			{
				fatal("Can't create object file");
			}
		}


		fwd_init(as);		/* forward ref init */
		local_init();		/* target machine specific init. */
		env_init(as);		/* environment variables init. */
	}
	else
	{
		/* Pass 2 initialization. */

		as->cumulative_blank_lines  = 0;
		as->cumulative_comment_lines  = 0;
		as->cumulative_total_lines  = 0;
		as->data_counter    = 0;
		as->program_counter	= 0;
		as->DP				= 0;
		as->E_total			= 0;
		as->P_total			= 0;
		as->Ctotal			= 0;
		as->f_new_page		= BP_FALSE;
		as->use_depth		= 0;
		
		as->current_org		= 0;
		
		fwd_reinit(as);

		as->conditional_stack_index = 0;
		as->conditional_stack[as->conditional_stack_index] = 1;
	}
	

    return;
}



static void mamou_deinitialize(assembler *as)
{
    if (as->o_debug)
    {
        printf("Deinitializing\n");
    }
	

    return;
}



void mamou_pass(assembler *as)
{
	int size = MAXBUF - 1;
	BP_char		input_line[1024];
	
	
	/* 1. Show debug output. */

    if (as->o_debug)
    {
        printf("\n------");
        printf("\nPass %u", (unsigned int)as->pass);
        printf("\n------\n");
    }

	
	/* 2. While we haven't encountered 'end' and there are more lines to read... */
	
	while (as->current_file->end_encountered == BP_FALSE && _coco_readln(as->current_file->fd, input_line, &size) == 0)
	{
		char *p = strchr(input_line, 0x0D);
//		BP_int32			line_type;
		struct source_line		line;
		
		
		as->line = &line;
		
			
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
		as->f_new_page = BP_FALSE;
	
//		line_type = mamou_parse_line(as, input_line);
		mamou_parse_line(as, input_line);
		
		if (as->line->type == LINETYPE_SOURCE && as->o_do_parsing == BP_TRUE)
		{
			process(as);
		}
		else
		{
			print_line(as, 0, ' ', 0);

			if (as->line->type == LINETYPE_BLANK)
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

		as->cumulative_total_lines++;

	
		as->P_total = 0;	/* reset byte count */

		as->cumulative_cycles = 0;	/* and per instruction cycle count */
	}

    f_record(as);
}



/*
 * mamou_parse_line: split input line into label, op and operand
 *
 * Returns: 0 if a blank line, 1 if a comment, 2 if an actual line
 */
 
void mamou_parse_line(assembler *as, BP_char *input_line)
{
    register char *ptrfrm = input_line;
    char *ptrto = as->line->label;
    static char hold_lbl[80];
    static int cont_prev = 0;
    register struct oper *i;


	/* 1. Initialize line structure. */
	
	as->line->has_warning = BP_FALSE;
	as->line->optr = as->line->Op;
	as->line->force_word = BP_FALSE;
	as->line->force_byte = BP_FALSE;
    *as->line->label = EOS;
    *as->line->Op = EOS;
    *as->line->operand = EOS;
    *as->line->comment = EOS;


	/* 2. First, check to see if this line has the 5 byte numerical field that
	 * is associated with EDTASM source files.
	 */
	
	if (
		numeric(*(ptrfrm + 0)) == BP_TRUE &&
		numeric(*(ptrfrm + 1)) == BP_TRUE &&
		numeric(*(ptrfrm + 2)) == BP_TRUE &&
		numeric(*(ptrfrm + 3)) == BP_TRUE &&
		numeric(*(ptrfrm + 4)) == BP_TRUE &&
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

		as->line->type = LINETYPE_BLANK;

		return;
	}
	
	
	/* 4. Reanchor pointer to start of line. */
	
	ptrfrm = input_line;
	
    if (*ptrfrm == '*' || *ptrfrm == '\n' ||
        *ptrfrm == ';' || *ptrfrm == '#')
    {
        strcpy(as->line->comment, input_line);
        ptrto = as->line->comment;

        while (!eol(*ptrto))
        {
            ptrto++;
        }

        *ptrto = EOS;

		as->line->type = LINETYPE_COMMENT;

        return;
    }

    while (delim(*ptrfrm) == BP_FALSE)
    {
        *ptrto++ = *ptrfrm++;
    }

    if (ptrto > as->line->label && *--ptrto != ':')
    {
        ptrto++;     /* allow trailing : */
    }

    *ptrto = EOS;

    ptrfrm = skip_white(ptrfrm);

    ptrto = as->line->Op;

    while (delim(*ptrfrm) == BP_FALSE)
    {
        *ptrto++ = mapdn(*ptrfrm++);
    }

    *ptrto = EOS;

    ptrfrm = skip_white(ptrfrm);

    /* determine whether this op code has a parameter */
    i = mne_look(as, as->line->Op);
    if (i != NULL)
    {
        if ((i->class == PSEUDO && i->cycles & 1 == 1) ||
            (i->class != INH && i->class != P2INH && i->class != P3INH) )
        {
            ptrto = as->line->operand;
            if (i->class == PSEUDO && i->cycles == 0x2)
            {
                char fccdelim;

                /* delimiter pseudo op (fcs/fcc) */
                fccdelim = *ptrfrm;
                do
                {
                    *ptrto++ = *ptrfrm++;
                } while (*ptrfrm != EOS && *ptrfrm != fccdelim);
                *ptrto++ = *ptrfrm++;
            }
            else if (i->class == PSEUDO && i->cycles == 0x4)
            {
                /* pseudo op has spaces in operand */
                do
                {
                    *ptrto++ = *ptrfrm++;
                } while (*ptrfrm != EOS && !eol(*ptrfrm));
            }
            else
            {
                while (delim(*ptrfrm) == BP_FALSE)
                {
                    *ptrto++ = *ptrfrm++;
                }
            }
            *ptrto = EOS;

            ptrfrm = skip_white(ptrfrm);
        }
    }

    ptrto = as->line->comment;
    while (!eol(*ptrfrm))
    {
        *ptrto++ = *ptrfrm++;
    }
    *ptrto = EOS;

/* Below added by GFC 8/30/94 */

    if (cont_prev)
    {
            cont_prev = 0;
            strcpy(as->line->label, hold_lbl);
    }

    if (as->line->Op[0] == ';')
    {
        if (as->line->label[0] == '\0')
        {
			as->line->type = LINETYPE_COMMENT;

            return;
        }
        else	/* save this label for the next mamou_parse_line() */
        {
            strcpy(hold_lbl, as->line->label);
            cont_prev = 1;

			as->line->type = LINETYPE_COMMENT;
			
            return;
        }
    }


    if (as->o_debug)
    {
        printf("\n");
        printf("Label      %s\n", as->line->label);
        printf("Op         %s\n", as->line->Op);
        printf("Operand    %s\n", as->line->operand);
    }


	as->line->type = LINETYPE_SOURCE;
	
	return;
}


/*
 *	process --- determine mnemonic class and act on it
 */
void process(assembler *as)
{
    register struct oper *i;

    as->old_program_counter = as->program_counter;		/* setup `old' program counter */
    as->line->optr = as->line->operand; 	/* point to beginning of operand field */

    if (as->conditional_stack[as->conditional_stack_index] == 0)
    {
        /* we should ignore this line unless it's an endc */
        i = mne_look(as, as->line->Op);
        if (i != NULL && i->class == PSEUDO)
        {
            i->func(as);
        }
        return;
    }

    if (*as->line->Op == EOS)
    {
        /* no mnemonic */
        if (*as->line->label != EOS)
        {
            symbol_add(as, as->line->label, as->program_counter, 0);
            print_line(as, 0, ' ', 0);
        }
    }
    else if ((i = mne_look(as, as->line->Op)) == NULL)
    {
        error(as, "Unrecognized Mnemonic");
    }
    else if (i->class == PSEUDO)
    {
        i->func(as);
        if (as->E_total >= E_LIMIT)
        {
            f_record(as);
        }

    }
    else
    {
        if (*as->line->label)
        {
            symbol_add(as, as->line->label, as->program_counter, 0);
        }
        if (as->f_count_cycles)
        {
            as->cumulative_cycles = i->cycles;
            if (as->o_h6309 == BP_TRUE)
            {
                as->cumulative_cycles--;
            }
        }
        i->func(as, i->opcode);
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


void init_globals(assembler *as)
{
    as->output_type = OUTPUT_BINARY;
	as->current_file = NULL;
    as->num_errors = 0;		/* total number of errors       */
    as->program_counter = 0;			/* Program Counter              */
    as->DP = 0;			/* Direct Page                  */
    as->num_warnings = 0;		/* total warnings               */
    as->old_program_counter = 0;		/* Program Counter at beginning */
	as->object_output = BP_FALSE;
    as->last_symbol = 0;		/* result of last symbol_find        */

    as->pass = 1;		/* Current pass #               */
    as->Ffn = 0;		/* forward ref file #           */
    as->F_ref = 0;		/* next line with forward ref   */
    as->arguments = 0;		/* pointer to file names        */

    as->E_total = 0;		/* total # bytes for one line   */
    as->E_bytes[0] = 0;		/* Emitted held bytes           */
    as->E_pc = 0;		/* Pc at beginning of collection*/

    as->P_force = 0;		/* force listing line to include Old_pc */
    as->P_total = 0;		/* current number of bytes collected    */
    as->P_bytes[0] = 0;		/* Bytes collected for listing  */

    as->cumulative_cycles = 0;		/* # of cycles per instruction  */
    as->Ctotal = 0;		/* # of cycles seen so far */
    as->f_new_page = BP_FALSE;		/* new page flag */
    as->page_number = 2;		/* page number */
    as->o_show_cross_reference = 0;		/* cross reference table flag */
    as->f_count_cycles = 0;		/* cycle count flag */
    as->Opt_C = BP_TRUE;		/* show conditionals in listing */
    as->o_page_depth = 66;
    as->o_show_error = BP_TRUE;
    as->Opt_F = BP_FALSE;
    as->Opt_G = BP_FALSE;
    as->o_show_listing = 0;		/* listing flag 0=nolist, 1=list*/
    as->o_asm_mode			= ASM_OS9;
    as->Opt_N = BP_FALSE;
    as->o_quiet_mode = BP_FALSE;
    as->o_show_symbol_table = BP_FALSE;		/* symbol table flag, 0=no symbol */
    as->o_pagewidth = 80;
    as->o_debug = 0;		/* debug flag */
    as->object_name[0] = EOS;
    as->bucket = NULL;
    as->do_module_crc = BP_FALSE;
    as->_crc[0] = 0xFF;
    as->_crc[1] = 0xFF;
    as->_crc[2] = 0xFF;
    as->o_do_parsing = BP_TRUE;
    as->include_index = 0;
    as->file_index = 0;
    as->current_line = 0;
    as->current_page = 1;
    as->header_depth = 3;
    as->footer_depth = 3;
    as->o_asm_mode = ASM_OS9;
    as->SuppressFlag = 0;	/* suppress errors and warnings */
    as->tabbed = 0;
    as->o_h6309 = BP_FALSE;		/* assume 6809 mode only */
	as->code_bytes = 0;

    return;
}
