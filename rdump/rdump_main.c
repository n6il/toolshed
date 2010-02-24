/*
 * rdump:  prints formatted dump of .r and .l files rdump [opts] <file>[
 * <file>] [opts] options: -g - add global definition info -r - add reference
 * info -o - add reference and local offset info -a - all of the above
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rof.h>
#include "hd6309.h"

typedef unsigned short u16;	/* Little-endian coco int */

void pass1(void);
void showhead(void);
void showfoot(void);
void Disasm(void);
void showrefs(void);
void showlcls(void);
void showglobs(void);
void getname(char *s);
unsigned int getwrd(FILE * fp);
int help(void);
void ftext(char c, int ref);
void ferr(char *s);
char *get_global( int pc, unsigned char flag_on, unsigned char flag_off );
char *get_external_ref( int pc, unsigned char flag_on, unsigned char flag_off, unsigned char *out_flag );
char *get_label( int pc, unsigned char flag_on, unsigned char flag_off );
char *remove_colon( char *s );
void add_code_label( int address );

#define MAXSOURCE 20
#define puts(s) fputs(s,stdout)
#define mc(c) ((c)&0xff)
#define DEF 1
#define REF 2

char           *snames[MAXSOURCE],
               *fname;
u16             scount;
u16             gflag,
                rflag,
                dflag,
                oflag;
binhead         hd;
FILE           *in;
long			global_symbol_table;
long            object_code;
long			rof_start;
long			code_size, init_data_size, init_dpdata_size,
                uninit_data_size, uninit_dpdata_size;
                
unsigned int    o9_int();
unsigned int    getwrd();
int             help();
char symbols[8][40];
int symPtr = 0;

typedef struct
{
	void *next;
	int address;
	int bytes_used;
	char line[40];
} text_line;

typedef struct
{
	void *next;
	int address;
} code_label;

code_label *cl_list;

int main(argc, argv)
	int             argc;
	char          **argv;
{
	register char  *p;

	while (--argc > 0)
	{
		if (*(p = *++argv) == '-')
		{
			while (*++p)
				switch (*p)
				{
					case 'g':
						gflag = 1;
						break;

					case 'r':
						rflag = 1;
						break;

					case 'o':
						oflag = 1;
						break;

					case 'd':
						dflag = 1;
						break;

					case 'a':
						gflag = rflag = oflag = 1;
						break;

					case '?':
						help();
						exit(0);

					default:
						fprintf(stderr, "unknown option -%c\n", *p);
						exit(1);
				}
		}
		else
		{
			if (scount == MAXSOURCE)
			{
				fprintf(stderr, "too many source files\n");
				exit(1);
			}
			snames[scount++] = *argv;
		}
	}
	
	rof_start = 0;
	code_size = init_data_size = init_dpdata_size = uninit_data_size = uninit_dpdata_size = 0;

	pass1();

	return 0;
}



