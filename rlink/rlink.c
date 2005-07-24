/**********************************************************************
 *
 * RLINK - Relocatable Linker
 *
 * Compatible with Microware's 'rlink' linker for the 6809.
 *
 * Written because Allen Huffman wouldn't get off his lazy
 * ass and send Tim those disks.
 *
 * Thanks for the motivation Allen!
 *
 **********************************************************************/

#include <stdio.h>
#ifdef UNIX
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#endif
#include "rlink.h"

unsigned o9_int();
unsigned getwrd();
int link();
int dump_rof_header();
char *flagtext();
unsigned adjoff();
int check_name();
int chk_dup();
int help();
int getname();
int rm_exref();
int asign_sm();
int dmp_ext();
unsigned getsym();

int ftext();
#define mc(c) ((c)&0xff)
#define DEF 1
#define REF 2


int compute_crc();
unsigned char _crc[3];
int buffer_crc();

#define MAX_RFILES	32
#define MAX_LFILES	32

struct ext_ref
{
	struct ext_ref *next;
	char name[SYMLEN+1];
};

struct exp_sym
{
	struct exp_sym *next;
	char name[SYMLEN+1];
	char flag;
	unsigned offset;
};

struct ob_files
{        
	struct ob_files	*next;
	char *filename;
	FILE *fp;
	long object;
	long locref;
	binhead hd;
	struct exp_sym *symbols;
	struct ext_ref *exts;
	char modname[MAXNAME+1];
	unsigned Code, IDat, UDat, IDpD, UDpD;
};

/* The 'main' function */
int main(argc,argv)
int argc;
char **argv;
{
	char *rfiles[MAX_RFILES];
	char *lfiles[MAX_LFILES];
	char *ofile, *modname;
	char *B09EntPt;
	int edition, extramem, okstatic;
	int printmap, printsym;
	int rfile_count, lfile_count, i;

	modname = NULL;
	ofile = NULL;
	B09EntPt = NULL;
	edition = 1;
	extramem = 0;
	printmap = 0;
	printsym = 0;
	okstatic = 0;
	
	rfile_count = 0;
	lfile_count = 0;

	/* Parse options */

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			/* option -- process it */
			switch (argv[i][1])
			{
				case 'e':
				case 'E':
					/* Edition */
					{
						char *p = &argv[i][2];

						if (argv[i][2] == '=')
						{
							p++;
						}

						edition = atoi(p);
					}
					break;

				case 'M':
					/* Extra Memory */
					{
						char *p = &argv[i][2];

						if (argv[i][2] == '=')
						{
							p++;
						}

						extramem = atoi(p);
						
						if( p[strlen(p)-1] == 'K' || p[strlen(p)-1] == 'k' )
							extramem *= 1024;
						else
							extramem *= 256;
							
					}
					break;

				case 'm':
					/* Print linkage map */
					{
						printmap = 1;
					}
					break;

				case 's':
					/* Print symbol table */
					{
						printsym = 1;
					}
					break;

				case 'n':
					/* Name */
					{
						char *p = &argv[i][2];

						if (argv[i][2] == '=')
						{
							p++;
						}

						modname = p;
					}
					break;

				case 'o':
					/* Output object file */
					{
						char *p = &argv[i][2];

						if (argv[i][2] == '=')
						{
							p++;
						}

						if (ofile == NULL)
						{
							ofile = p;
							if( modname == NULL )
							{
								/* Set the module name */
								modname = basename( p );
							}
						}
						else
						{
							fprintf(stderr, "linker fatal: -o option already specified\n");
							exit(1);
						}
					}
					break;

				case 'l':
					/* Library */
					{
						char *p = &argv[i][2];

						if (argv[i][2] == '=')
							p++;

						lfiles[lfile_count] = p;
						lfile_count++;
						if( lfile_count > MAX_LFILES )
						{
							fprintf(stderr, "linker fatal: To many library files\n" );
							exit(1);
						}
					}
					break;

				case 'b':
					/* BASIC09 Entry point */
					{
						char *p = &argv[i][2];
						
						if( argv[i][2] == '=' )
							p++;
						
						if( B09EntPt == NULL )
							B09EntPt = p;
						else
						{
							fprintf(stderr, "linker fatal: -b option already specified\n" );
							exit(1);
						}
					}
					break;
					
				case 't':
					/* Allow static data in BASIC09 modules */
					{
						okstatic = 1;
					}
					break;

				case '?':
					help();
					exit(0);

				default:
					fprintf(stderr, "linker fatal: unknown option %c in %s\n", argv[i][1], argv[i]);
					exit(1);
			}
		}
		else
		{
			/* file -- add it to the list */
			rfiles[rfile_count] = argv[i];
			rfile_count++;
			if( rfile_count > MAX_RFILES )
			{
				fprintf(stderr, "linker fatal: To many ROF files\n" );
				exit(1);
			}
		}
	}

	if( ofile == NULL )
	{
		fprintf( stderr, "linker fatal: no output file\n" );
		exit(1);
	}
	
	/* Call the function which does all the work! */

	return link(rfiles, rfile_count, lfiles, lfile_count, ofile, modname, edition, extramem, B09EntPt, printmap, printsym, okstatic);
}



