/********************************************************************
 * write.c - Disk BASIC Write routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <decbpath.h>


static error_code _raw_write(decb_path_id path, void *buffer, int *size);
static error_code extend_fat_chain(decb_path_id path, int current_size, int new_size);
error_code find_free_granule(decb_path_id path, int *granule, int next_to);


error_code _decb_write(decb_path_id path, void *buffer, int *size)
{
    error_code	ec = EOS_WRITE;
	int current_size = 0, accum_size = 0, curr_granule, bytes_left;
		

	/* 1. Check the mode. */
	
	if (path->mode & FAM_WRITE == 0)
    {
        /* 1. Must be writable. */

        return EOS_BMODE;
    }


    /* 2. Treat raw path differently. */
	
    if (path->israw == 1)
    {
        ec = _raw_write(path, buffer, size);
    }


	/* 3. Get the file's current size. */
	
	_decb_gs_size(path, &current_size);
	
	
	/* 4. If our file position is greater than the file size, return error. */
	
	if (path->filepos > current_size)
	{
		/* 1. End of file. */
		
		return(EOS_EOF);
	}
	
	
	/* 5. If there is not enough room, we need to extend the FAT chain. */

	if (current_size < path->filepos + *size)
	{
		/* 1. We need to enlarge the file. */		

		ec = extend_fat_chain(path, current_size, path->filepos + *size);
		
		
		/* 2. If there is an error, time to abort */

		if (ec != 0)
		{
			return ec;
		}
	}
	

	/* 6. Determine which granule the offset is in. */

	accum_size = 0;
	
	curr_granule = path->dir_entry.first_granule;
		
	while (path->FAT[curr_granule] < 0xC0)
	{
		accum_size += 2304;
		
		if (accum_size > path->filepos)
		{
			/* 1. This is the granule we begin at. */

			accum_size -= 2304;
			
			break;
		}

		curr_granule = path->FAT[curr_granule];
	}
	
	
	/* 7. Copy user supplied data into the file for 'bytes_left' bytes. */

	bytes_left = (path->filepos + *size) - current_size;


    while (bytes_left > 0)
    {
		char granule_buffer[2304];
		int write_size, offset_in_granule;
		int bytes_in_last_granule = 2304;
		
		
		_decb_gs_granule(path, curr_granule, granule_buffer);
		
		if (path->FAT[curr_granule] >= 0xC0)
		{
			bytes_in_last_granule = (((path->FAT[curr_granule] & 0x3F) - 1) * 256) + int2(path->dir_entry.last_sector_size);
		}
		
		
		offset_in_granule = path->filepos % 2304;
		
		write_size = bytes_in_last_granule - offset_in_granule;
		
		if (write_size > bytes_left)
		{
			write_size = bytes_left;
		}
		
		
		memcpy(granule_buffer + offset_in_granule, buffer, write_size);
		_decb_ss_granule(path, curr_granule, granule_buffer);

		
		bytes_left -= write_size;
		path->filepos += write_size;
		buffer += write_size;

		
		/* Point to next granule for next pass. */
		
		curr_granule = path->FAT[curr_granule];
	}
	
	
	/* 9. Write updated file descriptor back to image file. */

	_decb_seekdir(path, path->this_directory_entry_index);

	_decb_writedir(path, &path->dir_entry);
	
	
    return ec;
}



static error_code _raw_write(decb_path_id path, void *buffer, int *size)
{
    error_code	ec = 0;
    size_t ret_size;


    ret_size = fwrite(buffer, 1, *size, path->fd);
    *size = ret_size;


    return ec;
}