void pass1(void)
{
	int             count, i;
	char module_name[256];
	unsigned char flag;
	
	if (scount == 0)
		return;
	for (count = 0; count < scount; ++count)
	{
		fname = snames[count];
		if ((in = fopen(fname, "r")) == NULL)
		{
			printf("can't open '%s'\n", fname);
			continue;
		}
		for (;;)
		{
			char c1;

			if (fread(&hd, sizeof(hd), 1, in) == 0)
				break;
			if (hd.h_sync != ROFSYNC)
			{
				printf("'%s' is not a relocatable module\n", fname);
				break;
			}

			showhead();

			global_symbol_table = ftell( in );
			showglobs();

			object_code = ftell( in );
			if( dflag == 1 )
			{
				/* Build psect */
				
				printf( "\nDisassembly:\n                 psect " );
				fseek(in, rof_start + sizeof( binhead ), SEEK_SET);
				getname( module_name );
				printf( "%s, $%x, $%x, %d, %d, %s\n", module_name,
				o9_int(hd.h_tylan)>>8,
				o9_int(hd.h_tylan)&0x00ff,
				o9_int(hd.h_edit),
				o9_int(hd.h_stack),
				remove_colon(get_label( o9_int(hd.h_entry), CODENT, CONENT|SETENT )) );
				
				/* Build vsect dp */
				if( (o9_int(hd.h_ddata) > 0) || (o9_int(hd.h_dglbl) > 0) )
				{
					unsigned int c;
					char *label, *ext_ref;

					printf( "\n                 vsect dp\n" );
					for( i=0; i<o9_int(hd.h_ddata); i++ )
					{
						
						/* Get global symbol or local reference*/
						label = get_label( i, DIRENT|INIENT, CODENT );
						
						/* Get named external reference or global symbol or local label */
						ext_ref = get_external_ref( i, DIRLOC, CODLOC, &flag );

						if( label == ext_ref )
							printf( "oops!\n" );
							
						fseek( in, object_code + o9_int(hd.h_ocode) + o9_int(hd.h_data) + i, SEEK_SET );
						if( strcmp( ext_ref, "" ) == 0 )
						{
							c = getc( in );
							printf( "%04x   %02x       %s fcb $%x\n", i, c, label, c );
						}
						else
						{
							if( flag & LOC1BYT )
							{
								c = getc( in );
								printf( "%04x   %02x       %s fcb %s\n", i, c, label, ext_ref );
							}
							else
							{
								c = getwrd( in );
								printf( "%04x %04x       %s fdb %s\n", i, c, label, ext_ref );
								i++;
							}
						}
					}
					
					for( i=0; i<o9_int(hd.h_dglbl); i++ )
					{
						/* Check for global or local symbol */
						label = get_label( i, DIRENT, INIENT|CODENT );

						printf( "%04x            %s rmb 1\n", i+o9_int(hd.h_ddata), label );
					}
					
					printf( "                 endsect\n" );
				}
				
				/* Build vsect */
				if( (o9_int(hd.h_data) > 0) || (o9_int(hd.h_glbl) > 0) )
				{
					unsigned int c;
					char *label, *ext_ref;

					printf( "\n                 vsect\n" );
					for( i=0; i<o9_int(hd.h_data); i++ )
					{
						
						/* Get global symbol or local reference*/
						label = get_label( i, INIENT, DIRENT );

						/* Get named external reference or local label */
						
						ext_ref = get_external_ref( i, 0, CODLOC|DIRLOC, &flag );
						
						fseek( in, object_code + o9_int(hd.h_ocode) + i, SEEK_SET );
						if( strcmp( ext_ref, "" ) == 0 )
						{
							c = getc( in );
							printf( "%04x   %02x       %s fcb $%x\n", i, c, label, c );
						}
						else
						{
							if( flag & LOC1BYT )
							{
								c = getc( in );
								printf( "%04x   %02x       %s fcb %s\n", i, c, label, ext_ref );
							}
							else
							{
								c = getwrd( in );
								printf( "%04x %04x       %s fdb %s\n", i, c, label, ext_ref );
								i++;
							}
						}
					}
					
					for( i=0; i<o9_int(hd.h_glbl); i++ )
					{
						/* Check for global or local symbol */
						label = get_label( i, 0, DIRENT|INIENT|CODENT );

						printf( "%04x            %s rmb 1\n", i+o9_int(hd.h_data), label );
					}
					
					printf( "                 endsect\n" );
				}
				
				/* Disasemble object code */
				fseek(in, object_code, SEEK_SET);
				Disasm();

				printf( "                 endsect\n" );
			}

			fseek(in, object_code + 
			     o9_int(hd.h_ocode) +
				 o9_int(hd.h_ddata) +
				 o9_int(hd.h_data), SEEK_SET);

			
			showrefs();
			showlcls();

			/* Some ROFs have two extra zero bytes at the end.  We check if that is the case -- BGP */
			c1 = fgetc(in);
			if (c1 == 0)
				c1 = fgetc(in);
			else
				ungetc(c1, in);  /* not a zero, put it back. */

			rof_start = ftell( in );
		}
		fclose(in);
	}
	
	showfoot();
}

