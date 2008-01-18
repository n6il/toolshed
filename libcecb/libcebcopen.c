/********************************************************************
 * libcebcopen.c - Cassette BASIC open/create routines
 *
 * $Id$
 ********************************************************************/

#include "cecbpath.h"

double cecb_threshold = 0;
double cecb_frequency = 0;
_wave_parity cecb_wave_parity = NONE;
long cecb_start_sample = 0;

static error_code parse_header( cecb_path_id path  );
static error_code validate_pathlist(cecb_path_id path, char *pathlist);
static int init_pd(cecb_path_id *path, int mode);
static int term_pd(cecb_path_id path);

/*
 * _cecb_create()
 *
 * Create a file
 */

error_code _cecb_create(cecb_path_id *path, char *pathlist, int mode, int file_type, int data_type, int gap, int ml_load_address, int ml_exec_address)
{
	error_code		ec = EOS_BPNAM;
	char			*open_mode;

    /* 1. Allocate & initialize path descriptor. */

    ec = init_pd(path, mode);

    if (ec != 0)
    {
        return ec;
    }

    /* 2. Attempt to validate the pathlist. */
	
    ec = validate_pathlist(*path, pathlist);

    if (ec != 0)
    {
        term_pd(*path);

        return ec;
    }

	(*path)->mode = mode;
	
	/* 3. Open a path to the image file. */
	
	if (mode & FAM_WRITE)
	{
		open_mode = "rb+";
	}
	else
	{
		open_mode = "rb";
	}
	
	(*path)->fd = fopen((*path)->imgfile, open_mode);
	
	if ((*path)->fd == NULL)
	{
		term_pd(*path);
		
		return(EOS_BPNAM);
	}

	/* 4. Open and determine CAS or WAV and fill in data structors */
	
	/* 5. Seek to end of data */
	
	/* 6. Fill in dir_entry */
	
	/* 7. Write half second of silence */
	
	/* 8. Write leader, dir_entry */
	
	/* 9. Write gap, leader */

	return ec;
}

/*
 * _cecb_open()
 *
 * Open a path to a file
 *
 * Legal pathnames are:
 *
 * 1. imagename,      (considered to be a 'raw' open of the image)
 * 2. imagename,file  (considered to be a file open within the image)
 * 3. imagename       (considered to be an error) 
*/

error_code _cecb_open(cecb_path_id *path, char *pathlist, int mode )
{
	error_code	ec = 0;
	char *open_mode;

	/* 0. Currently -- no writing supported */

	if( (mode & FAM_WRITE) == FAM_WRITE )
		return EOS_IC;

	/* 1. Strip off FAM_NOCREATE if passed -- irrelavent to _cecb_open */
 
 	mode = mode & ~FAM_NOCREATE;
 	
	/* 2. Allocate & initialize path descriptor */
	
	ec = init_pd(path, mode);

	if (ec != 0)
	{
		return ec;
	}

	/* 3. Attempt to validate the pathlist */
	
	ec = validate_pathlist(*path, pathlist);
	
	if (ec != 0)
	{
		term_pd(*path);

		return ec;
	}
	
	
	/* 4. Determine if Cassette is being open in raw mode. (We know it is raw mode if there is no filename). */

	if ( strncmp( (*path)->filename, "        ", 8 ) == 0 )
	{
		/* 1. Yes, raw mode */

		(*path)->israw = 1;
	}
	else
	{
		(*path)->israw = 0;
		
		/* If mode is FAM_DIR, then we need to error */
		if (mode & FAM_DIR)
		{
			term_pd(*path);
			return EOS_SN;
		}
	}


	/* 5. Open a path to the image file. */
	
	if (mode & FAM_WRITE)
	{
		open_mode = "rb+";
	}
	else
	{
		open_mode = "rb";
	}

	(*path)->fd = fopen((*path)->imgfile, open_mode);

	if ((*path)->fd == NULL)
	{
		term_pd(*path);

		return(EOS_BPNAM);
	}
	
	/* 6. Open and determine CAS or WAV and fill in data structors */

	(*path)->play_at = cecb_start_sample;
	(*path)->wav_threshold = cecb_threshold;
	(*path)->wav_frequency_limit = cecb_frequency;
	(*path)->wav_parity = cecb_wave_parity;
	
	ec = parse_header( *path );

	if (ec != 0)
	{
		term_pd(*path);

		return ec;
	}
	
	/* if raw, exit */
	
	if( (*path)->israw == 1 )
		return ec;
	
	/* Find file */
	
	while( ec == 0)
	{
		ec = _cecb_read_next_dir_entry( *path, &((*path)->dir_entry) );
		
		if( ec == 0 )
		{
			if( strncmp( (char *)(*path)->dir_entry.filename, (*path)->filename, 8 ) == 0 )
			{
				/* File found, save the place of the start of data */
				
				if( (*path)->tape_type == CAS )
				{
					(*path)->cas_start_byte = (*path)->cas_current_byte;
					(*path)->cas_start_bit = (*path)->cas_current_bit;
				}
				else if( (*path)->tape_type == WAV )
					(*path)->wav_start_sample = (*path)->wav_current_sample;
					
				break;
			}
		}
	}
	
	return ec;
}

