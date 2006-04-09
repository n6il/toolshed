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
#ifndef BDS
#include <unistd.h>
#include <sys/stat.h>
#endif
#include <cocotypes.h>
#include <cocopath.h>

/* New for Windows             RG */
#ifdef	_WIN32
#define	_S_IRGRP	0x0020	/* RG, made this up */
#define	_S_IXGRP	0x0008	/* ditto */
#define	_S_IROTH	0x0004	/* ditto */
#define	_S_IXOTH	0x0001	/* ditto */
#define	S_IRGRP		_S_IRGRP	/* ditto */
#define	S_IXGRP		_S_IXGRP	/* ditto */
#define	S_IROTH		_S_IROTH	/* ditto */
#define	S_IXOTH		_S_IXOTH	/* ditto */
#define MKDIR(D,P) mkdir((D))
#else
#define MKDIR(D,P) mkdir(D,P)
#endif

static int do_makdir(char **argv, char *p);


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

		ec = do_makdir(argv, p);

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
	

static int do_makdir(char **argv, char *p)
{
	error_code		ec = 0;
	char			*subPath;
	int			i = 0, length = strlen(p);
        

	/* 1. Determine if there is an OS-9 pathlist. */
	
	if (strchr(p, ',') == NULL)
	{
		/* 1. Call the native file system makdir */
		ec = MKDIR(p, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		/* ec = mkdir(p); */
	}
	else
	{
		/* 1. Make a copy of the passed path into 'subPath'. */
		
		subPath = malloc(length + 1);
        
		strcpy(subPath, p);
		

		/* 2. Compute index to char past the native delimiter. */
		
		i = (strchr(p, ',') - p) + 1;
		
		
		/* 3. Walk path and create directory entries as we go */

		do
		{          
			if (subPath[i] == 0)
			{
				ec = _os9_makdir(subPath);

				return ec;
			}

			else if (subPath[i] == '/')
			{
				subPath[i] = 0;
				ec = _os9_makdir( subPath );
				subPath[i] = '/';
			}
        
			i++;
		} while (i <= length);

		free(subPath);
	}
	
	
    return ec;
}
