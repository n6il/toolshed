/********************************************************************
 * os9makdir.c - Create an os9 directory
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
        
        subPath = malloc( length + 1 );
        
	/* Walk path and create directory entries as we go */

    do
    {
        subPath[i] = p[i];
          
        if( subPath[i] == 0 )
        {
             ec = _os9_makdir(subPath);

             return ec;
        }
        else if( subPath[i] == '/' )
        {
            subPath[i] = 0;
            ec = _os9_makdir( subPath );
            subPath[i] = '/';
        }
        
        i++;
    } while (i <= length);

    free(subPath);
	
	
    return ec;
}
