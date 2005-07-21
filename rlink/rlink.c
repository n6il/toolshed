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
	char		*filename;
	FILE		*fp;
	long		object;
	binhead		hd;
	struct exp_sym *symbols;
	struct ext_ref *exts;
	char		modname[MAXNAME+1];
	unsigned	Code, IDat, UDat, IDpD, UDpD;
};

/* The 'main' function */
int main(argc,argv)
int argc;
char **argv;
{
	char *rfiles[MAX_RFILES];
	char *lfiles[MAX_LFILES];
	char *ofile, *modname;
	int edition, extramem;
	int printmap, printsym;
	int rfile_count, lfile_count, i;


	modname = NULL;
	ofile = NULL;
	edition = 1;
	extramem = 0;
	printmap = 0;
	printsym = 0;

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

	return link(rfiles, rfile_count, lfiles, lfile_count, ofile, modname, edition, extramem, printmap, printsym);
}



int help()
{
	fprintf(stderr, "Usage: rlink <opts> source_file [source_file ...] <opts>\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "   -o[=]file      output object file\n");
	fprintf(stderr, "   -n[=]name      module name of the object file\n");
	fprintf(stderr, "   -l[=]path      additional library\n");
	fprintf(stderr, "   -E[=]edition   edition number in the module\n");
	fprintf(stderr, "   -M[=]size      additional number of pages of memory\n");
	fprintf(stderr, "   -m             print the linkage map\n");
	fprintf(stderr, "   -s             print the symbol table\n");
/*	fprintf(stderr, "   -b=ept         Make callable from BASIC09\n"); */
/*	fprintf(stderr, "   -t             allow static data to appear in BASIC09 module\n"); */

	return 0;
}



int link(rfiles, rfile_count, lfiles, lfile_count, ofile, modname, edition, extramem, printmap, printsym)
char *rfiles[];
int rfile_count;
char *lfiles[];
int lfile_count;
char *ofile;
char *modname;
int edition, extramem, printmap, printsym;
{
	int	i;
	struct ob_files *ob_start, *ob_cur;
	struct exp_sym *es_cur;
	unsigned t_code = 0, t_idat = 0, t_udat = 0, t_idpd = 0, t_udpd = 0;	
	int	once = 0, dup_fnd = 0;

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
			
			/* Place initial settings stuct */
/* 			t_code = 0; */
/* 			t_idat = 0; */
/* 			t_udat = 0; */
/* 			t_idpd = 0; */
/* 			t_udpd = 0; */
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

		/* Check for validity of ROF file */
		
		if( ob_cur->hd.h_sync != ROFSYNC )
		{
			fprintf( stderr, "linker error: File %s is not an ROF file.\n", rfiles[i] );
			return 1;
		}
		
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
	}
	
	/* Process library files to resolve remaining undefined symbols */

	for (i = 0; i < lfile_count; i++)
	{
		FILE *fp;
		unsigned modcount;
		
		modcount = 0;
		
		fp = fopen(lfiles[i], "r");

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

			 	ob_cur = ob_temp;
			 }
			 else
			 {
			 	/* ROF was not used, release it */

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
			 }
			 
			/* Grind past local references */
			fseek( fp, getwrd( fp ) * 3, 1 );

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
	
	/* Adjust data offsets */
	ob_cur = ob_start;
	
	ob_cur->Code = 13 + strlen( ofile ) + 1;
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

	do
	{
		es_cur = ob_cur->symbols;
		do
		{
			es_cur->offset = adjoff( es_cur->offset, es_cur->flag, ob_cur );
		} while( (es_cur = es_cur->next) != NULL );
	} while( (ob_cur = ob_cur->next) != NULL );

	/* Add special symbols (linker generated symbols) */
	
	if( asign_sm( ob_start, "etext", CODENT, t_code - (13 + strlen( ofile ) + 1) ) != 0 ) return 1;
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
	
	/* Print linked list -- To make sure my code is working */
	printf( "Linkage map for a1  File - %s\n\n", ofile );
	printf( "Section          Code IDat UDat IDpD UDpD File\n\n" );
	
	ob_cur = ob_start;
	
	do
	{
		printf( "%-16s %4.4x %4.4x %4.4x %2.2x   %2.2x %s\n", ob_cur->modname, ob_cur->Code, ob_cur->IDat,
					ob_cur->UDat, ob_cur->IDpD, ob_cur->UDpD, ob_cur->filename );
		es_cur = ob_cur->symbols;
		do
		{
			printf( "     %-9s %s %4.4x\n", es_cur->name, flagtext(es_cur->flag), es_cur->offset);
		} while( (es_cur = es_cur->next) != NULL );
		
		
	} while( (ob_cur = ob_cur->next) != NULL );
	
	printf( "                 ---- ---- ---- --\n" );
	printf( "                 %4.4x %4.4x %4.4x %2.2x  %2.2x\n\n", t_code, t_idat, t_udat, t_idpd, t_udpd );
	
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
	char Msb, Lsb;
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
	
	if( ob == NULL )
		return 0;
	
	do
	{
		struct ext_ref *ext, *prev = NULL;
		
		ext = ob->exts;
		
		if( ext == NULL )
			continue;
			
		do
		{
				
			if( strcmp( name, ext->name ) == 0 )
			{
				/* Found it, now remove it */
				count++;
				if( prev == NULL )
				{
					ob->exts = NULL;
					free( ext );
					break;
				}
				else
				{
					prev->next = ext->next;
					free( ext );
					ext = prev;
				}
			}
			
			prev = ext;
		} while( (ext = ext->next) );
	} while( (ob=ob->next) );
	
	return count;
}









