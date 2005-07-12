#include <stdio.h>
#ifdef UNIX
#include <stdlib.h>
#endif
#include "rlink.h"


#define	DEBUG


int link();	/* The function that "does it all" */


#define MAX_RFILES	32
#define MAX_LFILES	32



/* The 'main' function */
main(argc,argv)
int argc;
char **argv;
{
	char *rfiles[MAX_RFILES];
	char *lfiles[MAX_RFILES];
	char *ofile;
	int rfile_count, lfile_count, i;


	ofile = NULL;
	rfile_count = 0;
	lfile_count = 0;


	/* Parse options */

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			/* option -- process it */
			switch (tolower(argv[i][1]))
			{
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

	return link(rfiles, rfile_count, lfiles, lfile_count, ofile);
}



int help()
{
	fprintf(stderr, "Usage: rlink <opts>\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "   -l[=]libfile   library\n");

	return 0;
}



int link(rfiles, rfile_count, lfiles, lfile_count, ofile)
char *rfiles[];
int rfile_count;
char *lfiles[];
int lfile_count;
char *ofile;
{
	int	i;


#ifdef DEBUG
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

	
	return 0;
}
