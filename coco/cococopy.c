/********************************************************************
 * os9copy.c - Copy utility for OS-9 filesystem
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <cocotypes.h>
#include <cocopath.h>
#include <sys/stat.h>


#define YES 1
#define NO 0

/* globals */
static u_int buffer_size = 32768;
static char *buffer;

static error_code CopyFile( char *srcfile, char *dstfile, int eolTranslate, int rewrite );
static char *GetFilename( char *path );
static EOL_Type DetermineEOLType(char *buffer, int size);
static void NativeToOS9(char *buffer, int size, char **newBuffer, int *newSize);
static void OS9ToNative(char *buffer, int size, char **newBuffer, int *newSize);


/* Help message */
static char const * const helpMessage[] =
{
    "Syntax: copy {[<opts>]} <srcfile> {[<...>]} <target> {[<opts>]}\n",
    "Usage:  Copy one or more files to a target directory.\n",
    "Options:\n",
    "     -b=size    size of copy buffer in bytes or K-bytes\n",
    "     -l         perform end of line translation\n",
    "     -r         rewrite if file exists\n",
    NULL
};

int os9copy(int argc, char *argv[])
{
    error_code	ec = 0;
    char *p = NULL, *q, *desttarget = NULL;
    os9_path_id destpath;
    int i, j;
    int targetDirectory = NO;
    int	count = 0;
    int	eolTranslate = 0;
    int	rewrite = 0;
    char	df[256];
	
    /* walk command line for options */
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

    /* allocate memory for the copy buffer */
    buffer = (char *)malloc(buffer_size);
    if (buffer == NULL)
    {
        fprintf(stderr, "%s: cannot allocate %d byte of buffer memory\n", argv[0], buffer_size);
        return(1);
    }

    /* Count non option arguments */
    for( i=1, count=0; i<argc; i++ )
    {
        if( argv[i] == NULL )
            continue;
		
        if( argv[i][0] == '-' )
            continue;
		
        count++;
    }

    if( count == 0 )
    {
        show_help(helpMessage);
        return(0);
    }

    /* Walk backwards and get the destination first. */
	
    for( i = argc-1; i > 0; i-- )
    {
        if( argv[i][0] != '-' )
        {
            struct stat sb;
			
            desttarget = argv[i];
			
            /* Determine if dest is native */
            if( strchr( desttarget, ',') == NULL )
            {
                /* Determine if this is a native directory */
                if( stat( desttarget, &sb ) == 0 )
                {
                    if( (sb.st_mode && S_IFDIR) != 0 )
                    {
                        /* Native directory! */
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
                        this os9copy. */
					   
                        targetDirectory = NO;
                }
            }
            else
            {
                /* Determine if this is an OS-9 directory */
                ec = _os9_open( &destpath, desttarget, FAM_DIR | FAM_READ );
				
                if( ec == 0 )
                {
                    _os9_close( destpath );
                    targetDirectory = YES;
                }
                else
                {
                    targetDirectory = NO;
                }
            }
            break;
        }
    }
	
    if( targetDirectory == NO && count > 2 )
    {
        printf("Error: two or more sources requires target to be a directory.\n\n" );
        show_help(helpMessage);
        return(0);
    }
	
    /* Now look for the source files  */
    for (j = 1 ; j < i; j++)
    {
        if (argv[j][0] == '-')
            continue;
		
        if( argv[j] == NULL )
            continue;

        strcpy( df, desttarget );
		
        if( targetDirectory == YES )
        { 
            /* OK, we need to add the filename to the end of the directory path */
			
            if( strchr( df, ',' ) != NULL )
            {
                /* OS-9 directory */
                if( df[ strlen( df )-1 ] != '/' )
                {
                    if( df[ strlen( df )-1 ] != ',' )
                        strcat( df, "/" );
                }
            }
            else
            {
                /* Native directory */
                if( df[ strlen( df )-1 ] != '/' )
                    strcat( df, "/" );
            }
			
            strcat( df, GetFilename( argv[j] ) );
        }

        ec = CopyFile( argv[j], df, eolTranslate, rewrite );
        if( ec != 0 )
        {
            fprintf(stderr, "%s: error %d creating '%s'\n", argv[0], ec, df);
        }
    }

    return( 0 );
}

static char *GetFilename( char *path )
{
    /* This works for both native file paths and os9 file paths */
	
    char *a, *b;
	
    a = strchr( path, ',' );
	
    if( a == NULL )
    {
        /* Native file */
        a = strrchr( path, '/' );
		
        if( a == NULL )
            return path;
		
        return a+1;
    }
	
    b = strrchr( a, '/' );
	
    if( b == NULL )
        return a+1;
	
    return b+1;
}

static error_code CopyFile( char *srcfile, char *dstfile, int eolTranslate, int rewrite )
{
    error_code	ec = 0;
    os9_path_id path;
    os9_path_id destpath;
    fd_stats	srcFD, destFD;
    int		mode = FAM_NOCREATE | FAM_WRITE;
	
    /* set mode based on rewrite */
    if (rewrite == 1)
    {
        mode &= ~FAM_NOCREATE;
    }

    /* open a path to the srcfile */
    ec = _os9_open(&path, srcfile, FAM_READ);
    if (ec != 0)
        return( ec );

    /* attempt to create the destfile */
    ec = _os9_create(&destpath, dstfile, mode, FAP_PREAD | FAP_READ | FAP_WRITE);
    if (ec != 0)
    {
        _os9_close( path );
        return( ec );
    }

    while (_os9_gs_eof(path) == 0)
    {
        char *newBuffer;
        int newSize;
        int size = buffer_size;

        ec = _os9_read(path, buffer, &size);

        if (ec != 0)
        {
            break;
        }

        if (eolTranslate == 1)
        {
            if (path->isnative == 1 && destpath->isnative == 0)
            {
                /* source is native, destination is OS-9 */

                NativeToOS9(buffer, size, &newBuffer, &newSize);

                ec = _os9_write(destpath, newBuffer, &newSize);

                free(newBuffer);
            }
            else if (path->isnative == 0 && destpath->isnative == 1)
            {
                /* source is OS-9, destination is native */
                
                OS9ToNative(buffer, size, &newBuffer, &newSize);
                ec = _os9_write(destpath, newBuffer, &newSize);

                free(newBuffer);
            }
        }
        else
        {
            /* One-to-one writing of the data -- no translation needed. */
            
            ec = _os9_write(destpath, buffer, &size);
        }

        if (ec != 0)
        {
            break;
        }
    }
	
    /* Copy meta data from file descriptor of source to destination */
	
    _os9_gs_fd(path, sizeof( fd_stats ), &srcFD);
    _os9_gs_fd(destpath, sizeof( fd_stats ), &destFD);
	
    destFD.fd_att = srcFD.fd_att;
    /* Note, the code change below forces copy to copy all files as root and not as the source file's owner. */
#if 0
    memcpy( &(destFD.fd_own), &(srcFD.fd_own), 2 );
#else
    memset( &(destFD.fd_own), sizeof(destFD.fd_own), 0 );
#endif
    memcpy( &(destFD.fd_dat), &(srcFD.fd_dat), 5 );
    memcpy( &(destFD.fd_creat), &(srcFD.fd_creat), 3 );
	
    _os9_ss_fd( destpath, sizeof( fd_stats ), &destFD );
	 
    _os9_close(path);
    _os9_close(destpath);

    return(ec);
}



/*
 * Scan a buffer to determine the type of end-of-line termination it has.
 *
 * Returns EOL_DOS, EOL_UNIX or EOL_OS9
 */
static EOL_Type DetermineEOLType(char *buffer, int size)
{
    EOL_Type eol = 0;
    int i;
    
    
    /* Scan to determine EOL ending type */
    
    for (i = 0; i < size; i++)
    {
        if (i < size - 1 && (buffer[i] == 0x0D && buffer[i + 1] == 0x0A))
        {
            /* We have DOS/Windows line endings (0D0A)... */
            eol = EOL_DOS;

            break;
        }

        if (buffer[i] == 0x0A)
        {
            /* We have unix line endings. */
            eol = EOL_UNIX;

            break;
        }
        
        if (buffer[i] == 0x0D)
        {
            /* We have OS-9 line endings. */
            eol = EOL_OS9;
            
            break;
        }
    }


    return eol;
}



/*
 * Converts a buffer containing native EOLs to one with OS-9 EOLs.
 *
 * The caller must free the returned buffer in 'newBuffer' once
 * finished with the buffer.
 */
static void NativeToOS9(char *buffer, int size, char **newBuffer, int *newSize)
{
    EOL_Type	eolMethod;
    int		i;


    eolMethod = DetermineEOLType(buffer, size);
    
    switch (eolMethod)
    {
        case EOL_UNIX:
            /* Change all occurences of 0x0A to 0x0D */
            
            for(i = 0; i < size; i++)
            {
                if (buffer[i] == 0x0A)
                {
                    buffer[i] = 0x0D;
                }
            }
            *newBuffer = (char *)malloc(size);
            if (*newBuffer == NULL)
            {
                return;
            }
            
            memcpy(*newBuffer, buffer, size);
            
            *newSize = size;
            
            break;

        case EOL_DOS:
            /* Things are a bit more involved here. */
            
            /* We will strip all 0x0As out of the buffer, leaving the 0x0Ds. */
            
            {
                int dosEOLCount = 0;
                char *newP;
                int i;


                /* 1. First we count up the number of 0x0A line endings. */

                for (i = 0; i < size; i++)
                {
                    if (buffer[i] == 0x0A)
                    {
                        dosEOLCount++;
                    }
                }


                /* 2. Now we allocate a buffer to hold the current size -
                    'dosEOLCount' bytes.
                */

                *newSize = size - dosEOLCount;
                
                *newBuffer = (char *)malloc(*newSize);
                
                if (*newBuffer == NULL)
                {
                    return;
                }

                newP = *newBuffer;
                
                for (i = 0; i < size; i++)
                {
                    if (buffer[i] != 0x0A)
                    {
                        *newP = buffer[i];
                        newP++;
                    }
                }
            }
            break;

        default:
            /* No eols, binary copy */

            *newBuffer = (char *)malloc(size);
            if (*newBuffer == NULL)
            {
                return;
            }
            
            memcpy(*newBuffer, buffer, size);
            
            *newSize = size;
    }
    
    
    return;
}


static void OS9ToNative(char *buffer, int size, char **newBuffer, int *newSize)
{
#ifdef _WIN32
    int dosEOLCount = 0;
    char *newP;
    int		i;


    /* Things are a bit more involved here. */
            
    /* We will add 0x0As after all 0x0Ds. */
            

    /* 1. First we count up the number of 0x0D OS-9 line endings. */

    for (i = 0; i < size; i++)
    {
        if (buffer[i] == 0x0D)
        {
            dosEOLCount++;
        }
    }


    /* 2. Now we allocate a buffer to hold the current size +
        'dosEOLCount' bytes.
    */
    
    *newSize = size + dosEOLCount;
    *newBuffer = (char *)malloc(*newSize);
                
    if (*newBuffer == NULL)
    {
        return;
    }

    newP = *newBuffer;
    
    for (i = 0; i < size; i++)
    {
        *newP = buffer[i];
        newP++;

        if (buffer[i] == 0x0D)
        {
            *newP = 0x0A;
            newP++;
        }
    }
#else
    int		i;


    /* Change all occurences of 0x0D to 0x0A */
            
    for(i = 0; i < size; i++)
    {
        if (buffer[i] == 0x0D)
        {
            buffer[i] = 0x0A;
        }
    }

    *newBuffer = (char *)malloc(size);
    if (*newBuffer == NULL)
    {
        return;
    }
            
    memcpy(*newBuffer, buffer, size);
            
    *newSize = size;
#endif


    return;
}
