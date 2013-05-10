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
#include "util.h"


static int init_pd(decb_path_id *path, int mode);
static int term_pd(decb_path_id path);
static int validate_pathlist(decb_path_id *path, char *pathlist);
static int _decb_cmp(decb_dir_entry *entry, char *name);

extern error_code find_free_granule(decb_path_id path, int *granule, int next_to);


/*
 * _decb_create()
 *
 * Create a file
 */

error_code _decb_create(decb_path_id *path, char *pathlist, int mode, int file_type, int data_type)
{
	error_code		ec = EOS_BPNAM;
	int				empty_entry = -1;
	char			*open_mode;
	

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
	
	
	(*path)->disk_offset = 161280 * (*path)->drive;
	(*path)->disk_offset += (*path)->hdbdos_offset;
	
	
	/* 4. At this point, sector and granule function will work - Load FAT */
	
	_decb_gs_sector(*path, 17, 2, (char *)(*path)->FAT);
	
	
	/* 5. Determine if there is enough space. */

	{
		int			i, free_granules = 0;

	
		for (i = 0; i < 256; i++)
		{
			if ((*path)->FAT[i] == 0xFF)
			{
				free_granules++;
				
				break;
			}
		}
		
		if (free_granules == 0)
		{
			fclose((*path)->fd);
			
			term_pd(*path);

			return EOS_DF;
		}
	}
	

	/* 6. Construct a directory entry. */
	
	{
		char *p = strchr((*path)->filename, '.');
		
		
		/* 1. Clear memory. */
		
		memset(&((*path)->dir_entry), 0, sizeof(decb_dir_entry));
		memset(&((*path)->dir_entry), 0x20, 11);
		
		
		if (p == NULL)
		{
			int length = strlen((*path)->filename);
			
			if (length > 8) length = 8;
			
			strncpy((char *)(*path)->dir_entry.filename, (*path)->filename, length);
		}
		else
		{
			int length = p - (*path)->filename;
			
			
			if (length > 8)
			{
				length = 8;
			}
			
			strncpy((char *)(*path)->dir_entry.filename, (*path)->filename, length);
			
			
			p++; /* skip over '.' */
			
			length = strlen(p);
			
			if (length > 3)
			{
				length = 3;
			}
			
			strncpy((char *)(*path)->dir_entry.file_extension, p, length);
		}
		
		(*path)->dir_entry.file_type = file_type;
		
		(*path)->dir_entry.ascii_flag = data_type;
	}

	
	/* 7. Determine if file already exists. */

	{
		decb_dir_entry		de;
		
		
		/* 1. Find an empty dir (and check for duplicate entry too) */

		_decb_seekdir(*path, 0, SEEK_SET);
		
		while (_decb_readdir(*path, &de) == 0)
		{
			/* 1. If we run across an empty directory entry, make a note of it. */
			
			if ((de.filename[0] == 255 || de.filename[0] == '\0') && empty_entry == -1)
			{
				empty_entry = (*path)->directory_entry_index - 1;
				
				(*path)->this_directory_entry_index = (*path)->directory_entry_index;
			}

			if (strncmp((char *)de.filename, (char *)(*path)->dir_entry.filename, 8) == 0)
			{
				if( strncmp((char *)de.file_extension, (char *)(*path)->dir_entry.file_extension, 3) == 0)
				{
					/* 1. A file of this type already exists. */
					
					/* Error if we are not to create it */
					if( mode & FAM_NOCREATE )
					{
						fclose((*path)->fd);
						term_pd(*path);
						return EOS_FAE;
					}
					else
					{
						fclose((*path)->fd);
						term_pd(*path);
						_decb_kill(pathlist);
						return _decb_create( path, pathlist, mode, file_type, data_type );
					}
				}
			}
		}

	
		if (empty_entry == -1)
		{
			/* 1. There are no more directory entries left. */
			
			fclose((*path)->fd);
			
			term_pd(*path);
			
			return EOS_DF;
		}
	}

	
	/* 8. Allocate a granule for this file. */
	
	{
		error_code  ec;
		int			new_granule;
		
		
		ec = find_free_granule(*path, &new_granule, 34);
		
		if (ec != 0)
		{
			return ec;
		}
		
		(*path)->FAT[new_granule] = 0xC1;
		(*path)->dir_entry.first_granule = new_granule;
		
		_int2(0, (*path)->dir_entry.last_sector_size);
	}
	

	/* 9. Write the new directory entry. */	
	
	_decb_seekdir(*path, empty_entry, SEEK_SET);
	
	(*path)->this_directory_entry_index = empty_entry;
	
	_decb_writedir(*path, &(*path)->dir_entry);

	
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


	(*path)->disk_offset = 161280 * (*path)->drive;
	(*path)->disk_offset += (*path)->hdbdos_offset;
	
	
	/* 6. At this point, sector and granule function will work - Load FAT */

	_decb_gs_sector(*path, 17, 2, (char *)(*path)->FAT);


	/* 7. If path is raw, just return now. */
	
	if ((*path)->israw == 1)
	{
		return 0;
	}
	

	/* 8. Find directory entry matching filename. */

	{
		/* 1. Seek to the first directory entry. */

		_decb_seekdir(*path, 0, SEEK_SET);
		
		
		/* 2. Check each entry until we find a match. */
		
		while ((ec = _decb_readdir(*path, &(*path)->dir_entry)) == 0)
		{
			if (_decb_cmp(&(*path)->dir_entry, (*path)->filename) == 0)
			{
				/* 1. We have a match! */
				
				(*path)->directory_entry_index--;

				(*path)->this_directory_entry_index = (*path)->directory_entry_index;
								
				break;
			}
		}

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
	
	_decb_ss_sector(path, 17, 2, (char *)path->FAT);
	
	
	/* 2. Close path. */

	fclose(path->fd);


	/* 3. Terminate path descriptor */
	
	ec = term_pd(path);
	
	
	/* 4. Return status. */
	
	return(ec);
}



static int _decb_cmp(decb_dir_entry *entry, char *name)
{
	char modified_name[13];
	u_char *filename = entry->filename;
	u_char *ext = entry->file_extension;
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
 * foo,				 	Opens foo for raw access
 * foo,:2				Opens third disk in foo for raw access
 * foo,bar.bas			Opens file bar.bas in disk image foo
 * foo,bar.bin:1		Opens file bar.bin in second disk image in file foo
 * foo,bar.bin:0+12345  Opens file bar.bin in first disk at OS9 offset 12345
 */

static int validate_pathlist(decb_path_id *path, char *pathlist)
{
	error_code  ec = 0;
	char		*p;


	/* 1. Validate the pathlist. */

	if ((p = strchr(pathlist, ',')) == NULL)
	{
		/* 1. No native/RS-DOS delimiter in pathlist, return error. */

		ec = EOS_BPNAM;
	}
	else
	{
		char *q;
		

		/* 1. Extract information out of pathlist. */

		*p = '\0';
		strcpy((*path)->imgfile, pathlist);
		*p = ',';
		p++;
		if (*p == '/') p++;
		q = strchr(p, ':');
		if (q != NULL)
		{
			*q = '\0';
			strcpy((*path)->filename, p);
			(*path)->drive = atoi(q + 1);
			*q = ':';

			q = strchr(p, '+');
		    if (q != NULL)
		    {
		    	if(strncmp(q + 1, "0x", 2) == 0 || strncmp(q + 1, "0X", 2) == 0)
	 				(*path)->hdbdos_offset = strtol(q + 3, (char **) NULL, 16) * 256;
	 			else
	 				(*path)->hdbdos_offset = atoi(q + 1) * 256;
	 		}
		}
		else
		{
			strcpy((*path)->filename, p);			
		}
		
#if 0
		count = sscanf(pathlist, "%512[^,]%*c%64[^:]%*c%d", (*path)->imgfile, (*path)->filename, &((*path)->drive));

		if (count < 2)
		{
			count = sscanf(pathlist, "%512[^,]%*c%*c%d", (*path)->imgfile, &((*path)->drive));
		}
#endif
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

	memset(*path, 0, sizeof(struct _decb_path_id));
	
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