int help()
{
	fprintf(stderr, "Usage: rlink <opts> source_file [source_file ...] <opts>\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "   -o[=]file      output object file\n");
	fprintf(stderr, "   -n[=]name      module name of the object file\n");
	fprintf(stderr, "   -l[=]path      additional library\n");
	fprintf(stderr, "   -E[=]edition   edition number in the module\n");
	fprintf(stderr, "   -M[=]size[K]   additional number of pages [or kilobytes] of memory\n");
	fprintf(stderr, "   -m             print the linkage map\n");
	fprintf(stderr, "   -s             print the symbol table\n");
	fprintf(stderr, "   -b=ept         Make callable from BASIC09\n"); 
	fprintf(stderr, "   -t             allow static data to appear in BASIC09 module\n");

	return 0;
}



int link(rfiles, rfile_count, lfiles, lfile_count, ofile, modname, edition, extramem, B09EntPt, printmap, printsym, okstatic)
char *rfiles[];
int rfile_count;
char *lfiles[];
int lfile_count;
char *ofile;
char *modname, *B09EntPt;
int edition, extramem, printmap, printsym, okstatic;
{
	int	i;
	struct ob_files *ob_start, *ob_cur;
	struct exp_sym *es_cur;
	unsigned t_code = 0, t_idat = 0, t_udat = 0, t_idpd = 0, t_udpd = 0, t_stac = 0, t_dt = 0, t_dd = 0;	
	int	once = 0, dup_fnd = 0;
	FILE *ofp;
	unsigned headerParity, moduleSize, nameOffset, execOffset, dataSize;

	/* We have the ROF input files, the library input files and
	 * the output file. Now let's go to work and link 'em!
	 */
	if( rfile_count == 0 )
	{
		fprintf( stderr, "linker fatal: no files specified to link\n" );
		return 1;
	}
	
	for (i = 0; i < rfile_count; i++)
	{
		struct ob_files *ob_temp;
		unsigned count;
		
		ob_temp = malloc( sizeof( struct ob_files  ));
		
		if( ob_temp == NULL )
		{
			fprintf( stderr, "linker fatal: Out of memory\n" );
			return 1;
		}
		
		if( i == 0 )
		{
			ob_start = ob_temp;
			ob_cur = ob_start;
		}
		else
		{
			ob_cur->next = ob_temp;
			ob_cur = ob_temp;
		}
		
		ob_cur->next = NULL;
		ob_cur->filename = rfiles[i];
		ob_cur->symbols = NULL;
		ob_cur->exts = NULL;
		ob_cur->fp = fopen(rfiles[i], "r");

		if (ob_cur->fp != NULL)
			fread(&(ob_cur->hd), sizeof(binhead), 1, ob_cur->fp);
		else
		{
			fprintf(stderr, "linker error: cannot open file %s\n", rfiles[i]);
			return 1;
		}

		/* Accumulate offsets */
		t_code += ob_cur->hd.h_ocode;
		t_idat += ob_cur->hd.h_data;
		t_udat += ob_cur->hd.h_glbl;
		t_idpd += ob_cur->hd.h_ddata;
		t_udpd += ob_cur->hd.h_dglbl;
		t_stac += ob_cur->hd.h_stack;
		
		/* Check for validity of ROF file */
		
		if( ob_cur->hd.h_sync != ROFSYNC )
		{
			fprintf( stderr, "linker error: File %s is not an ROF file.\n", rfiles[i] );
			return 1;
		}
		
		if( B09EntPt != NULL )
		{
			if( i==0 ) /* First ROF file needs special header */
			{
				if( ob_cur->hd.h_tylan == 0 )
				{
					fprintf( stderr, "linker error: Initial ROF file (%s) must be type non-zero.\n", rfiles[i] );
					return 1;
				}
			}
			else
			{
				if( ob_cur->hd.h_tylan != 0 )
				{
					fprintf( stderr, "linker fatal: ROF file (%s) must be type zero\n", rfiles[i] );
					return 1;
				}
			}
		}
		
		if( ob_cur->hd.h_valid )
		{
			fprintf( stderr, "linker fatal: ROF file: %s must contain valid object code\n", rfiles[i] );
			return 1;
		}
		
		/* Get module name */
		getname( ob_cur->modname, ob_cur->fp ); 

		/* Add external symbols to linked list */
		
	     count=getwrd( ob_cur->fp );
		
	     while( count-- )
	     {
	     	struct exp_sym *es_temp = malloc( sizeof( struct exp_sym) );
	     	
	     	if( es_temp == NULL )
	     	{
				fprintf( stderr, "linker fatal: Out of memory\n" );
				return 1;
	     	}

	     	getname( es_temp->name, ob_cur->fp );
	     	es_temp->flag = getc(ob_cur->fp);
	     	es_temp->offset = getwrd(ob_cur->fp );

			dup_fnd += chk_dup( es_temp, ob_start, ob_cur->modname );

			es_temp->next = ob_cur->symbols;
			ob_cur->symbols = es_temp;
			
			/* Now find and remove this symbol from the external references table */
			rm_exref( es_temp->name, ob_start );

	     }
	     
		/* Record the position of the start of the object code */
		ob_cur->object = ftell( ob_cur->fp );
	     
		/* Grind past the object code and initialized data */

		fseek( ob_cur->fp, ob_cur->hd.h_ocode + ob_cur->hd.h_ddata + ob_cur->hd.h_data, 1 );
		
		/* Now add external references to linked list - only if they not already in the global list */
		
		count = getwrd( ob_cur->fp );
		
		while( count-- )
		{
			struct ext_ref *er_temp;
			
			er_temp = malloc( sizeof( struct ext_ref ) );
			getname( er_temp->name, ob_cur->fp );
			fseek( ob_cur->fp, getwrd(ob_cur->fp) * 3, 1 );
			
			/* Check if name is listed in globals */
			if( check_name( ob_start, er_temp->name ) )
			{
				/* Add name to unknown symbols */
				er_temp->next = ob_cur->exts;
				ob_cur->exts = er_temp;
			}
			else
			{
				free( er_temp );
			}
		}

		/* count up local references that need data-data or data-text adjustments */
		ob_cur->locref = ftell( ob_cur->fp );
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned char flag;
			unsigned offset;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{}
			else
			{
/*				printf( "mod: %s (%d) (.r) ",ob_cur->modname, count );*/
/* 				ftext(flag, DEF|REF); printf("\n" ); */

				if( flag & CODENT )
					t_dt++;
				else
					t_dd++;
			}
		}
	}
	
/* 	printf( "Starting library files\n" ); */
	
	/* Process library files to resolve remaining undefined symbols */

	for (i = 0; i < lfile_count; i++)
	{
		FILE *fp;
		unsigned modcount;
		
		modcount = 0;
		
		fp = fopen(lfiles[i], "r");
		
/* 		printf( "Doing file: %s\n", lfiles[i] ); */
		
		if (fp == NULL)
		{
			fprintf( stderr, "linker fatal: Library file %s could not be opened\n", lfiles[i] );
			return 1;
		}
		
		do
		{
			unsigned count, addrof;
			struct ob_files *ob_temp;
			
			modcount++;
			addrof = 0;
			ob_temp = malloc( sizeof( struct ob_files ));
			
			if( ob_temp == NULL )
			{
				fprintf( stderr, "linker fatal: Out of memory\n" );
				return 1;
			}
	
			ob_temp->next = NULL;
			ob_temp->filename = lfiles[i];
			ob_temp->fp = fp;
			ob_temp->symbols = NULL;
			ob_temp->exts = NULL;
			
			if( fread(&(ob_temp->hd), sizeof(binhead), 1, fp) == 0 )
			{
				break; /* All done */
			}
			
			if( ob_temp->hd.h_sync != ROFSYNC )
			{
				if( modcount == 1 )
				{
					fprintf( stderr, "linker fatal: '%s' is not a library file\n", lfiles[i] );
					return 1;
				}
				else
				{
					fprintf( stderr, "linker fatal: Module number %d in library %s is corrupt\n", modcount, lfiles[i] );
					return 1;
				}
			}
			
			getname( ob_temp->modname, fp );

/* 			printf( "Doing file: %s, mod %s\n", lfiles[i], ob_temp->modname ); */

			if( ob_temp->hd.h_tylan != 0 )
			{
				fprintf( stderr, "linker fatal: Library file %s, module %s must be type zero\n", rfiles[i], ob_temp->modname );
				return 1;
			}

			if( ob_temp->hd.h_valid )
			{
				fprintf( stderr, "linker fatal: Library file: %s, module %s must contain valid object code\n", rfiles[i], ob_temp->modname );
				return 1;
			}
			
			count=getwrd( fp );

			 while( count-- )
			 {
				struct exp_sym *es_temp;
				
				es_temp = malloc( sizeof( struct exp_sym) );
				
				if( es_temp == NULL )
				{
					fprintf( stderr, "linker fatal: Out of memory\n" );
					return 1;
				}
	
				getname( es_temp->name, fp );
				es_temp->flag = getc( fp );
				es_temp->offset = getwrd( fp );
				es_temp->next = NULL;
				
				/* Now find and remove this symbol from the external references table */
				addrof += rm_exref( es_temp->name, ob_start );
				
				if( addrof > 0 )
					dup_fnd += chk_dup( es_temp, ob_start, ob_temp->modname );

				es_temp->next = ob_temp->symbols;
				ob_temp->symbols = es_temp;
			 }
			 
			 /* Record the position of the start of the object code */
			ob_temp->object = ftell( fp );

			/* Grind past the object code and initialized data */
			fseek( fp, ob_temp->hd.h_ocode + ob_temp->hd.h_ddata + ob_temp->hd.h_data, 1 );
				
			 if( addrof > 0 )
			 {
			 	/* Library module is needed, add it to the list */
				/* Now add external references to linked list - only if they not already in the global list */
				
				count = getwrd( fp );
				
				while( count-- )
				{
					struct ext_ref *er_temp;
					
					er_temp = malloc( sizeof( struct ext_ref ) );
					getname( er_temp->name, fp );
					er_temp->next = NULL;
					fseek( fp, getwrd(fp) * 3, 1 );
					
					/* Check if name is listed in globals */
					if( check_name( ob_start, er_temp->name ) )
					{
						/* Add name to unknown symbols */
						er_temp->next = ob_temp->exts;
						ob_temp->exts = er_temp;
					}
					else
					{
						free( er_temp );
					}
				}

			 	ob_cur->next = ob_temp;

				t_code += ob_temp->hd.h_ocode;
				t_idat += ob_temp->hd.h_data;
				t_udat += ob_temp->hd.h_glbl;
				t_idpd += ob_temp->hd.h_ddata;
				t_udpd += ob_temp->hd.h_dglbl;
				t_stac += ob_temp->hd.h_stack;

			 	ob_cur = ob_temp;
#if 0
				printf( "\nNeeded %s, mod %s\n", ob_temp->filename, ob_temp->modname );
				dmp_ext( ob_start );
#endif
				/* count up local references that need data-data or data-text adjustments */
				ob_temp->locref = ftell( ob_temp->fp );
				count = getwrd( fp );
				while( count-- )
				{
					unsigned char flag;
					unsigned offset;
					
					flag = getc( fp );
					offset = getwrd( fp );
					
					if( flag & CODLOC )
					{}
					else
					{
						
/* 						printf( "mod: %s (%d) (.l) ",ob_temp->modname, count ); */
/* 						ftext(flag, DEF|REF); printf("\n" ); */
						
						if( flag & CODENT )
							t_dt++;
						else
							t_dd++;
					}
				}
			}
			 else
			 {
			 	/* ROF was not used, release it */
				
#if 0
				printf("\n%s of %s was not needed\n\n", ob_temp->modname, ob_temp->filename );
#endif
			 	while( ob_temp->symbols != NULL )
			 	{
			 		struct exp_sym *next;

			 		next = ob_temp->symbols->next;
			 		free( ob_temp->symbols );
			 		ob_temp->symbols = next;
			 	}
				
				free( ob_temp );
				
				/* Grind past external references */
				count = getwrd( fp );
				while( count-- )
				{
					char	tmpname[SYMLEN+1];
					getname( tmpname, fp );
					fseek( fp, getwrd(fp) * 3, 1 );
				}
				
				/* Spin past local references */
				count = getwrd( fp );
				fseek( fp, count*3, SEEK_CUR );
				
			 }
			 


		} while( !feof( fp ));
		
		clearerr( fp );
	}
	
	/* Check if duplicate globals are found */
	if( dup_fnd > 0 )
	{
		fprintf( stderr, "linker fatal: name clash\n" );
		return 1;
	}
	
	/* Check if direct page usage overflows */

	if( t_idpd + t_udpd > 0xff )
	{
		fprintf( stderr, "linker fatal: direct page allocation is %d bytes\n", t_idpd + t_udpd );
		return 1;
	}
	
	/* Static data and BASIC09 usually dont mix */
	
	if( t_idat > 0 || t_idpd > 0 )
	{
		if( okstatic == 0 )
		{
			fprintf( stderr, "no static data\nlinker fatal: BASIC09 conflict\n" );
			return 1;
		}
		else
		{
			printf( "BASIC09 static data size is %d byte%s.\n", t_idat+t_idpd, t_idat+t_idpd > 1 ? "s" : "" );
		}
	}
	
	/* Adjust data offsets */
	ob_cur = ob_start;
	
	ob_cur->Code = 14 + strlen( modname );
	ob_cur->IDpD = 0;
	ob_cur->UDpD = ob_cur->IDpD + t_idpd;
	ob_cur->IDat = ob_cur->UDpD + t_udpd;
	ob_cur->UDat = ob_cur->IDat + t_idat;
	
	if( ob_cur->next != NULL )
	{
		struct ob_files *prev;
		
		prev = ob_cur;
		ob_cur = ob_cur->next;
		do
		{	
			ob_cur->Code = prev->Code + prev->hd.h_ocode;
			ob_cur->IDpD = prev->IDpD + prev->hd.h_ddata;
			ob_cur->UDpD = prev->UDpD + prev->hd.h_dglbl;
			ob_cur->IDat = prev->IDat + prev->hd.h_data;
			ob_cur->UDat = prev->UDat + prev->hd.h_glbl;
			prev = ob_cur;
		} while( (ob_cur = ob_cur->next) != NULL );
	}
	
	/* Adjust exported symbols */
	
	ob_cur = ob_start;

	while( ob_cur != NULL )
	{
		es_cur = ob_cur->symbols;
		
		while( es_cur != NULL )
		{
			es_cur->offset = adjoff( es_cur->offset, es_cur->flag, ob_cur );
			es_cur = es_cur->next;
		}
		ob_cur = ob_cur->next;
	}
	
/* 	do */
/* 	{ */
/* 		es_cur = ob_cur->symbols; */
/* 		do */
/* 		{ */
/* 			es_cur->offset = adjoff( es_cur->offset, es_cur->flag, ob_cur ); */
/* 		} while( (es_cur = es_cur->next) != NULL ); */
/* 	} while( (ob_cur = ob_cur->next) != NULL ); */

	/* Add special symbols (linker generated symbols) */
	
	if( asign_sm( ob_start, "etext", CODENT, 14 + strlen( ofile ) + t_code ) != 0 ) return 1;
	if( asign_sm( ob_start, "btext", CODENT, 0 ) != 0 ) return 1;
	if( asign_sm( ob_start, "edata", INIENT, t_idpd + t_udpd + t_idat ) != 0 ) return 1;
	if( asign_sm( ob_start, "end", 0, t_idpd + t_udpd + t_idat + t_udat ) != 0 ) return 1;
	if( asign_sm( ob_start, "dpsiz", DIRENT, t_idpd + t_udpd ) != 0 ) return 1;
		
	
	/* Check if there are any unresolved symbols */
	ob_cur = ob_start;
	
	do
	{
		struct ext_ref *ex_cur;
		
		if( ob_cur->exts != NULL )
		{
			if( once == 0 )
			{
				fprintf( stderr, "Unresolved references:\n" );
				once = 1;
			}
			
			ex_cur = ob_cur->exts;
			do
			{
				fprintf( stderr, " %-15s %-15s in %-15s\n", ex_cur->name, ob_cur->modname, ob_cur->filename );
			
			} while( (ex_cur = ex_cur->next) );
		}
	} while( (ob_cur = ob_cur->next) != NULL );
	
	if( once == 1 )
	{
		fprintf( stderr, "linker fatal: unresolved references\n" );
		return 1;
	}
	
	/* Print link Map */
	if( printmap )
	{
		printf( "Linkage map for %s  File - %s\n\n", modname, ofile );
		printf( "Section          Code IDat UDat IDpD UDpD File\n\n" );
		
		ob_cur = ob_start;
		
		do
		{
			printf( "%-16s %4.4x %4.4x %4.4x %2.2x   %2.2x %s\n", ob_cur->modname, ob_cur->Code, ob_cur->IDat,
						ob_cur->UDat, ob_cur->IDpD, ob_cur->UDpD, ob_cur->filename );
			es_cur = ob_cur->symbols;
			if( printsym )
			{
				do
				{
					printf( "     %-9s %s %4.4x\n", es_cur->name, flagtext(es_cur->flag), es_cur->offset);
				} while( (es_cur = es_cur->next) != NULL );
			}
			
		} while( (ob_cur = ob_cur->next) != NULL );
		
		printf( "                 ---- ---- ---- --\n" );
		printf( "                 %4.4x %4.4x %4.4x %2.2x  %2.2x\n\n", t_code, t_idat, t_udat, t_idpd, t_udpd );
	}
	
	/*printf("Total stack space: %4.4x\n", t_stac );*/
	/*printf( "Data-Text count: %d\n", t_dt );*/
	/*printf( "Data-data count: %d\n", t_dd );*/
	
	ofp = fopen( ofile, "w+");
	
	if( ofp == NULL )
	{
		fprintf( stderr, "linker fatal: Cannot open output file %s\n", ofile );
		return 1;
	}
	
	_crc[0] = 0xFF;		/* CRC */
	_crc[1] = 0xFF;
	_crc[2] = 0xFF;
	headerParity = 0;

	/* Start Generating Module */
	/* Module signature */
	fputc(0x87, ofp);
	fputc(0xCD, ofp);
	compute_crc(0x87); compute_crc(0xCD);
	headerParity ^= 0x87;
	headerParity ^= 0xCD;
	
	moduleSize = 14 						/* module header */
	           + strlen( modname )			/* module name */
	           + t_code						/* Code size of all segements */
	           + t_idpd                     /* Initialized direct page data of all segements */
	           + t_idat						/* Initialized data of all segments */
	           + 4							/* Extra linker initialized data */
	           + 2 + t_dt * 2  				/* Data-text reference table */
	           + 2 + t_dd * 2  				/* Data-data reference table */
	           + strlen( modname ) + 1		/* Program name (NULL terminated) */
	           + 3;							/* CRC bytes */
	
	if( moduleSize > 0xffff )
	{
		fprintf( stderr, "linker fatal: Module size to big (%d)\n", moduleSize );
		return 1;
	}
	
	fputc(moduleSize >> 8, ofp);
	fputc(moduleSize & 0xFF, ofp);
	compute_crc(moduleSize >> 8); compute_crc(moduleSize & 0xFF);
	headerParity ^= moduleSize >> 8;
	headerParity ^= moduleSize & 0xFF;
	
	/* Write module name offset (assumed for now) */
	nameOffset = 0x0D;
	fputc(nameOffset >> 8, ofp);
	fputc(nameOffset & 0xFF, ofp);
	compute_crc(nameOffset >> 8); compute_crc(nameOffset & 0xFF);
	headerParity ^= nameOffset >> 8;
	headerParity ^= nameOffset & 0xFF;

	unsigned char typelang, attrev;
	
	/* module type/lang (assume prgrm+objct for now) */
	typelang = 0x11;
	fputc(typelang, ofp);
	compute_crc(typelang);
	headerParity ^= typelang;
		
	/* module attr/rev (assume reent+0 for now) */
	attrev = 0x81;
	fputc(attrev, ofp);
	compute_crc(attrev);
	headerParity ^= attrev;
		
	/* header check (computed at end) */
	headerParity = ~headerParity;
	fputc(headerParity, ofp);
	compute_crc(headerParity);
		
	/* execution offset */
	execOffset = ob_start->hd.h_entry + 14 + strlen( modname );
	fputc(execOffset >> 8, ofp);
	fputc(execOffset & 0xFF, ofp);
	compute_crc(execOffset >> 8); compute_crc(execOffset & 0xFF);

	/* Compute data size */
	dataSize = t_stac + t_idat + t_udat + t_idpd + t_udpd + extramem;
	fputc(dataSize >> 8, ofp);
	fputc(dataSize & 0xFF, ofp);
	compute_crc(dataSize >> 8); compute_crc(dataSize & 0xFF);

	/* module name */
	for (i = 0; i < strlen(modname) - 1; i++)
	{
		fputc(modname[i], ofp);
		compute_crc(modname[i]);
	}
	fputc(modname[i] | 0x80, ofp);
	compute_crc(modname[i] | 0x80);
		
	/* edition */
	fputc(edition, ofp);
	compute_crc(edition);

	/* Now dump all of the code */
	
	ob_cur = ob_start;
	
	while( ob_cur != NULL )
	{
		char *data;
		unsigned count;
		
		/*printf( "Object %s is %4.4lx - %4.4lx\n", ob_cur->modname, ftell(ofp), ftell(ofp)+ob_cur->hd.h_ocode );*/
		
		fseek( ob_cur->fp, ob_cur->object, SEEK_SET );
		data = malloc( ob_cur->hd.h_ocode );
		if( data == NULL )
		{
			fprintf( stderr, "linker fatal: Out of memory\n" );
			return 1;
		}
		
		fread( data, ob_cur->hd.h_ocode, 1, ob_cur->fp);

		/* Now patch binary */
		fseek( ob_cur->fp, ob_cur->object + ob_cur->hd.h_ocode + ob_cur->hd.h_data + ob_cur->hd.h_ddata, SEEK_SET );
		count = getwrd( ob_cur->fp );

/*		if( count > 0 )*/
			/*printf( "External References:\n" );*/
			
		while( count-- )
		{
			char	symbol[SYMLEN+1], valueflg;
			unsigned number, value;
			
			getname( symbol, ob_cur->fp );
			value = getsym( ob_start, symbol, &valueflg );
			number = getwrd( ob_cur->fp );
			
			/*printf( "%-10s %-10s %4.4x (", ob_cur->modname, symbol, value );*/
			/*ftext( valueflg, DEF );*/
			/*printf( ") " );*/
			
			while( number-- )
			{
				unsigned char flag;
				unsigned offset, result, scratch;
				flag = getc( ob_cur->fp );
				offset = getwrd( ob_cur->fp );

				if( flag & CODLOC )
				{
					/*printf( " External ref patch: (" );*/
					/*ftext( flag, REF );*/
					
					if( offset > ob_cur->hd.h_ocode )
					{
						fprintf( stderr, "linker fatal: Code external reference offset greater than code size\n" );
						return 1;
					}
					
					if( flag & LOC1BYT )
						scratch = data[offset];
					 else
						scratch = *((unsigned short *)(&data[offset]));
						
					/*printf( ") %4.4x (%4.4x) data: %4.4x, ", offset + ob_cur->Code, offset, scratch );*/
					
					if( flag & NEGMASK )
						result = ~value;
					else
						result = value;
						
					switch( flag & ~(LOC1BYT|NEGMASK) )
					{
						case 0x20:
							result += scratch;
							break;
						case 0xa0:
							result -= ob_cur->Code;
							result -= offset;
							result -= 2;
							break;
						default:
							fprintf ( stderr, "fatal error: Unknown external reference flag %2.2x\n", flag );
							return 1;
							break;
					}
					
					if( flag & LOC1BYT )
						data[offset] = result;
					else
					{
						data[offset] = result >> 8;
						data[offset+1] = result & 0xff;
					}
				}
				
			}
			
			/*printf( "\n" );*/

		}
		
		/* Patch local refs */
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned char flag;
			unsigned offset, result;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{
				if( offset > ob_cur->hd.h_ocode )
				{
					fprintf( stderr, "linker fatal: Code local reference offset greater than code size\n" );
					return 1;
				}
				
				if( flag & LOC1BYT )
					result = data[offset];
				else
					result = *((unsigned short *)(&data[offset]));
	
				if( flag & NEGMASK )
					result = ~result;
				else
					result = result;
				
				if( flag & DIRENT )
				{
					if( flag & INIENT )
						result += ob_cur->IDpD;
					else
						result += ob_cur->UDpD;
				}
				else
				{
					if( flag & INIENT )
						result += ob_cur->IDat;
					else
						result += ob_cur->UDat;
				}
				
				if( flag & LOC1BYT )
					data[offset] = result;
				else
				{
					data[offset] = result >> 8;
					data[offset+1] = result & 0xff;
				}
	
				/*printf( " Local ref patch (" );*/
				/*ftext( flag, DEF|REF );*/
				/*printf( ") %4.4x (%4.4x)\n", offset + ob_cur->Code, offset );*/
			}
		}
		
		fwrite( data, ob_cur->hd.h_ocode, 1, ofp );
		buffer_crc( data, ob_cur->hd.h_ocode );
		free( data );
		
		ob_cur = ob_cur->next;
	}
		
	/* Now dump all of the Initialized DP data */
	
	ob_cur = ob_start;
	
	while( ob_cur != NULL )
	{
		char *data;
		unsigned count;
		
		/*printf( "Initialized DP data %s is %4.4lx - %4.4lx\n", ob_cur->modname, ftell(ofp), ftell(ofp)+ob_cur->hd.h_ddata );*/

		fseek( ob_cur->fp, ob_cur->object + ob_cur->hd.h_ocode + ob_cur->hd.h_data, SEEK_SET );
		data = malloc( ob_cur->hd.h_ddata );
		if( data == NULL )
		{
			fprintf( stderr, "linker fatal: Out of memory\n" );
			return 1;
		}
		
		fread( data, ob_cur->hd.h_ddata, 1, ob_cur->fp);

		/* Adjust local references */
		fseek( ob_cur->fp, ob_cur->locref, SEEK_SET );
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned char flag;
			unsigned offset, result;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{}
			else
			{
				if( flag & DIRLOC )
				{
					if( flag & LOC1BYT )
						result = data[offset];
					else
						result = *((unsigned short *)(&data[offset]));
		
					if( flag & NEGMASK )
						result = ~result;
					else
						result = result;

					if( flag & CODENT )
					{
						result += ob_cur->Code;
					}
					else
					{
						if( flag & DIRENT )
						{
							if( flag & INIENT )
							{
								result += ob_cur->IDpD;
							}
							else
							{
								result += ob_cur->UDpD;
							}
						}
						else
						{
							if( flag & INIENT )
							{
								result += ob_cur->IDat;
							}
							else
							{
								result += ob_cur->UDat;
							}
						}
					}

					if( flag & LOC1BYT )
						data[offset] = result;
					else
					{
						data[offset] = result >> 8;
						data[offset+1] = result & 0xff;
					}
				}
			}
		}

		fwrite( data, ob_cur->hd.h_ddata, 1, ofp );
		buffer_crc( data, ob_cur->hd.h_ddata );
		free( data );
		
		/* Dump special linker initialized dp data */
		if( ob_cur == ob_start )
		{
			/*printf( "Initialized linker dp data is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+2 );*/
			fputc(t_idpd, ofp);
			compute_crc(t_idpd);
			fputc(t_udpd, ofp);
			compute_crc(t_udpd);
		}

		ob_cur = ob_cur->next;
	}
		
	/* Now dump all of the Initialized data */
	
	ob_cur = ob_start;
	
	while( ob_cur != NULL )
	{
		char *data;
		unsigned count;
		
		/*printf( "Initialized data %s is %4.4lx - %4.4lx\n", ob_cur->modname, ftell(ofp), ftell(ofp)+ob_cur->hd.h_data );*/
		fseek( ob_cur->fp, ob_cur->object + ob_cur->hd.h_ocode, SEEK_SET );
		data = malloc( ob_cur->hd.h_data );
		if( data == NULL )
		{
			fprintf( stderr, "linker fatal: Out of memory\n" );
			return 1;
		}
		
		fread( data, ob_cur->hd.h_data, 1, ob_cur->fp);

		/* Adjust local references */
		fseek( ob_cur->fp, ob_cur->locref, SEEK_SET );
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned char flag;
			unsigned offset, result;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{}
			else
			{
				if( flag & DIRLOC )
				{
				}
				else
				{
					if( flag & LOC1BYT )
						result = data[offset];
					else
						result = *((unsigned short *)(&data[offset]));
		
					if( flag & NEGMASK )
						result = ~result;
					else
						result = result;

					if( flag & CODENT )
					{
						result += ob_cur->Code;
					}
					else
					{
						if( flag & DIRENT )
						{
							if( flag & INIENT )
							{
								result += ob_cur->IDpD;
							}
							else
							{
								result += ob_cur->UDpD;
							}
						}
						else
						{
							if( flag & INIENT )
							{
								result += ob_cur->IDat;
							}
							else
							{
								result += ob_cur->UDat;
							}
						}
					}

					if( flag & LOC1BYT )
						data[offset] = result;
					else
					{
						data[offset] = result >> 8;
						data[offset+1] = result & 0xff;
					}
				}
			}
		}

		fwrite( data, ob_cur->hd.h_data, 1, ofp );
		buffer_crc( data, ob_cur->hd.h_data );
		free( data );
		
		/* Dump special linker initialized data */
		if( ob_cur == ob_start )
		{
			/*printf( "Initialized linker data is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+2 );*/
			fputc(t_idat>>8, ofp);
			compute_crc(t_idat>>8);
			fputc(t_idat&0xff, ofp);
			compute_crc(t_idat&0xff);
		}

		ob_cur = ob_cur->next;
	}
	
	/*printf( "Data-text table is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+2+(t_dt*2) );*/
	/* Now dump Data-text table */
	fputc(t_dt>>8, ofp);
	compute_crc(t_dt>>8);
	fputc(t_dt&0xff, ofp);
	compute_crc(t_dt&0xff);

	ob_cur = ob_start;
	while( ob_cur != NULL )
	{
		unsigned count;
		
		fseek( ob_cur->fp, ob_cur->locref, SEEK_SET );
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned char flag;
			unsigned offset;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{}
			else
			{
				if( flag & CODENT )
				{
					if( flag & DIRLOC )
						offset += ob_cur->IDpD;
					else
						offset += ob_cur->IDat;

					fputc(offset>>8, ofp);
					compute_crc(offset>>8);
					fputc(offset&0xff, ofp);
					compute_crc(offset&0xff);
				}
				else
				{}
			}
		}

		ob_cur = ob_cur->next;
	}

	/*printf( "Data-data table is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+2+(t_dd*2) );*/
	/* Now dump Data-data table */
	fputc(t_dd>>8, ofp);
	compute_crc(t_dd>>8);
	fputc(t_dd&0xff, ofp);
	compute_crc(t_dd&0xff);
	
	ob_cur = ob_start;
	while( ob_cur != NULL )
	{
		unsigned count;
		
		fseek( ob_cur->fp, ob_cur->locref, SEEK_SET );
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned char flag;
			unsigned offset;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{}
			else
			{
				if( flag & CODENT )
				{}
				else
				{
					if( flag & DIRENT )
						offset += ob_cur->IDpD;
					else
						offset += ob_cur->IDat;
						
					fputc(offset>>8, ofp);
					compute_crc(offset>>8);
					fputc(offset&0xff, ofp);
					compute_crc(offset&0xff);
				}
			}
		}

		ob_cur = ob_cur->next;
	}

	/*printf( "Program name is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+strlen( modname )+1 );*/
	/* Now dump program name as a C string */
	fwrite( modname, strlen( modname ), 1, ofp );
	buffer_crc( modname, strlen( modname ) );
	fputc(0, ofp);
	compute_crc(0);
	
	/* Now write CRC */
	fputc(~_crc[0], ofp);
	fputc(~_crc[1], ofp);
	fputc(~_crc[2], ofp);

	return 0;
}

