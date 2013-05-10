/********************************************************************
 * os9list.c - List utility for OS-9
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cocotypes.h>
#include <cocopath.h>


/* Help message */
static char *helpMessage[] =
{
	"Syntax: list {[<opts>]} {<file> [<...>]} {[<opts>]}\n",
	"Usage:  Display contents of a text file.\n",
	"Options:\n",
	NULL
};


int os9list(int argc, char *argv[])
{
	error_code	ec = 0;
	char *p = NULL;
	os9_path_id path;
	int i;

	/* walk command line for options */
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
			break;
		}
	}

	if (p == NULL)
	{
		show_help(helpMessage);
		return(0);
	}

	/* open a path to the file */
	ec = _os9_open(&path, p, FAM_READ);
	if (ec != 0)
	{
		_os9_close(path);
		printf("Error %d opening %s\n", ec, p);

		return(ec);
	}

	while (_os9_gs_eof(path) == 0)
	{
		int size = 1022;
		char buffer[1024];
		char *p;

		ec = _os9_readln(path, buffer, &size);
		if (ec != 0)
		{
			break;
		}
		
		buffer[size] = '\0';

		p = strchr(buffer, 0x0D);

		if (p != NULL)
		{
			*p = '\0';
		}

		printf("%s\n", buffer);
	}

	_os9_close(path);

	return(0);
}
