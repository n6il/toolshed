#include <stdio.h>
#ifdef UNIX
#include <stdlib.h>
#endif
#include "rlink.h"


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
	int rfile_count, lfile_count, i;


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
					exit(0);
			}
		}
		else if (rfile_count < MAX_RFILES)
		{
			/* file -- add it to the list */
			rfiles[rfile_count] = argv[i];
			rfile_count++;
		}
	}

	/* Get list of .r files and put them together */

	return link(rfiles, rfile_count, lfiles, lfile_count);
}



int help()
{
	fprintf(stderr, "Usage: rlink <opts>\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "   -l[=]libfile   library\n");

	return 0;
}



int link(rfiles, rfile_count, lfiles, lfile_count)
char *rfiles[];
int rfile_count;
char *lfiles[];
int lfile_count;
{
	int	i;


#ifndef DEBUG
	if (rfile_count > 0)
	{
		printf("ROF files:\n");
		for (i = 0; i < rfile_count; i++)
		{
			printf("   %s\n", rfiles[i]);
		}
	}

	if (lfile_count > 0)
	{
		printf("\nLibrary files:\n");
		for (i = 0; i < lfile_count; i++)
		{
			printf("   %s\n", lfiles[i]);
		}
	}
#endif

	
	return 0;
}