/* Assign symbol
   ob must be ob_start */
   
int asign_sm( ob, name, flag, offset )
struct ob_files *ob;
char *name;
char flag;
unsigned offset;
{
	struct exp_sym *exp;

	exp = malloc( sizeof( struct exp_sym ) );
	
	if( exp == NULL )
	{
		fprintf( stderr, "linker fatal: Out of memory\n" );
		return 1;
	}
	
	strcpy( exp->name, name );
	exp->flag = flag;
	exp->offset = offset;

	if( chk_dup( exp, ob, "linker" ) > 0 )
	{
		free( exp );
		return 1;
	}
	
	rm_exref( name, ob );
	
	exp->next = ob->symbols;
	ob->symbols = exp;
	return 0;
}


int dump_rof_header(hd)
char *hd;
{
	printf("ROF Header:\n");
	return 0;
}

int getname(s, fp)
register char *s;
FILE *fp;
{
     while( (*s++ = getc(fp)) );
     *s = '\0';
     return 0;
}

/* Switch a Little-Endian number byte order to make it Big-Endian */

unsigned o9_int(nbr)
u16 nbr;
{
#ifndef __BIG_ENDIAN__
	return( ((nbr&0xff00)>>8) + ((nbr&0xff)<<8)  );
#else
	return nbr;
#endif
}

