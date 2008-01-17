/********************************************************************
 * cecbfstat.c - File status utility for Cassette BASIC
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef BDS
#include <unistd.h>
#endif
#include <cocotypes.h>
#include <cecbpath.h>

static int do_fstat(char **argv, char *p);

/* Help message */
static char *helpMessage[] =
{
	"Syntax: fstat {[<opts>]} {<file> [<...>]} {[<opts>]}\n",
	"Usage:  Display detailed information about a file.\n",
	"Options:\n",
	NULL
};


int cecbfstat(int argc, char *argv[])
{
	char *p = NULL;
	int i;


	/* 1. Walk command line for options. */

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (p = &argv[i][1]; *p != '\0'; p++)
			{
				switch (*p)
				{
					case 'h':
					case '?':
						show_help(helpMessage);
						return(0);
	
					default:
						fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
						return(0);
				}
			}
		}
	}


	/* 2. Walk command line for pathnames. */
	
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			continue;
		}
		else
		{
			p = argv[i];

			do_fstat(argv, p);
			printf("\n");
		}
	}

	if (p == NULL)
	{
		show_help(helpMessage);
		return(0);
	}


	return(0);
}
	

static int do_fstat(char **argv, char *p)
{
	error_code	ec = 0;
	cecb_path_id path;
	

	/* 1. Open a path to the device. */

	ec = _cecb_open( &path, p, FAM_READ );

	if (ec == 0)
	{
		printf("File Information for %s\n", p);
		printf("  File type            : ");

		switch (path->dir_entry.file_type)
		{
			case 0:
				printf("BASIC Program\n");
				break;

			case 1:
				printf("Data\n");
				break;

			case 2:
				printf("M/L Program\n");
				break;

			case 3:
				printf("Text Editor Source\n");
				break;

			default:
				printf("???\n");
				break;
		}
		
		printf("  Data type            : ");
		
		switch (path->dir_entry.ascii_flag)
		{
			case 0:
				printf("Binary\n");
				break;

			case 255:
				printf("ASCII\n");
				break;

			default:
				printf("???\n");
				break;
		}

		printf("  Gap type             : ");
		
		switch (path->dir_entry.gap_flag)
		{
			case 0:
				printf("No\n");
				break;

			case 255:
				printf("Yes\n");
				break;

			default:
				printf("???\n");
				break;
		}
		
		printf( "  ML Load Address      : %d (0x%4.4x)\n",
					path->dir_entry.ml_load_address1 << 8 | path->dir_entry.ml_load_address2,
					path->dir_entry.ml_load_address1 << 8 | path->dir_entry.ml_load_address2 );

		printf( "  ML Execution Address : %d (0x%4.4x)\n",
					path->dir_entry.ml_exec_address1 << 8 | path->dir_entry.ml_exec_address2,
					path->dir_entry.ml_exec_address1 << 8 | path->dir_entry.ml_exec_address2 );
	}
	

	_cecb_close(path);


	return ec;
}
