/********************************************************************
 * decb_open.c - Disk BASIC open/create routines
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>

#include "decbpath.h"
#include "errno.h"
#include "dirent.h"



static int init_pd(decb_path_id *path, int mode);
static int term_pd(decb_path_id path);
static int validate_pathlist(decb_path_id *path, char *pathlist);
static int _decb_cmp(decb_dir_entry *entry, char *name);



/*
 * _decb_create()
 *
 * Create a file
 */
error_code _decb_create(decb_path_id *path, char *pathlist, int mode, int file_type, int data_type)
{
	error_code	ec = EOS_BPNAM;


#if 0
    /* 1. Allocate & initialize path descriptor. */

    ec = init_pd(path, mode);

    if (ec != 0)
    {
        return ec;
    }


    /* 2. Attempt to validate the pathlist. */
	
    ec = validate_pathlist(path, pathlist);

    if (ec != 0)
    {
        term_pd(*path);

        return ec;
    }
#endif
	
    return ec;
}



/*
 * _decb_open()
 *
 * Open a path to a file
 *
 * Legal pathnames are:
 *
 * 1. imagename,      (considered to be a 'raw' open of the image)
 * 2. imagename,file  (considered to be a file open within the image)
 * 3. imagename       (considered to be an error) 
*/
error_code _decb_open(decb_path_id *path, char *pathlist, int mode)
{
	error_code	ec = 0;
	char *open_mode;


	/* 1. Strip off FAM_NOCREATE if passed -- irrelavent to _decb_open */
	
	mode = mode & ~FAM_NOCREATE;
	
	
	/* 2. Allocate & initialize path descriptor */
	
	ec = init_pd(path, mode);

	if (ec != 0)
	{
		return ec;
	}


	/* 3. Attempt to validate the pathlist */
	
	ec = validate_pathlist(path, pathlist);
	
	if (ec != 0)
	{
		term_pd(*path);

		return ec;
	}
	
	
	/* 4. Determine if disk is being open in raw mode. (We know it is raw mode if there is no filename). */

	if (*(*path)->filename == '\0')
	{
		/* 1. Yes, raw mode */

		(*path)->israw = 1;
	}
	else
	{
		/* 1. No, normal mode */

		if ((*path)->mode & FAM_DIR)
		{
			term_pd(*path);

			return EOS_BMODE;
		}
		
		(*path)->israw = 0;
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


	(*path)->disk_offset = 161280 * (*path)->drive;
	
	
	/* 6. At this point, sector and granule function will work - Load FAT */

	_decb_gs_sector(*path, 17, 2, (*path)->FAT);


	/* 7. If path is raw, just return now. */
	
	if ((*path)->israw == 1)
	{
		return 0;
	}
	

	/* 8. Find directory entry matching filename. */

	{
		decb_dir_entry entry;
		int mode = (*path)->mode;
		
		
		(*path)->mode |= FAM_DIR;
		
		
		/* 1. Seek to the first directory entry. */

		_decb_seekdir(*path, 0);
		
		
		/* 2. Check each entry until we find a match. */
		
		while ((ec = _decb_readdir(*path, &entry)) == 0)
		{
			if (_decb_cmp(&entry, (*path)->filename) == 0)
			{
				/* 1. We have a match! */
				
				(*path)->directory_entry_index--;

				(*path)->this_directory_entry_index = (*path)->directory_entry_index;
								
				(*path)->first_granule = entry.first_granule;
				
				(*path)->bytes_in_last_sector = int2(entry.last_sector_size);
				
				break;
			}
		}

		(*path)->mode = mode;
		
		
		if (ec != 0)
		{
			ec = EOS_PNNF;
		}
	}
	

	/* 9. Return status. */
	
	return(ec);
}



/*
 * _decb_close()
 *
 * Close a path to a file
 */
error_code _decb_close(decb_path_id path)
{
	error_code	ec = 0;
	

	/* 1. Write out FAT sector. */
	
	_decb_ss_sector(path, 17, 2, path->FAT);
	
	
	/* 2. Terminate path descriptor */
	
	ec = term_pd(path);
	
	
	/* 3. Return status. */
	
	return(ec);
}



static int _decb_cmp(decb_dir_entry *entry, char *name)
{
	char modified_name[13];
	char *filename = entry->filename;
	char *ext = entry->file_extension;
	char *p = modified_name;
	int count = 0;
	
	
	/* 1. Copy filename. */
	
	while (*filename != ' ' && count++ < 8)
	{
		*(p++) = *(filename++);
	}


	/* 2. If an extension exists, add it. */
	
	if (ext[0] != ' ')
	{
		*(p++) = '.';
		
		count = 0;
	
		/* 1. Copy extension. */
	
		while (*ext != ' ' && count++ < 3)
		{
			*(p++) = *(ext++);
		}
	}
	
	*p = '\0';
	
	
	return (strcasecmp(modified_name, name));
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
 *	foo,:2			Opens third disk in foo for raw access
 *	foo,bar.bas		Opens file bar.bas in disk image foo
 *	foo,bar.bin:1	Opens file bar.bin in second disk image in file foo
 */
static int validate_pathlist(decb_path_id *path, char *pathlist)
{
	error_code  ec = 0;
	char		*comma;
	int			count;

	
	/* 1. Validate the pathlist. */
	
    if ((comma = strchr(pathlist, ',')) == NULL)
    {
        /* 1. No native/RS-DOS delimiter in pathlist, return error. */

		ec = EOS_BPNAM;
    }
	else
	{
		/* 1. Extract information out of pathlist. */
	
		count = sscanf(pathlist, "%512[^,]%*c%64[^:]%*c%d", (*path)->imgfile, (*path)->filename, &((*path)->drive));
	
		if (count < 3)
		{
			count = sscanf(pathlist, "%512[^,]%*c%*c%d", (*path)->imgfile, &((*path)->drive));
		}
	}


	/* 2. Return. */
	
    return ec;
}



static int init_pd(decb_path_id *path, int mode)
{
	/* 1. Allocate path structure and initialize it. */
	
	*path = malloc(sizeof(struct _decb_path_id));
	
	if (*path == NULL)
	{
		return 1;
	}


	/* 2. Clear out newly allocated path structure. */

	memset(*path, 0, sizeof(**path));

	(*path)->mode = mode;


	/* 3. Return. */
	
	return 0;
}



static int term_pd(decb_path_id path)
{
	/* 1. Deallocate path structure. */
	
	free(path);


	/* 2. Return. */
	
	return 0;
}
