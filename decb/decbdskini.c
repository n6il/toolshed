/********************************************************************
 * decbdskini.c - Disk BASIC Format Utility
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <decbpath.h>
#include <nativepath.h>
#include <cocotypes.h>


#define BUFFSIZ	256

static int do_dskini(char **argv, char *vdisk, int tracks, char *diskName);

/* Help message */
static char *helpMessage[] =
{
	"Syntax: dskini {[<opts>]} <disk> {[<...>]} {[<opts>]}\n",
	"Usage:  Create a Disk BASIC image.\n",
	"Options:\n",
	"     -3     = 35 track disk (default)\n",
	"     -4     = 40 track disk\n",
	"     -8     = 80 track disk\n",
	"     -nname = HDB-DOS disk name\n",
	NULL
};


int decbdskini(int argc, char **argv)
{
	error_code	ec = 0;
	char *p = NULL;
	int i;
	int tracks = 35;
	char *diskName = NULL;


	/* 1. If no arguments, show help and return. */
	
	if (argv[1] == NULL)
	{
		show_help(helpMessage);

		return(0);
	}


	/* 2. Walk command line for options. */
	
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (p = &argv[i][1]; *p != '\0'; p++)
			{
				switch(*p)
				{
					case '3':	/* 35 tracks */
						tracks = 35;
						break;

					case '4':	/* 40 tracks */
						tracks = 40;
						break;

					case '8':	/* 80 tracks */
						tracks = 80;
						break;

					case 'n':	/* disk name */
						diskName = p + 1;
						while (*(p + 1) != '\0') p++;
						break;

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


	/* 3. Walk command line for pathnames. */
	
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			continue;
		}
		else
		{
			do_dskini(argv, argv[i], tracks, diskName);
		}
	}

	return(ec);
}



static int do_dskini(char **argv, char *vdisk, int tracks, char *diskName)
{
	error_code	ec = 0;
	native_path_id nativepath;
	int max_s;
	char sector[256];


	/* 1. Open a path to the virtual disk. */
	
	ec = _native_open(&nativepath, vdisk, FAM_WRITE);

	if (ec != 0)
	{
		ec = _native_create(&nativepath, vdisk, FAM_READ | FAM_WRITE, FAP_READ | FAP_WRITE);

		if (ec != 0)
		{
			fprintf(stderr, "%s: cannot open virtual disk\n", argv[0]);

			return(ec);
		}
	}

	_native_seek(nativepath, 0, SEEK_SET);


	/* 2. Write 17 tracks of $FF */

	memset(sector, 0xFF, 256);

	{
		int t, s;
		
		for (t = 0; t < 17; t++)
		{
			for (s = 1; s < 19; s++)
			{
				int size = 256;
				
				_native_write(nativepath, sector, &size);
			}
		}
	}


	/* 4. Write directory track. */

	memset(sector, 0x00, 256);

	{
		int s;
		int size;
		
		
		/* 1. Write sector of track 17 (all 0s..). */
		
		size = 256;
				
		_native_write(nativepath, sector, &size);


		/* 2. Write FAT sector. */
		
		switch (tracks)
		{
			case 40:
				max_s = 78;
				break;
				
			case 80:
				max_s = 156;
				break;

			case 35:
			default:
				max_s = 68;
				break;
		}

		for (s = 0; s < max_s; s++)
		{
			sector[s] = 0xFF;
		}

		size = 256;
				
		_native_write(nativepath, sector, &size);


		/* 3. Write 14 sectors of 0xFF. */
		
		memset(sector, 0xFF, 256);

		for (s = 0; s < 14; s++)
		{
			size = 256;
				
			_native_write(nativepath, sector, &size);
		}
		

		/* 4. If disk name was provided, copy it to sector. */
		
		if (diskName != NULL)
		{
			strcpy(sector, diskName);
		}


		/* 5. Write 17th sector. */

		size = 256;
				
		_native_write(nativepath, sector, &size);


		memset(sector, 0xFF, 256);


		/* 6. Write 18th sector. */

		size = 256;
				
		_native_write(nativepath, sector, &size);
	}


	/* 5. Write remaining 17, 22 or 62 tracks of $FF. */

	memset(sector, 0xFF, 256);

	{
		int t, s;
		
		for (t = 0; t < tracks - 18; t++)
		{
			for (s = 1; s < 19; s++)
			{
				int size = 256;
				
				_native_write(nativepath, sector, &size);
			}
		}
	}
	

	_native_close(nativepath);


	return(ec);
}