/* Switch a Little-Endian number byte order to make it Big-Endian */

unsigned int o9_int(nbr)
u16 nbr;
{
#if defined(__BIG_ENDIAN__)
	return nbr;
#else
	return (((nbr & 0xff00) >> 8) + ((nbr & 0xff) << 8));
#endif
}



void showhead(void)
{
	int             c;

	puts("\nModule name: ");
	while ((c = getc(in)) != 0)
		putchar(c);
	if (ferror(in))
		ferr(fname);
	printf("\nTyLa/RvAt:   %02x/%02x\n", mc(o9_int(hd.h_tylan) >> 8), mc(o9_int(hd.h_tylan)));
	printf("Asm valid:   %s\n", hd.h_valid ? "No" : "Yes");
	printf("Create date: %.3s %2d, %4d %02d:%02d\n",
	       &("JanFebMarAprMayJunJulAugSepOctNovDec"[(mc(hd.h_date[1]) - 1) * 3]),
	       mc(hd.h_date[2]), 1900 + mc(hd.h_date[0]),
	       mc(hd.h_date[3]), mc(hd.h_date[4]));
	printf("Edition:     %2d\n", hd.h_edit);
	puts("  Section    Init Uninit\n");
	printf("   Code:     %04x\n", o9_int(hd.h_ocode));
	printf("     DP:       %02x   %02x\n", o9_int(hd.h_ddata),
	       o9_int(hd.h_dglbl));
	printf("   Data:     %04x %04x\n", o9_int(hd.h_data),
	       o9_int(hd.h_glbl));
	printf("  Stack:     %04x\n", o9_int(hd.h_stack));
	printf("Entry point: %04x\n", o9_int(hd.h_entry));
	
	code_size += o9_int(hd.h_ocode);
	init_data_size += o9_int(hd.h_data);
	init_dpdata_size += o9_int(hd.h_ddata);
	uninit_data_size += o9_int(hd.h_glbl);
	uninit_dpdata_size += o9_int(hd.h_dglbl);
}

void showfoot()
{
	printf("\nTotals:\n");
	puts("  Section    Init     Uninit\n");
	printf("   Code:     %08lx\n", code_size);
	printf("     DP:     %08lx %08lx\n", init_dpdata_size, uninit_dpdata_size);
	printf("   Data:     %08lx %08lx\n", init_data_size, uninit_data_size);
}


void showglobs(void)
{
	register unsigned count,
	                offset;
	char            sym[SYMLEN + 1],
	                flag;

	count = getwrd(in);	/* global def count */
	if (gflag)
		printf("\n%u global symbols defined:\n", count);
	while (count--)
	{
		getname(sym);
		flag = getc(in);
		offset = getwrd(in);
		if (gflag)
		{
			printf(" %9s %04x ", sym, offset);
			ftext(flag, DEF);
		}
	}


	return;
}



void getname(char *s)
{
	while (feof(in) == 0 && (*s++ = getc(in)) != 0);
	*s = '\0';
	if (ferror(in))
		ferr(fname);
}



void ftext(char c, int ref)
{
	printf("(%02x) ", mc(c));
	if (ref & REF)
	{
		if (c & CODLOC)
		{
			puts("in code");
		}
		else
			puts(c & DIRLOC ? "in dp data" : "in non-dp data");
		puts(c & LOC1BYT ? "/byte" : "/word");
		if (c & NEGMASK)
			puts("/neg");
		if (c & RELATIVE)
			puts("/pcr");
	}
	if (ref & DEF)
	{
		if (ref & REF)
			puts(" - ");
		if (c & CODENT)
		{
			if( c & CONENT )
				puts( "a constant (csect rmb)" );
			else if( c & SETENT )
				puts( "a constant (set)" );
			else
				puts("to code");
		}
		else
		{
			puts(c & DIRENT ? "to dp" : "to non-dp");
			puts(c & INIENT ? " data" : " bss");
		}
	}
	putchar('\n');

	return;
}