unsigned getwrd( fp )
FILE *fp;
{
	unsigned char Msb, Lsb;
	unsigned nbr;
	
	Msb=getc(fp); Lsb=getc(fp);
	nbr = Msb << 8 | Lsb;
	return (o9_int(nbr));
}

char *flagtext( flag )
char flag;
{
	if( flag & CODENT )
	{
		return "code";
	}
	else
	{
		if( flag & INIENT )
		{
			if( flag & DIRENT )
				return "idpd";
			else
				return "idat";
		}
		else
		{
			if( flag & DIRENT )
				return "udpd";
			else
				return "udat";
		}
	}
}

unsigned adjoff( offset, flag, ob )
unsigned offset;
char flag;
struct ob_files *ob;
{
	if( flag & CODENT )
	{
		return offset + ob->Code;
	}
	else
	{
		if( flag & INIENT )
		{
			if( flag & DIRENT )
				return offset + ob->IDpD;
			else
				return offset + ob->IDat;
		}
		else
		{
			if( flag & DIRENT )
				return offset + ob->UDpD;
			else
				return offset + ob->UDat;
		}
	}
}

/* Check if a symbol name is in the exported symbol table
   Returns true, false */
   
int check_name( ob, name )
struct ob_files *ob;
char *name;
{
	do
	{
		struct exp_sym *exp;

		exp = ob->symbols;
		if( exp == NULL )
			continue;
		
		do
		{
			if( strcmp( name, exp->name ) == 0 )
			{
				return 0;
			}
				
		} while( (exp = exp->next) );
		
	} while( (ob = ob->next) );
	
	return 1;
}

