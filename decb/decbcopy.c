/********************************************************************
 * decbcopy.c - Copy utility for Disk BASIC
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <cococonv.h>
#include <decbpath.h>
#include <sys/stat.h>


#define YES 1
#define NO 0

/* globals */
static u_int buffer_size = 32768;
static char *buffer;

static error_code CopyFile(char *srcfile, char *dstfile, int eolTranslate, int rewrite, int file_type, int data_type);
static char *GetFilename(char *path);
static EOL_Type DetermineEOLType(char *buffer, int size);
static void NativeToDECB(char *buffer, int size, char **newBuffer, int *newSize);
static void DECBToNative(char *buffer, int size, char **newBuffer, int *newSize);


/* Help message */
static char *helpMessage[] =
{
    "Syntax: copy {[<opts>]} <srcfile> {[<...>]} <target> {[<opts>]}\n",
    "Usage:  Copy one or more files to a target directory.\n",
    "Options:\n",
    "     -[0-3]     file type (when copying to a Disk BASIC image)\n",
	"     -[a|b]     data type (a = ASCII, b = binary)\n",
    "     -l         perform end of line translation\n",
    "     -r         rewrite if file exists\n",
    NULL
};



int decbcopy(int argc, char *argv[])
{
    error_code	ec = 0;
    char *p = NULL, *desttarget = NULL;
    int i, j;
    int targetDirectory = NO;
    int	count = 0;
    int	eolTranslate = 0;
    int	rewrite = 0;
	int file_type = 0, data_type = 0;
    char	df[256];


    /* 1. Walk command line for options. */
	
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (p = &argv[i][1]; *p != '\0'; p++)
            {
                switch(*p)
                {
                    case '0':
						file_type = 0;
                        break;
	
                    case '1':
						file_type = 2;
                        break;
	
                    case '2':
						file_type = 2;
                        break;
	
                    case '3':
						file_type = 3;
                        break;
	
                    case 'a':
						data_type = 255;
						break;
	
                    case 'b':
						data_type = 0;
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

        ec = CopyFile(argv[j], df, eolTranslate, rewrite, file_type, data_type);

        if (ec != 0)
        {
            fprintf(stderr, "%s: error %d\n", argv[0], ec);
        }
    }


    return 0;
}


static char *GetFilename(char *path)
{
    /* This works for both native file paths and os9 file paths */
	
    char *a, *b;
	
    a = strchr(path, ',');
	
    if (a == NULL)
    {
        /* Native file */
        a = strrchr(path, '/');
		
        if (a == NULL)
		{
            return path;
		}
		
        return a + 1;
    }
	
    b = strrchr(a, '/');
	
    if (b == NULL)
	{
        return a + 1;
	}
	
    return b + 1;
}



static error_code CopyFile(char *srcfile, char *dstfile, int eolTranslate, int rewrite, int file_type, int data_type)
{
    error_code	ec = 0;
    coco_path_id path;
    coco_path_id destpath;
    int		mode = FAM_NOCREATE | FAM_WRITE;
	

    /* 1. Set mode based on rewrite. */
	
    if (rewrite == 1)
    {
        mode &= ~FAM_NOCREATE;
    }


    /* 2. Open a path to the srcfile. */

    ec = _coco_open(&path, srcfile, FAM_READ);

    if (ec != 0)
	{
        return ec;
	}


    /* 3. Attempt to create the destfile. */
	
    ec = _coco_create(&destpath, dstfile, mode, FAP_READ | FAP_WRITE | FAP_PREAD);

    if (ec != 0)
    {
        _coco_close(path);

        return ec;
    }


    while (_coco_gs_eof(path) == 0)
    {
        char *newBuffer;
        int newSize;
        int size = buffer_size;

        ec = _coco_read(path, buffer, &size);

        if (ec != 0)
        {
            break;
        }


        if (eolTranslate == 1)
        {
            if (path->type == NATIVE && destpath->type != NATIVE)
            {
                /* source is native, destination is coco */

                NativeToDECB(buffer, size, &newBuffer, &newSize);

                ec = _coco_write(destpath, newBuffer, &newSize);

                free(newBuffer);
            }
            else if (path->type != NATIVE && destpath->type == NATIVE)
            {
                /* source is coco, destination is native */
                
                DECBToNative(buffer, size, &newBuffer, &newSize);
                ec = _coco_write(destpath, newBuffer, &newSize);

                free(newBuffer);
            }
        }
        else
        {
            /* One-to-one writing of the data -- no translation needed. */
            
            ec = _coco_write(destpath, buffer, &size);
        }

        if (ec != 0)
        {
            break;
        }
    }
	

    /* Copy meta data from file descriptor of source to destination */
	
	{
		coco_file_stat fd;
		
		
		_coco_gs_fd(path, &fd);

		_coco_ss_fd(destpath, &fd);
	}
	
	
	/* Special -- if this is a DECB file we wrote to, set passed file type and data type. */
	
	{
		_path_type t;
		
		_coco_gs_pathtype(destpath, &t);
		
		if (t == DECB)
		{
			decb_file_stat f;
			
			
			_decb_gs_fd(destpath->path.decb, &f);
			
			f.file_type = file_type;
			f.data_type = data_type;
			
			_decb_ss_fd(destpath->path.decb, &f);
		}
	}
	
	 
    _coco_close(path);
    _coco_close(destpath);

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
 * Converts a buffer containing native EOLs to one with Disk BASIC EOLs.
 *
 * The caller must free the returned buffer in 'newBuffer' once
 * finished with the buffer.
 */
static void NativeToDECB(char *buffer, int size, char **newBuffer, int *newSize)
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
            return;
    }
    
    
    return;
}


static void DECBToNative(char *buffer, int size, char **newBuffer, int *newSize)
{
#ifdef _WIN32
    int dosEOLCount = 0;
    char *newP;
    int		i;


    /* Things are a bit more involved here. */
            
    /* We will add 0x0As after all 0x0Ds. */
            

    /* 1. First we count up the number of 0x0D Disk BASIC line endings. */

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
