/********************************************************************
 * decbrename.c - Renames a Disk BASIC file
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cocotypes.h>
#include <cocopath.h>
#include <toolshed.h>


/* Help message */
static char const * const helpMessage[] =
{
	"Syntax: rename {<file> <newfilename>}\n",
	"Usage:  Give a file a new filename.\n",
	"Options:\n",
	NULL
};


int os9rename(int argc, char *argv[])
{
    error_code	ec = 0;
    char *p = NULL, *new_name = NULL;
    int i;


    /* 1. Walk command line for options. */
	
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


    /* 2. Walk command line for pathnames. */

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            continue;
        }
        else
        {
            if( p == NULL )
            {
                p = argv[i];
            }
            else
            {
                new_name = argv[i];
            }
        }
		
        if( (p != NULL) && (new_name != NULL) )
        {
            ec = TSRename(p, new_name);
		
            if (ec != 0)
            {
                fprintf(stderr, "%s: error %d opening '%s'\n", argv[0], ec, p);
            }
			
            return ec;
        }
    }

    if (p == NULL)
    {
        show_help(helpMessage);
    }


    return 0;
}