/* Checks if a symbol name is in the exported symbol table
   Prints duplicate error if it is */
   
int chk_dup( es, ob, modname )
struct exp_sym *es;
struct ob_files *ob;
char *modname;
{
	int	result = 0;
	
	if( ob == NULL )
		return result;
	
	do
	{
		struct exp_sym *ext;
		
		ext = ob->symbols;
		
		if( ext == NULL )
			continue;
			
		do
		{
				
			if( strcmp( es->name, ext->name ) == 0 )
			{
				fprintf( stderr, "symbol already defined: %-10s in %s and %s\n", es->name, ob->modname, modname );
				result++;
			}
		} while( (ext = ext->next) );
	} while( (ob=ob->next) );

	return result;
}

/* Remove external refrence from external reference table */
int rm_exref( name, ob )
char *name;
struct ob_files *ob;
{
	int	count = 0;

	while( ob != NULL )
	{
		struct ext_ref *ext, *prev;
		
		prev = NULL;
		ext = ob->exts;
		while( ext != NULL )
		{
			if( strcmp( name, ext->name ) == 0 )
			{
				count++;

				if( prev == NULL )
				{
					ob->exts = ext->next;
					free( ext );
					ext = ob->exts;
				}
				else
				{
					struct ext_ref *tmp;
					
					tmp = ext->next;
					prev->next = tmp;
					free( ext );
					ext = tmp;
				}
			}
			else
			{
				prev = ext;
				ext = ext->next;
			}
		}
		
		ob = ob->next;
	}
	
