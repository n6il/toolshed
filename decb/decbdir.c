/********************************************************************
 * decbdir.c - Directory utility for decb
 *
 * $Id$
 ********************************************************************/
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef BDS
#include <unistd.h>
#endif

#include "util.h"
#include "cocotypes.h"
#include "decbpath.h"

static int do_dir(char **argv, char *p);


/* Help Message */
static char *helpMessage[] =
{
	"Syntax: dir {[<opts>]} {<dir> [<...>]} {[<opts>]}\n",
	"Usage:  Display the contents of a directory.\n",
	"Options:\n",
	NULL
};


int decbdir(int argc, char *argv[])
{
	error_code	ec = 0;
	char *p = NULL;
	int i;


	if (argv[1] == NULL)
	{
		show_help(helpMessage);

		return 0;
	}

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
						return 0;
	
					default:
						fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
						return 0;
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

		ec = do_dir(argv, p);

		if (ec != 0)
		{
			fprintf(stderr, "%s: error %d opening '%s'\n", argv[0], ec, p);

			return ec;
		}
	}


	return 0;
}



static int do_dir(char **argv, char *p)
{
	error_code		ec = 0;
	char			decbpathlist[256], sector[256];
	decb_path_id	path;
	decb_dir_entry  de;
	
	
	/* 1. If a comma isn't present in the string, then add it so that the path is opened as a non-native path. */
	
	memset(decbpathlist, 0, 256);
	
	if (strchr(p, ',') == NULL)
	{
		char *q = strchr(p, ':');
		
		if (q == NULL)
		{
			strcpy(decbpathlist, p);
		}
		else
		{
			strncpy(decbpathlist, p, q - p);
			strcat(decbpathlist, ",");
			strcat(decbpathlist, q);
		}
	}
	else
	{
		strcpy(decbpathlist, p);
	}
	
	
	/* 3. Open a path to the device. */

	ec = _decb_open(&path, decbpathlist, FAM_READ);
	
	if (ec != 0)
	{
		fprintf(stderr, "%s: error %d opening '%s'\n", argv[0], ec, decbpathlist);
		
		return ec;
	}
	
	
	/* 4. Obtain HDB-DOS disk name, if any. */
	
	_decb_gs_sector(path, 17, 17, sector);
	
	if (sector[0] != '\xFF')
	{
		printf("%s\n", sector);
	}
	
	
	/* 5. Read and print each directory entry*/
	
	while (_decb_readdir(path, &de) == 0)
	{
		char	asciiflag;
		int		granule_size = 1;
		int		curr_granule;


		if (de.filename[0] == 0 || de.filename[0] == 255)
		{
			continue;
		}
		
		
		switch(de.ascii_flag)
		{
			case 0x00:
				asciiflag = 'B';
				break;
			case 0xFF:
				asciiflag = 'A';
				break;
			default:
				asciiflag = '?';
				break;
		}
		
		curr_granule = de.first_granule;
		
		while (path->FAT[curr_granule] < 0xC0)
		{
			curr_granule = path->FAT[curr_granule];
			
			granule_size++;
		}
		
		printf("%8.8s %3.3s  %1.1d  %c  %d\n", de.filename, de.file_extension, de.file_type, asciiflag, granule_size);
	}

	
	return 0;
}

