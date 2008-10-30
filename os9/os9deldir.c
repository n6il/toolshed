/********************************************************************
 * os9deldir.c - Delete file entry for OS-9 filesystems
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


static int do_deldir(char **argv, char *path, int interaction);


/* Help message */
static char *helpMessage[] =
{
    "Syntax: deldir {[<opts>]} {<directory>} {[<opts>]}\n",
    "Usage:  Delete a directory and its contents.\n",
    "Options:\n",
    "     -q    quiet mode (suppress interaction)\n",
    NULL
};



int os9deldir(int argc, char *argv[])
{
    error_code	ec = 0;
    char *p = NULL;
    int i, interaction = 0;

    /* walk command line for options */
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (p = &argv[i][1]; *p != '\0'; p++)
            {
                switch(*p)
                {
                    case 'q':
                        interaction = 1;
                        break;

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
		
        ec = do_deldir(argv, p, interaction);
		
        if( ec == 1) /* User quit */
            ec = 0;
			
        if (ec != 0)
        {
            fprintf(stderr, "%s: error %d deleting '%s'\n", argv[0], ec, p);
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


static int do_deldir(char **argv, char *path, int interaction)
{
	error_code	ec = 0;
	os9_path_id fold_path;
	char	*dirpath;
	fd_stats fdbuf;
	char	c[10];

	if (interaction == 0)
	{
		do
		{
			printf("\nDeleting directory: %s\n", path );
			printf("List directory, delete directory, or quit? (l/d/q) ");

			scanf("%s", c);

			switch( c[0] )
			{
				case 'l':
					{
						 char *argv[3];
						 argv[0] = "dir";
						 argv[1] = path;
						 argv[2] = NULL;

						 os9dir(2, argv);
					}
					break;

				case 'd':
					break;

				case 'q':
					return 1;

				default:
                    c[0] = 'l'; /* Force the user to type l, d, or q */
                    break;
            }
        } while (c[0] == 'l');
    }

    /* open a path to the device */
    ec = _os9_open(&fold_path, path, FAM_WRITE | FAM_DIR);
    if( ec != 0 )
    {
        return ec;
    }
	
    while (_os9_gs_eof(fold_path) == 0)
    {
        u_int size, i = 0;
        os9_dir_entry dentry;
        os9_path_id path2;

        size = sizeof(dentry);
        ec = _os9_read(fold_path, &dentry, &size);
        if (ec != 0)
            break;

        OS9StringToCString(dentry.name);
		
        /* Skip over dot directories and empty entries. */
        if (dentry.name[0] == '\0' )
            continue;
        if( strcmp((char *)dentry.name, "." ) == 0 )
            continue;
        if( strcmp((char *)dentry.name, ".." ) == 0 )
            continue;
		
        dirpath = malloc( strlen((char *)path ) + strlen((char *)dentry.name) + 2 );
        if( dirpath == NULL )
        {
            fprintf( stderr, "Not enough memory.\n" );
            return(1);
        }

        _os9_close( fold_path );

        strcpy( dirpath, path );
        strcat( dirpath, "/" );
        strcat( dirpath, (char *)dentry.name );

        /* Determine if file is really another directory */
        ec = _os9_open(&path2, dirpath, FAM_DIR | FAM_READ);
        if( ec == 0 )
        {
            /* Yup it is a directory, we need to delete it */
            _os9_close( path2 );
            ec = do_deldir( argv, dirpath, interaction );
			
            if( ec == 1 )
            {
                /* User asked to quit */
                free( dirpath );
                return( 1 );
            }
        }
		
        /* Delete file */
        ec = _os9_delete( dirpath );
        free( dirpath );
        
        /* Open orginal directory */
        ec = _os9_open( &fold_path, path, FAM_WRITE | FAM_DIR);
        ec = _os9_seek( fold_path, i*sizeof(fd_stats ), SEEK_SET );
		
        /* Incement directory entry count */
        i++;
    }
	
    /* All directory entried have been deleted.
        Turn off directory attribute, and delete directory file */
	   
    _os9_gs_fd( fold_path, sizeof(fd_stats), &fdbuf);
    fdbuf.fd_att &= ~FAP_DIR;
    _os9_ss_fd( fold_path, sizeof(fd_stats), &fdbuf );

    ec = _os9_close( fold_path );

    ec = _os9_delete( path );
	
    return( ec );
}
