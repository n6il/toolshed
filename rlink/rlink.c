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
#endif
#include "rlink.h"

unsigned int o9_int();
unsigned int getwrd();

/* #define	DEBUG */


int link();	/* The function that "does it all" */
int dump_rof_header();
char *flagtext();
unsigned adjoff();


#define MAX_RFILES	32
#define MAX_LFILES	32

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
	int			r_file;
	int			l_file;
	FILE		*fp;
	int			lf_offset;
	binhead		hd;
	struct exp_sym	*symbols;
	char		modname[MAXNAME+1];
	unsigned	Code, IDat, UDat, IDpD, UDpD;
};

/* The 'main' function */
main(argc,argv)
int argc;
char **argv;
{
	char *rfiles[MAX_RFILES];
	char *lfiles[MAX_RFILES];
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
						{
							p++;
						}

						if (lfile_count < MAX_LFILES)
						{
							lfiles[lfile_count] = p;
							lfile_count++;
							if( lfile_count > MAX_LFILES )
							{
								fprintf(stderr, "Linker fatal: To many library files.\n" );
								exit(1);
							}
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
		else if (rfile_count < MAX_RFILES)
		{
			/* file -- add it to the list */
			rfiles[rfile_count] = argv[i];
			rfile_count++;
			if( rfile_count > MAX_RFILES )
			{
				fprintf(stderr, "Linker fatal: To many ROF files.\n" );
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
	fprintf(stderr, "Usage: rlink <opts>\n");
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
	

#ifdef DEBUG
	printf("edition = %d, memsize = %d, modname = %s, printmap=%d, printsym=%d\n",
		edition, extramem, modname, printmap, printsym);

	if (rfile_count > 0)
	{
		printf("ROF files: ");
		for (i = 0; i < rfile_count; i++)
		{
			printf("[%s] ", rfiles[i]);
		}

		printf("\n");
	}

	if (lfile_count > 0)
	{
		printf("Library files: ");
		for (i = 0; i < lfile_count; i++)
		{
			printf("[%s] ", lfiles[i]);
		}

		printf("\n");
	}

	if (ofile != NULL)
	{
		printf("Output file: [%s]\n", ofile);
	}
#endif

	/* We have the ROF input files, the library input files and
	 * the output file. Now let's go to work and link 'em!
	 */

	for (i = 0; i < rfile_count; i++)
	{
		struct ob_files *ob_temp;
		unsigned count;
		
		ob_temp = malloc( sizeof( struct ob_files  ));
		
		if( ob_temp == NULL )
		{
			fprintf( stderr, "linker fatal: Out of memory\n" );
			exit(1);
		}
		
		if( i == 0 )
		{
			ob_start = ob_temp;
			ob_cur = ob_start;
			
			/* Place initial settings stuct */
			ob_cur->Code = 13 + strlen(ofile) + 1;
			ob_cur->IDat = 1;			/* Not 100% sure of this value */
			ob_cur->UDat = 5;			/* Not 100% sure of this value */
			ob_cur->IDpD = 0;			/* Not 100% sure of this value */
			ob_cur->UDpD = 1;			/* Not 100% sure of this value */
		}
		else
		{
			ob_cur->next = ob_temp;
			
			/* Accumulate offsets */
			ob_temp->Code = ob_cur->Code + ob_cur->hd.h_ocode;
			ob_temp->IDat = ob_cur->IDat + ob_cur->hd.h_data;
			ob_temp->UDat = ob_cur->UDat + ob_cur->hd.h_glbl;
			ob_temp->IDpD = ob_cur->IDpD + ob_cur->hd.h_ddata;
			ob_temp->UDpD = ob_cur->UDpD + ob_cur->hd.h_dglbl;
			
			ob_cur = ob_temp;
		}
		
		ob_cur->next = NULL;
		ob_cur->r_file = i;
		ob_cur->l_file = -1;
		ob_cur->lf_offset = 0;
		ob_cur->symbols = NULL;
		ob_cur->fp = fopen(rfiles[i], "r");

		if (ob_cur->fp != NULL)
		{
			fread(&(ob_cur->hd), sizeof(binhead), 1, ob_cur->fp);
#ifdef DEBUG
			dump_rof_header(hd);
#endif
		}
		else
		{
			fprintf(stderr, "linker error: cannot open file %s\n", rfiles[i]);
			exit(1);
		}
		
		/* Check for validity of ROF file */
		
		if( ob_cur->hd.h_sync != ROFSYNC )
		{
			fprintf( stderr, "linker error: File %s is not an ROF file.\n", rfiles[i] );
			exit(1);
		}
		
		if( i==0 ) /* First ROF file needs special header */
		{
			if( ob_cur->hd.h_tylan == 0 )
			{
				fprintf( stderr, "Linker error: Initial ROF file (%s) must be type non-zero.\n", rfiles[i] );
				exit(1);
			}
		}
		else
		{
			if( ob_cur->hd.h_tylan != 0 )
			{
				fprintf( stderr, "Linker error: ROF file (%s) must be type zero.\n", rfiles[i] );
				exit(1);
			}
		}
		
		if( ob_cur->hd.h_valid )
		{
			fprintf( stderr, "Linker error: ROF file: %s must contain valid object code.\n", rfiles[i] );
			exit( 1 );
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
				exit(1);
	     	}
	     	
			es_temp->next = ob_cur->symbols;
			ob_cur->symbols = es_temp;

	     	getname( ob_cur->symbols->name, ob_cur->fp );
	     	ob_cur->symbols->flag = getc(ob_cur->fp);
	     	ob_cur->symbols->offset = getwrd(ob_cur->fp );
	     }
	     
	     /* Add special symbols to first ROF file */
#if 1     
	     if( i == 0 )
	     {
	     	struct exp_sym *es_temp;
	     	
	     	es_temp = malloc( sizeof( struct exp_sym) );
	     	
	     	if( es_temp == NULL )
	     	{
				fprintf( stderr, "linker fatal: Out of memory\n" );
				exit(1);
	     	}
	     	
			es_temp->next = ob_cur->symbols;
			ob_cur->symbols = es_temp;
	     	
	     	strcpy( ob_cur->symbols->name, "etext" );
	     	ob_cur->symbols->flag = CODENT; /* To determine */
	     	ob_cur->symbols->offset = 0; /* To determine */

	     	es_temp = malloc( sizeof( struct exp_sym) );
	     	
	     	if( es_temp == NULL )
	     	{
				fprintf( stderr, "linker fatal: Out of memory\n" );
				exit(1);
	     	}
	     	
			es_temp->next = ob_cur->symbols;
			ob_cur->symbols = es_temp;
	     	
	     	strcpy( ob_cur->symbols->name, "btext" );
	     	ob_cur->symbols->flag = CODENT; /* To determine */
	     	ob_cur->symbols->offset = 0; /* To determine */

	     	es_temp = malloc( sizeof( struct exp_sym) );
	     	
	     	if( es_temp == NULL )
	     	{
				fprintf( stderr, "linker fatal: Out of memory\n" );
				exit(1);
	     	}
	     	
			es_temp->next = ob_cur->symbols;
			ob_cur->symbols = es_temp;
	     	
	     	strcpy( ob_cur->symbols->name, "edata" );
	     	ob_cur->symbols->flag = INIENT; /* To determine */
	     	ob_cur->symbols->offset = 0; /* To determine */

	     	es_temp = malloc( sizeof( struct exp_sym) );
	     	
	     	if( es_temp == NULL )
	     	{
				fprintf( stderr, "linker fatal: Out of memory\n" );
				exit(1);
	     	}
	     	
			es_temp->next = ob_cur->symbols;
			ob_cur->symbols = es_temp;
	     	
	     	strcpy( ob_cur->symbols->name, "end" );
	     	ob_cur->symbols->flag = 0; /* To determine */
	     	ob_cur->symbols->offset = 0; /* To determine */

	     	es_temp = malloc( sizeof( struct exp_sym) );
	     	
	     	if( es_temp == NULL )
	     	{
				fprintf( stderr, "linker fatal: Out of memory\n" );
				exit(1);
	     	}
	     	
			es_temp->next = ob_cur->symbols;
			ob_cur->symbols = es_temp;
	     	
	     	strcpy( ob_cur->symbols->name, "dpsize" );
	     	ob_cur->symbols->flag = DIRENT; /* To determine */
	     	ob_cur->symbols->offset = 0; /* To determine */

	     }
#endif
	}
	
	/* Check if direct page usage overflows */
	/* ob_cur must point to last object entry */

	if( ob_cur->IDpD + ob_cur->UDpD > 0xff )
	{
		fprintf( stderr, "linker fatal: direct page allocation is %d bytes\n", ob_cur->IDpD + ob_cur->UDpD );
		exit(1);
	}

	/* Print linked list -- To make sure my code is working */
	printf( "Linkage map for a1  File - %s\n\n", ofile );
	printf( "Section          Code IDat UDat IDpD UDpD File\n\n" );
	
	ob_cur = ob_start;
	
	do
	{
		printf( "%-16s %4.4x %4.4x %4.4x   %2.2x   %2.2x %s\n", ob_cur->modname, ob_cur->Code, ob_cur->IDat,
					ob_cur->UDat, ob_cur->IDpD, ob_cur->UDpD, rfiles[ob_cur->r_file] );
		es_cur = ob_cur->symbols;
		do
		{
			printf( "     %-9s %s %4.4x\n", es_cur->name, flagtext(es_cur->flag),
										adjoff( es_cur->offset, es_cur->flag, ob_cur ) );
		} while( (es_cur = es_cur->next) != NULL );
		
		
	} while( (ob_cur = ob_cur->next) != NULL );
	
	return 0;
}


int dump_rof_header(hd)
char *hd;
{
	printf("ROF Header:\n");
}

getname(s, fp)
register char *s;
FILE *fp;
{
     while(*s++ = getc(fp));
     *s = '\0';
}

/* Switch a Little-Endian number byte order to make it Big-Endian */

unsigned int o9_int(nbr)
u16 nbr;
{
#ifndef __BIG_ENDIAN__
	return( ((nbr&0xff00)>>8) + ((nbr&0xff)<<8)  );
#else
	return nbr;
#endif
}

unsigned int getwrd(FILE *fp)
{
	unsigned char Msb, Lsb, nbr;
	Msb=getc(fp); Lsb=getc(fp);
	nbr = Msb << 16 | Lsb;
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
