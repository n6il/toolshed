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


#define	DEBUG


int link();	/* The function that "does it all" */
int dump_rof_header();


#define MAX_RFILES	32
#define MAX_LFILES	32



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
		}
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
		FILE *fp;

		fp = fopen(rfiles[i], "r");
		
		if (fp != NULL)
		{
			binhead hd;

			fread(&hd, sizeof(hd), 1, fp);
#ifdef DEBUG
			dump_rof_header(hd);
#endif
			fclose(fp);
		}
		else
		{
			fprintf(stderr, "linker error: cannot open file %s\n", rfiles[i]);
		}
	}
	
	return 0;
}


int dump_rof_header(hd)
char *hd;
{
	printf("ROF Header:\n");
}

