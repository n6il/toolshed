/********************************************************************
 * gs.c - Disk BASIC GetStatus routines
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "decbpath.h"
#include "cococonv.h"



error_code _decb_gs_fd(decb_path_id path, decb_file_stat *stat)
{
    error_code		ec = 0;
	int				size;
	decb_dir_entry  de;


	{
		int mode = path->mode;
		
		
		path->mode |= FAM_DIR;
		_decb_seekdir(path, path->this_directory_entry_index);
		_decb_readdir(path, &de);
		path->mode = mode;
	}
	
	stat->file_type = de.file_type;
	stat->data_type = de.ascii_flag;
	
	ec = _decb_gs_size(path, &size);
	
	if (ec == 0)
	{
		stat->file_size = size;
	}


	return ec;
}



error_code _decb_gs_eof(decb_path_id path)
{
    error_code	ec = 0;
	int filesize;


	ec = _decb_gs_size(path, &filesize);

	if (ec != 0 || path->filepos >= filesize)
	{
		ec = EOS_EOF;
    }

    
    return(ec);
}



error_code _decb_gs_size(decb_path_id path, int *size)
{
    error_code	ec = 0;
	int curr_granule;
	

	*size = 0;
	

	/* 1. The following code is for DECB paths. */
	
	curr_granule = path->dir_entry.first_granule;

	while (path->FAT[curr_granule] < 0xC0)
	{
		curr_granule = path->FAT[curr_granule];
		
		*size += 2304;
	}

	*size += 256 * ((path->FAT[curr_granule] & 0x3f) - 1)+ int2(path->dir_entry.last_sector_size);
	
#if 0
	if (path->FAT[curr_granule] > 0xC0)
	{
		*size += 256 * ((path->FAT[curr_granule]) & 0x3F);

		if (path->dir_entry.last_sector_size[1] != 0)
		{
			*size += path->dir_entry.last_sector_size[1];
		
			*size -= 256;
		}
	}
#endif

    return(ec);
}


error_code _decb_gs_sector(decb_path_id path, int track, int sector, char *buffer)
{
	error_code	ec = 0;
	size_t		size;


	/* 1. Seek to the track and sector. */
	
	_decb_seeksector(path, track, sector);


	/* 2. Get the sector into the buffer. */
	
	size = fread(buffer, 1, 256, path->fd);
	
//	assert( size == 256 );

	/* 3. Return status. */
	
	return ec;
}


error_code _decb_gs_granule(decb_path_id path, int granule, char *buffer)
{
	error_code	ec = 0;


	/* 1. Seek to granule. */
	
	_decb_seekgranule(path, granule);
	

	/* 2. Read granule into buffer. */
	
	fread(buffer, 1, 2304, path->fd);
	

	/* 3. Return status. */
	
	return ec;
}