error_code _decb_writedir(decb_path_id path, decb_dir_entry *dirent)
{
    error_code	ec = 0;
	char buffer[256];
	int sector = (path->directory_entry_index * sizeof(decb_dir_entry)) / 256;
	int entry_in_sector;
	

	/* 1. Check the mode. */
	
	if (path->mode & FAM_WRITE == 0)
    {
        return EOS_BMODE;
    }

	
	entry_in_sector = (path->directory_entry_index * sizeof(decb_dir_entry)) % 256;


	/* 2. Check if we have passed last entry. */
	
	if (path->directory_entry_index == 73)
	{
		/* 1. Yes.  Return error. */
		
		return EOS_DF;
	}
	
	
	ec = _decb_gs_sector(path, 17, 3 + sector, buffer);

	if (ec == 0)
	{
		/* 1. Check if this is an empty entry. */
		
		memcpy(buffer + entry_in_sector, dirent, sizeof(decb_dir_entry));

		ec = _decb_ss_sector(path, 17, 3 + sector, buffer);
	}


    return ec;
}



static error_code extend_fat_chain(decb_path_id path, int current_size, int new_size)
{
	int curr_granule = path->dir_entry.first_granule;
	int max_size_with_curr_granules_allocated = 0;
	unsigned char tmp_FAT[256];
	
	
	/* 1. Save a copy in case we run out of disk space. */
	
	memcpy(tmp_FAT, path->FAT, 256);
	
	
	/* 1. Compute maximum size of file with current granules allocated. */
	
	while (path->FAT[curr_granule] < 0xC0)
	{
		curr_granule = path->FAT[curr_granule];

		max_size_with_curr_granules_allocated += 2304;
	}
	
	max_size_with_curr_granules_allocated += 2304;

	
	if (new_size > max_size_with_curr_granules_allocated)
	{
		int new_granule;

		
		do
		{
			/* 1. We're gonna have to find a free granule. */
	
			if (find_free_granule(path, &new_granule, curr_granule) != 0)
			{
				/* 1. Could not find any free granules. */
		
				memcpy(path->FAT, tmp_FAT, 256);

				return EOS_DF;
			}
		
			path->FAT[curr_granule] = new_granule;
			curr_granule = new_granule;
			path->FAT[curr_granule] = 0xC0;
			max_size_with_curr_granules_allocated += 2304;
		}
		while (new_size > max_size_with_curr_granules_allocated);
		
	}

	{
		/* 1. The new size will fit in the currently allocated granules,
		 *    so we'll just extend the last granule entry.
		 */

		int expand_size = new_size - (max_size_with_curr_granules_allocated - 2304);
		
		
		/* 2. Reset the last granule's sector size. */
	
		if (expand_size % 256 == 0)
		{
			path->FAT[curr_granule] = 0xC0 + (expand_size / 256);
			_int2(256, path->dir_entry.last_sector_size);
		}
		else
		{
			path->FAT[curr_granule] = 0xC1 + (expand_size / 256);
			_int2(expand_size % 256, path->dir_entry.last_sector_size);
		}
	}
	
	
	return 0;
}



/*
 * find_free_granule: find a free granule.
 *
 * The 'next_to' parameter lets this function attempt to find a granule
 * either ahead of or behind the next_to.
 */

error_code find_free_granule(decb_path_id path, int *granule, int next_to)
{
	int		t_next_to = next_to + 1;
	
	
	*granule = 0;
	
	/* 1. Validate next_to. */
	
	if (t_next_to > 255)
	{
		return EOS_DF;
	}


	/* 2. Start search from next_to to last_granule. */
	
	while (path->FAT[t_next_to] != 0x00)
	{
		if (path->FAT[t_next_to] == 0xFF)
		{
			/* 1. Found one!  Return it. */
			
			*granule = t_next_to;
			
			return 0;
		}
		
		t_next_to++;
	}


	/* 3. Now try searching from t_nexto - 1 to 0. */

	if (next_to > 0)
	{
		t_next_to = next_to - 1;
		

		while (t_next_to >= 0)
		{
			if (path->FAT[t_next_to] == 0xFF)
			{
				/* 1. Found one!  Return it. */
			
				*granule = t_next_to;
			
				return 0;
			}
		
			t_next_to--;
		}
	}

	
	return EOS_DF;
}