	return count;
}

int dmp_ext( ob )
struct ob_files *ob;
{
	
	while( ob != NULL )
	{
		struct ext_ref *exts;
		
		printf( "   File: %s, mod: %s: ", ob->filename, ob->modname );
		
		exts = ob->exts;
		while( exts != NULL )
		{
			printf( "%s ", exts->name );
			exts = exts->next;
		
		}
		
		printf( "\n" );
		ob = ob->next;
	}
	
	return 0;
}

int compute_crc(a)
unsigned char a;
{
	a ^= _crc[0];
	_crc[0] = _crc[1];
	_crc[1] = _crc[2];
	_crc[1] ^= (a >> 7);
	_crc[2] = (a << 1);
	_crc[1] ^= (a >> 2);
	_crc[2] ^= (a << 6);
	a ^= (a << 1);
	a ^= (a << 2);
	a ^= (a << 4);
	if (a & 0x80)
	{
		_crc[0] ^= 0x80;
		_crc[2] ^= 0x21;
	}

	return 0;
}

int buffer_crc( data, size )
unsigned char data[];
unsigned size;
{
	int	i;
	for( i=0; i<size; i++ )
		compute_crc( data[i] );
	
	return 0;
}

/* Get value for symbol */
unsigned getsym( ob, symbol, flag )
struct ob_files *ob;
char *symbol;
char *flag;
{
	while( ob != NULL )
	{
		struct exp_sym *exp;
		
		exp = ob->symbols;
		
		while( exp != NULL )
		{
			if( strcmp( symbol, exp->name ) == 0 )
			{
				*flag = exp->flag;
				return exp->offset;
			}
			
			exp = exp->next;
		}
		
		ob = ob->next;
	}
	
	fprintf( stderr, "linker fatal: Could not find requested symbol: %s\n", symbol );
	exit( 1 );
	
	return 0;
}

int ftext(c, ref)
char c;
int ref;
{
	printf("(%02x) ", mc(c));
	if (ref & REF)
	{
		if (c & CODLOC)
			printf("in code");
		else
			printf(c & DIRLOC ? "in dp data" : "in non-dp data");
		printf(c & LOC1BYT ? "/byte" : "/word");
		if (c & NEGMASK)
			printf("/neg");
		if (c & RELATIVE)
			printf("/pcr");
	}
	if (ref & DEF)
	{
		if (ref & REF)
			printf(" - ");
		if (c & CODENT)
			printf("to code");
		else
		{
			printf(c & DIRENT ? "to dp" : "to non-dp");
			printf(c & INIENT ? " data" : " bss");
		}
	}
/*	putchar('\n'); */
	return 0;
}






