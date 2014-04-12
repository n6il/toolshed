/********************************************************************
 * decbfree.c - Disk utilization reporting utility for Disk BASIC
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <decbpath.h>
#include <toolshed.h>


static int do_free(char **argv, char *p);

/* Help message */
static char *helpMessage[] =
{
	"Syntax: free {[<opts>]} {<disk> [<...>]} {[<opts>]}\n",
	"Usage:  Displays the amount of free space on an image.\n",
	"Options:\n",
	NULL
};


int decbfree(int argc, char *argv[])
{
	error_code	ec = 0;
	char		*p = NULL;
	int			i;
	u_int		free_granules;

	/* walk command line for options */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (p = &argv[i][1]; *p != '\0'; p++)
			{
				switch(*p)
				{
					case '?':
					case 'h':
						show_help(helpMessage);
						return(0);
	
					default:
						fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
						return(0);
				}
			}
		}
	}

	/* walk command line for pathnames */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			continue;
		}
		else
		{
			p = argv[i];
		}


		ec = TSDECBFree(p, &free_granules);

		if (ec == 0)
		{
			printf("Free granules: %d (%d bytes)\n", free_granules, free_granules * (4608 / 2));
		}
		else
		{
			fprintf(stderr, "%s: error %d determining free space for '%s'\n", argv[0], ec, p);
			return(ec);
		}
	}

	if (argv[1] == NULL)
	{
		show_help(helpMessage);
		return(0);
	}

	return(0);
}


static int do_free(char **argv, char *p)
{
	error_code	ec = 0;
	int i;
	char decbpathlist[256];
	decb_path_id path;
	int free_granules = 0;


	/* 1. Make a copy of the pathlist. */
	
	strcpy(decbpathlist, p);


	/* 2. If a comma isn't present int he string, then add it so that the path is opened as a non-native path. */
	
	if (strchr(decbpathlist, ',') == NULL)
	{
		strcat(decbpathlist, ",");
	}
	
	
	/* 3. Open a path to the device. */
	
	ec = _decb_open(&path, decbpathlist, FAM_READ);

	if (ec != 0)
	{
		fprintf(stderr, "%s: error %d opening '%s'\n",
			argv[0], ec, decbpathlist);

		return(ec);
	}


	/* 4. Walk the FAT. */

	for (i = 0; i < 256; i++)
	{
		if (path->FAT[i] == 0xFF)
		{
			free_granules++;
		}
	}


	/* 5. Report the free space. */
	
	printf("Free granules: %d (%d bytes)\n", free_granules, free_granules * (4608 / 2));


	/* 6. Close the path. */
	
	_decb_close(path);


	/* 7. Return status. */
	
	return(0);
}