void showrefs(void)
{
	register unsigned count,
	                rcount;
	def_ref         ref;
	char            sym[SYMLEN + 1];
	int             fflag;

	count = getwrd(in);
	if (rflag)
		printf("\n%u external references:\n", count);
	while (count--)
	{
		getname(sym);
		rcount = getwrd(in);
		if (rflag)
			printf(" %9s ", sym);
		fflag = 0;
		while (rcount--)
		{
			fread(&ref.r_flag, sizeof(ref.r_flag), 1, in);
			fread(&ref.r_offset, sizeof(ref.r_offset), 1, in);
			if (ferror(in))
				ferr(fname);
			if (rflag && oflag)
			{
				if (fflag)
					puts("           ");
				else
					fflag = 1;
				printf("%04x ", o9_int(ref.r_offset));
				ftext(ref.r_flag, REF);
			}
		}
		if (rflag && !oflag)
			putchar('\n');
	}
	
	
	return;
}



void showlcls(void)
{
	register unsigned count;
	def_ref         ref;


	count = getwrd(in);
	if (oflag)
		printf("\n%u local references\n", count);
	while (count--)
	{
		fread(&ref.r_flag, sizeof(ref.r_flag), 1, in);
		fread(&ref.r_offset, sizeof(ref.r_offset), 1, in);
		if (ferror(in))
			ferr(fname);
		if (oflag)
		{
			printf("   %04x ", o9_int(ref.r_offset));
			ftext(ref.r_flag, DEF | REF);
		}
	}
	
	
	return;
}



void Disasm(void)
{
	unsigned char *buffer;
	int i, x;
	text_line *start, *cur_line;
	
	buffer = malloc( o9_int(hd.h_ocode) );
	
	if( buffer == NULL )
	{
		fprintf( stderr, "Not enough memory to disasemble\n\b" );
		return;
	}
	
	fread( buffer, o9_int(hd.h_ocode), 1, in );
	i=0;
	
	start = malloc( sizeof( text_line ) );
	if( start == NULL )
	{
		fprintf( stderr, "Disasm: out of memory\n" );
		exit( 1 );
	}
	
	/* Initialize code labels linked list */
	
	cl_list = NULL;
	
	cur_line = start;
	
	while( i < o9_int(hd.h_ocode) )
	{
		text_line *next_line;
		
		cur_line->next = NULL;
		cur_line->address = i;
		cur_line->line[0] = '\0';
		cur_line->bytes_used = Dasm6309 (cur_line->line, i, buffer, 0l);

		next_line = malloc( sizeof( text_line ) );
		if( next_line == NULL )
		{
			fprintf( stderr, "Disasm: out of memory\n" );
			exit( 1 );
		}
	
		i += cur_line->bytes_used;
		cur_line->next = next_line;
		cur_line = next_line;	

	}
	
	for( cur_line = start; cur_line->next != NULL; cur_line = cur_line->next )
	{
		printf( "%4.4X ", cur_line->address );
		
		x = 0;
		while( x<5 )
		{
			if( x<cur_line->bytes_used )
				printf( "%2.2X", buffer[cur_line->address+x] );
			else
				printf( "  " );
			
			x++;
		}
		
		/* Write global label if this PC has one */
		if( strcmp( get_label( cur_line->address, CODENT, CONENT|SETENT ), "" ) != 0 )
			printf( " %s" , get_label( cur_line->address, CODENT, CONENT|SETENT ));
		else
		{
			/* Else see if we need a local label */
			code_label *walk;
			
			for( walk = cl_list; walk != NULL; walk = walk->next )
			{
				if( walk->address == cur_line->address )
				{
					printf( " _%04x", cur_line->address );
					break;
				}
			}
		}
		
		/* Write disasembled line */
		printf( "  %s\n", cur_line->line );
	}

	free( buffer );
	
	
	return;
}

