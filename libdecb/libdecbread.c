/********************************************************************
 * read.c - Disk BASIC Read routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>

#include "cocotypes.h"
#include "decbpath.h"


error_code _decb_read(decb_path_id path, void *buffer, int *size)
{
	error_code		ec = 0;
    int				curr_granule, last_granule;
    int				accum_size = 0;
    int				bytes_left;
    unsigned int	filesize;


	/* 1. Check the mode. */
	
    if (path->mode & FAM_DIR || path->mode & FAM_READ == 0)
    {
        /* 1. Must use _decb_readdir if FAM_DIR. */
		
        return EOS_FNA;
    }


     /* 2. Treat raw path differently. */
	
    if (path->israw == 1)
    {
        unsigned int  disksize = 35 * (18 * 256);


        if (path->filepos >= disksize)
        {
			*size = 0;
			
            ec = EOS_EOF;
        }
		else
		{
			fseek(path->fd, path->filepos, SEEK_SET);
			fread(buffer, 1, *size, path->fd);
			path->filepos += *size;
		}

		return ec;
    }
    
	
	/* 3. Get file's size. */

	ec = _decb_gs_size(path, &filesize);

	if (ec != 0)
	{
		return ec;
	}
	

    /* 4. If our file position is greater than the file size, return error. */

    if (path->filepos >= filesize)
    {
        /* 1. End of file */
		
		*size = 0;

        return EOS_EOF;
    }


    /* 5. If the passed size is greater than the length of the file minus
     *    the file position, then reset the size
     */
	 
    if (*size > filesize - path->filepos)
    {
        *size = filesize - path->filepos;
    }


    /* 6. Determine which granule the offset starts by looping
     *    through each entry until we reach the end.
     */
	 
    accum_size = 0;

	curr_granule = path->first_granule;
	last_granule = curr_granule;
	
	while (path->FAT[curr_granule] < 0xC0)
	{
		last_granule = curr_granule;
		curr_granule = path->FAT[curr_granule];
		
		accum_size += 4608 / 2;

        if (accum_size > path->filepos)
        {
            /* this is the granule! */
            accum_size -= 4608 / 2;
			curr_granule = last_granule;
			
            break;
        }
	}


    /* 7. Start copying data into the user supplied buffer for 'bytes_left' bytes. */

    bytes_left = *size;

    while (bytes_left > 0)
    {
		char granule_buffer[2304];
		int read_size, offset_in_granule;
		int bytes_in_last_granule = 2304;
			
		
		_decb_gs_granule(path, curr_granule, granule_buffer);
		
		if (path->FAT[curr_granule] > 0xC0)
		{
			bytes_in_last_granule = ((path->FAT[curr_granule] & 0x3F) * 256) - 256 + path->bytes_in_last_sector;
		}
		else
		{
			curr_granule = path->FAT[curr_granule];
		}
		

//		offset_in_granule = path->filepos % bytes_in_last_granule;
		offset_in_granule = path->filepos % 2304;

		read_size = bytes_in_last_granule - offset_in_granule;

		if (read_size > bytes_left)
		{
			read_size = bytes_left;
		}


		memcpy(buffer, granule_buffer + offset_in_granule, read_size);

		bytes_left -= read_size;
		path->filepos += read_size;
		buffer += read_size;
	}

	
    return ec;
}



error_code _decb_readln(decb_path_id path, void *buffer, int *size)
{
	error_code		ec = 0;
	

	/* 1. Check the mode. */
	
    if (path->mode & FAM_DIR || path->mode & FAM_READ == 0)
    {
        /* 1. Must use _decb_readdir if FAM_DIR. */
		
        return EOS_FNA;
    }

	
	return ec;
}



error_code _decb_readdir(decb_path_id path, decb_dir_entry *dirent)
{
    error_code	ec = 0;
	char buffer[256];
	int sector;
	int entry_in_sector;
	

#if 0
	/* 1. Check the mode. */
	
	if (path->mode & FAM_DIR == 0 || path->mode & FAM_READ == 0)
    {
        /* 1. Must be a directory. */

        return EOS_BMODE;
    }
#endif
	

retry:
	sector = (path->directory_entry_index * sizeof(decb_dir_entry)) / 256;
	entry_in_sector = (path->directory_entry_index++ * sizeof(decb_dir_entry)) % 256;


	/* 2. Check if we have passed last entry. */
	
	if (path->directory_entry_index == 73)
	{
		/* 1. Yes.  Return error. */
		
		path->directory_entry_index = 72;
		
		return(EOS_PNNF);
	}
	
	
	ec = _decb_gs_sector(path, 17, 3 + sector, buffer);

	if (ec == 0)
	{
		memcpy(dirent, buffer + entry_in_sector, sizeof(decb_dir_entry));
	}


    return(ec);
}


