/********************************************************************
 * os9makdir.c - Create an os9 directory
 *
 * $Id$
 ********************************************************************
*
* Edt/Rev         YYYY/MM/DD     Modified by
* Comment
* ----------------------------------------------------------
*   1             2005/12/22     Robert Gault
* Added missing Defines for Windows. I'm not sure if _WIN32 is defined
* on systems other than Windows. This will need to be tested.
*/

#include <util.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#if !defined(BDS) && !defined(VS)
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <cocotypes.h>
#include <cocopath.h>


/* Help message */
static char *helpMessage[] =
{
	"Syntax: makdir {<dirname> [<...>]}\n",
	"Usage:  Create one or more directories.\n",
	NULL
};


int os9makdir(int argc, char *argv[])
{
	error_code	ec = 0;
	char *p = NULL;
	int i;

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

		ec = TSMakeDirectory(p);

		if (ec != 0)
		{
			fprintf(stderr, "%s: error %d creating '%s'\n", argv[0], ec, p);
			return(ec);
		}
	}

	if (p == NULL)
	{
		show_help(helpMessage);
		return(0);
	}

	return(0);
}
