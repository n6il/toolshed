/********************************************************************
 * decblist.c - List utility for Disk BASIC
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
	"Usage:  Display contents of a text file. List will also\n",
	"        de-tokenize a binary Color BASIC file.\n",
	"Options:\n",
	NULL
};


int decblist(int argc, char *argv[])
{
	error_code	ec = 0;
	char *p = NULL;
	coco_path_id path;
	int i;
	unsigned char *buffer;
	int size;
	

	/* 1. Walk command line for options */
	
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

	
	/* 2. Walk command line for pathnames. */
	
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

	/* 3. Open a path to the file. */
	
	ec = _coco_open(&path, p, FAM_READ);

	if (ec != 0)
	{
		_coco_close(path);
		printf("Error %d opening %s\n", ec, p);

		return(ec);
	}

	ec = _coco_gs_size( path, &size );
	
	buffer = malloc( size );
	
	if( buffer == NULL )
	{
		/* Memory error */
		return -1;
	}
	
	ec = _coco_read(path, buffer, &size);
	if (ec != 0)
	{
		return -1;
	}

	if( _decb_detect_tokenized( buffer, size ) == 0 )
	{
		char *program;
		int program_size;
		
		ec = _decb_detoken( buffer, size, &program, &program_size);
		if (ec != 0)
		{
			return ec;
		}
		
		free( buffer );
		buffer = program;
		size = program_size;
	}

	printf( "%s", buffer );

	free( buffer );
	
	_coco_close(path);

	return(0);
}