/*
 * _cecb_close()
 *
 * Close a path to a file
 */
 
error_code _cecb_close(cecb_path_id path)
{
	error_code	ec = 0;
		
	/* if data was written, write end block */
	
	/* Write half second of silence */

	/* Close path. */

	fclose(path->fd);


	/* Terminate path descriptor */
	
	ec = term_pd(path);
	
	
	/* Return status. */
	
	return(ec);
	
}

/*
 * parse_header()
 *
 * Determines if file is CAS or WAV.
 *
 * If necessary reads WAV headers to fill in data structurs
 */

static error_code parse_header( cecb_path_id path  )
{
	error_code ec = 0;
		
	if( strendcasecmp( path->imgfile, CAS_FILE_EXTENSION ) == 0 )
		ec = _cecb_parse_cas( path );
	else
		ec = _cecb_parse_riff( path );

	return ec;
}
 
 
/*
 * validate_pathlist()
 *
 * Determines if the passed <image,path:drive> pathlist is valid.
 *
 * Copies the image file and pathlist file portions into
 * the path descriptor.
 *
 * Valid pathlist examples:
 *	foo,			Opens foo for raw access
 *	foo,bar			Opens file bar in Cassette image foo
 */

static error_code validate_pathlist(cecb_path_id path, char *pathlist)
{
	error_code  ec = 0;
	char		*p;
	int			i;

	/* 1. Validate the pathlist. */

	if ((p = strchr(pathlist, ',')) == NULL)
	{
		/* 1. No native/Cassette delimiter in pathlist, return error. */

		ec = EOS_BPNAM;
	}
	else
	{
		/* 1. Extract information out of pathlist. */

		*p = '\0';
		strncpy(path->imgfile, pathlist, 512);
		*p = ',';
		p++;

		strncpy(path->filename, p, 8);		
		
		/* Space fill remaining characters */
		for( i=strlen(p); i<8; i++ )
			path->filename[i] = ' ';
			
	}

	/* 2. Return. */

	return ec;
}

static int init_pd(cecb_path_id *path, int mode)
{
	/* 1. Allocate path structure and initialize it. */
	
	*path = malloc(sizeof(struct _cecb_path_id));
	
	if (*path == NULL)
	{
		return 1;
	}


	/* 2. Clear out newly allocated path structure. */

	memset(*path, 0, sizeof(struct _cecb_path_id));
	
	(*path)->mode = mode;

	/* 3. Return. */
	
	return 0;
}

static int term_pd(cecb_path_id path)
{
	/* 1. Deallocate path structure. */
	
	free(path);

	/* 2. Return. */
	
	return 0;
}

