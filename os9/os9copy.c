/********************************************************************
 * os9copy.c - Copy utility for OS-9 filesystem
 *
 * $Id$
 ********************************************************************/
#include <string.h>
#ifndef _BORLAND
#include <sys/stat.h>
#endif
#include <util.h>
#include <cocopath.h>
#include <cocotypes.h>


#define YES 1
#define NO 0

/* globals */
static char *ExtractFilename(char *path);


/* Help message */
static char *helpMessage[] =
{
    "Syntax: copy {[<opts>]} <srcfile> {[<...>]} <target> {[<opts>]}\n",
    "Usage:  Copy one or more files to a target directory.\n",
    "Options:\n",
    "     -b=size    size of copy buffer in bytes or K-bytes\n",
    "     -l         perform end of line translation\n",
    "     -o=id      set file's owner as id\n",
    "     -r         rewrite if file exists\n",
    NULL
};


int os9copy(int argc, char *argv[])
{
    error_code	ec = 0;
    char *p = NULL, *q, *desttarget = NULL;
    int i, j;
    int targetDirectory = NO;
    int	count = 0;
    int	eolTranslate = 0;
    int	rewrite = 0;
    char	df[256];
	int owner = 0, owner_set = 0;
	char *buffer;
	u_int buffer_size = 32768;



    /* 1. Walk command line for options. */

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (p = &argv[i][1]; *p != '\0'; p++)
            {
                switch(*p)
                {
                    case 'b':
                        if (*(++p) == '=')
                        {
                            p++;
                        }
                        q = p + strlen(p) - 1;
                        if (toupper(*q) == 'K')
                        {
                            *q = '0';
                            buffer_size = atoi(p) * 1024;
                        }
                        else
                        {
                            buffer_size = atoi(p);
                        }
                        p = q;
                        break;

                    case 'l':
                        eolTranslate = 1;
                        break;

                    case 'r':
                        rewrite = 1;
                        break;

                    case 'o':
                        if (*(++p) == '=')
                        {
                            p++;
                        }

                        q = p + strlen(p) - 1;
						{
							owner = atoi(p);
							owner_set = 1;
						}
						p = q;
                        break;

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


    /* 2. Allocate memory for the copy buffer. */

    buffer = (char *)malloc(buffer_size);

    if (buffer == NULL)
    {
        fprintf(stderr, "%s: cannot allocate %d byte of buffer memory\n", argv[0], buffer_size);

        return 1;
    }


    /* 3. Count non option arguments. */

    for (i = 1, count = 0; i < argc; i++)
    {
        if (argv[i] == NULL)
		{
            continue;
		}

        if (argv[i][0] == '-')
		{
            continue;
		}

        count++;
    }

    if (count == 0)
    {
        show_help(helpMessage);
	free(buffer);
	buffer=NULL;
        return 0;
    }


    /* 4. Walk backwards and get the destination first. */

    for (i = argc - 1; i > 0; i--)
    {
		error_code ec;
		coco_path_id tmp_path;


        if (argv[i][0] != '-')
        {
            desttarget = argv[i];


            /* 1. Determine if dest is native */

			ec = _coco_open(&tmp_path, desttarget, FAM_DIR | FAM_READ);

//			_coco_gs_pathtype(desttarget, &type);


			if (ec == 0)
			{
				targetDirectory = YES;

				_coco_close(tmp_path);
			}
			else
			{
				targetDirectory = NO;
			}

#if 0
            if (type == NATIVE)
            {
                /* Determine if this is a directory. */

                if  (stat(desttarget, &sb) == 0)
                {
                    if ((sb.st_mode & S_IFDIR) != 0)
                    {
                        /* Yes! directory! */
                        targetDirectory = YES;
                    }
                    else
                    {
                        /* Not Native directory! */
                        targetDirectory = NO;
                    }
                }
                else
                {
                    /* These is either a file system error or
                        the target is a file that will be created by
                        this copy. */

                        targetDirectory = NO;
                }
            }
            else if (type == OS9)
            {
                /* Determine if this is a  directory. */


                ec = _coco_open(&destpath, desttarget, FAM_DIR | FAM_READ);

                if (ec == 0)
                {
                    targetDirectory = YES;
                }
                else
                {
                    targetDirectory = NO;
                }

				_coco_close(destpath);
            }
#endif
            break;
        }
    }

    if (targetDirectory == NO && count > 2)
    {
        printf("Error: two or more sources requires target to be a directory.\n\n" );
        show_help(helpMessage);
	free(buffer);
	buffer=NULL;
        return(0);
    }

    /* Now look for the source files  */
    for (j = 1 ; j < i; j++)
    {
        if (argv[j][0] == '-')
            continue;

        if (argv[j] == NULL)
            continue;

        strcpy(df, desttarget);

        if (targetDirectory == YES)
        {
            /* OK, we need to add the filename to the end of the directory path */

            if (strchr(df, ',') != NULL)
            {
                /* OS-9 directory */
                if (df[strlen(df) - 1] != '/')
                {
                    if (df[strlen(df) - 1] != ',')
                        strcat(df, "/");
                }
            }
            else
            {
                /* Native directory */
                if (df[strlen(df) - 1] != '/' )
                    strcat(df, "/");
            }

            strcat(df, ExtractFilename(argv[j]));
        }

        ec = TSCopyFile(argv[j], df, eolTranslate, rewrite, owner, owner_set, buffer, buffer_size);

        if (ec != 0)
        {
            fprintf(stderr, "%s: error %d on file %s\n", argv[0], ec, argv[j]);
        }
    }

    if(buffer != NULL)
    {
	free(buffer);
	buffer=NULL;
    }
    return 0;
}



static char *ExtractFilename( char *path )
{
    /* This works for both native file paths and os9 file paths */

    char *a, *b;

    a = strchr(path, ',');

    if (a == NULL)
    {
        /* Native file */
        a = strrchr(path, '/');

        if (a == NULL)
            return path;

        return a + 1;
    }

    b = strrchr(a, '/');

    if (b == NULL)
        return a + 1;

    return b + 1;
}