void add_code_label( int address )
{
	code_label *walk, *new;
	
	for( walk = cl_list; walk != NULL; walk = walk->next )
	{
		if( walk->address == address )
			return;
	}

	/* Add address to begining */
	new = malloc( sizeof( code_label ) );

	if( new == NULL )
	{
		fprintf( stderr, "Out of memory: add_code_label\n" );
		exit(1);
	}

	if( cl_list == NULL )
		new->next = NULL;
	else
		new->next = cl_list;
		
	cl_list = new;
	cl_list->address = address;
}

unsigned int
                getwrd(FILE * fp)
{
	unsigned        Msb,
	                Lsb;
	unsigned int    nbr;

	Msb = getc(fp);
	Lsb = getc(fp);
	nbr = Msb << 8 | Lsb;

	return nbr;
}



int             help(void)
{
	fprintf(stderr, "Usage: rdump <opts> object_file [object_file ...] <opts>\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "   -d             disassemble object code\n");

	return 0;
}


void ferr(char *s)
{
	fprintf(stderr, "error reading '%s'\n", s);
	
	exit(1);
}

char *remove_colon( char *s )
{
	int l = strlen( s ) - 1;
	
	if( s[l] == ':' )
		s[l] = '\0';
	
	return s;
}

char *get_global( int pc, unsigned char flag_on, unsigned char flag_off )
{
	int count;
	int flag, offset;
	int pointer = symPtr++;
	
	symPtr &= 0x07;
	
	/* Position file pointer to start of Global Symbol Table */
	fseek( in, global_symbol_table, SEEK_SET );

	/* Get table size */
	count = getwrd( in );

	while (count--)
	{
		getname(symbols[pointer]);
		flag = getc(in);
		offset = getwrd(in);
		
		if( (offset == pc) && ((flag & flag_on)==flag_on) && ((~flag & flag_off)==flag_off) )
		{
			strcat( symbols[pointer], ":" );
			return symbols[pointer];
		}
	}
	
	return strcpy( symbols[pointer], "" );
}

char *get_label( int pc, unsigned char flag_on, unsigned char flag_off )
{
	int count;
	int flag, offset;
	long filepos;
	int pointer = symPtr++;
	
	symPtr &= 0x07;

	/* Check if global */
	strcpy( symbols[pointer], get_global( pc, flag_on, flag_off ) );
	if( strcmp(symbols[pointer], "" ) != 0 )
		return symbols[pointer];
		
	/* Spin past external ref table */
	
	fseek(in, object_code + 
		 o9_int(hd.h_ocode) +
		 o9_int(hd.h_ddata) +
		 o9_int(hd.h_data), SEEK_SET);
	
	count = getwrd( in );
	
	while( count-- )
	{	
		int refcount;
		
		getname( symbols[pointer] );
		refcount = getwrd( in );
		while( refcount-- )
		{
			flag = getc( in );
			offset = getwrd( in );
		}
	}
	
	/* Check if this is a local reference. */

	/* Get table size */
	count = getwrd( in );
	filepos = ftell( in ) - 3;
	while (count--)
	{
		int data;
		
		filepos += 3;
		fseek( in, filepos, SEEK_SET );
		flag = getc(in);
		offset = getwrd(in);

		/* Now seek to the proper data block */
		if( flag & CODLOC )
			fseek( in, object_code + offset, SEEK_SET);
		else if( flag & DIRLOC )
			fseek( in, object_code + o9_int(hd.h_ocode) + o9_int(hd.h_data) + offset, SEEK_SET);
		else
			fseek( in, object_code + o9_int(hd.h_ocode) + offset, SEEK_SET);
		
		if( flag & LOC1BYT )
			data = getc( in );
		else
			data = getwrd( in );

		if( ( (pc == data) && (flag & flag_on)==flag_on) && ((~flag & flag_off)==flag_off) )
		{
			if( flag & CODENT )
			{
				sprintf( symbols[pointer], "_%04x", data );

				return symbols[pointer];
			}
			else if( flag & DIRENT )
			{
				if( flag & INIENT )
				{
					sprintf( symbols[pointer], "di%02x", data );
						
					return symbols[pointer];
				}
				else
				{
					sprintf( symbols[pointer], "du%02x", data );
					return symbols[pointer];
				}
			}
			else
			{
				if( flag & INIENT )
				{
					sprintf( symbols[pointer], "i%04x", data );
					return symbols[pointer];
				}
				else
				{
					sprintf( symbols[pointer], "u%04x", data );
					return symbols[pointer];
				}
			}
		}
	}
					
	return strcpy( symbols[pointer], "" );
}

char *get_external_ref( int pc, unsigned char flag_on, unsigned char flag_off, unsigned char *out_flag )
{
	int count, refcount;
	unsigned char flag;
	int offset;
	long filepos;
	int pointer = symPtr++;
	
	symPtr &= 0x07;
	
	/* forward to external ref table */
	fseek(in, object_code + 
		 o9_int(hd.h_ocode) +
		 o9_int(hd.h_ddata) +
		 o9_int(hd.h_data), SEEK_SET);
	
	count = getwrd( in );
	
	while( count-- )
	{
		getname( symbols[pointer] );
		refcount = getwrd( in );
		while( refcount-- )
		{
			flag = getc( in );
			offset = getwrd( in );

			if( (offset == pc) && ((flag & flag_on)==flag_on) && ((~flag & flag_off)==flag_off) )
			{
				if( out_flag )
					*out_flag = flag;
				return symbols[pointer];
			}
		}
	}
	
	/* Check if this is a local reference. */

	/* Get table size */
	count = getwrd( in );
	filepos = ftell( in ) - 3;
	while (count--)
	{
		int value;
		
		filepos += 3;
		fseek( in, filepos, SEEK_SET );
		flag = getc(in);
		offset = getwrd(in);
		if( out_flag )
			*out_flag = flag;

		if( (pc == offset) && ((flag & flag_on)==flag_on) && ((~flag & flag_off)==flag_off) )
		{
			/* Now seek to the proper data block */
			if( flag & CODLOC )
				fseek( in, object_code + offset, SEEK_SET);
			else if( flag & DIRLOC )
				fseek( in, object_code + o9_int(hd.h_ocode) + o9_int(hd.h_data) + offset, SEEK_SET);
			else
				fseek( in, object_code + o9_int(hd.h_ocode) + offset, SEEK_SET);
			
			if( flag & LOC1BYT )
				value = getc( in );
			else
				value = getwrd( in );
			
			/* Check if label corresponds to a global symbol */
			strcpy( symbols[pointer], remove_colon( get_label( value, flag & 0x07, ~(flag & 0x07) ) ) );
			if( strcmp( symbols[pointer], "" ) != 0 )
				return symbols[pointer];
			
			/* no global symbol, create a local symbol */
			
			if( flag & CODENT )
			{
				sprintf( symbols[pointer], "_%04x", value );
				return symbols[pointer];
			}
			else if( flag & DIRENT )
			{
				if( flag & INIENT )
				{
					sprintf( symbols[pointer], "di%02x", value );
					return symbols[pointer];
				}
				else
				{
					sprintf( symbols[pointer], "du%02x", value+o9_int(hd.h_ddata) );
					return symbols[pointer];
				}
			}
			else
			{
				if( flag & INIENT )
				{
					sprintf( symbols[pointer], "i%04x", value );
					return symbols[pointer];
				}
				else
				{
					sprintf( symbols[pointer], "u%04x", value );
					return symbols[pointer];
				}
			}
		}
	}

	return strcpy( symbols[pointer], "" );
}

