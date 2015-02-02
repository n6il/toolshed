/********************************************************************
 * gs.c - Disk BASIC GetStat routines
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
	u_int			size;

	
	stat->file_type = path->dir_entry.file_type;
	stat->data_type = path->dir_entry.ascii_flag;
	
	ec = _decb_gs_size(path, &size);
	
	if (ec == 0)
	{
		stat->file_size = size;
	}


	return ec;
}



error_code _decb_gs_fd_pathlist(char *pathlist, decb_file_stat *statbuf)
{
    error_code	ec = 0;
	decb_path_id path;
	
	/* Open a path to the pathlist */
	
	ec = _decb_open(&path, pathlist, FAM_READ);
	
	if (ec != 0)
	{
		return ec;
	}
	
	
	ec = _decb_gs_fd(path, statbuf);
	
	_decb_close(path);
	

    return ec;
}



error_code _decb_gs_eof(decb_path_id path)
{
    error_code		ec = 0;
	u_int			filesize;


	ec = _decb_gs_size(path, &filesize);

	if (ec != 0 || path->filepos >= filesize)
	{
		ec = EOS_EOF;
    }

    
    return ec;
}



error_code _decb_gs_size(decb_path_id path, u_int *size)
{
    error_code	ec = 0;
	int curr_granule;
	int sectors_in_last_granule;

	*size = 0;
	

	/* 1. The following code is for DECB paths. */
	
	curr_granule = path->dir_entry.first_granule;

	while (path->FAT[curr_granule] < 0xC0)
	{
		curr_granule = path->FAT[curr_granule];
		
		*size += 2304;
	}

	sectors_in_last_granule = (path->FAT[curr_granule] & 0x3f) - 1;
	sectors_in_last_granule = sectors_in_last_granule < 0 ? 0 : sectors_in_last_granule;
	
	*size += (256 * sectors_in_last_granule) + int2(path->dir_entry.last_sector_size);
	

    return ec;
}



error_code _decb_gs_size_pathlist(char *pathlist, u_int *size)
{
    error_code	ec = 0;
	decb_path_id path;
	
	/* Open a path to the pathlist */
	
	ec = _decb_open(&path, pathlist, FAM_READ);
	
	if (ec != 0)
	{
		return ec;
	}
	
	
	ec = _decb_gs_size(path, size);
	
	_decb_close(path);
	

    return ec;
}



error_code _decb_gs_pos(decb_path_id path, u_int *pos)
{
    error_code	ec = 0;


	*pos = path->filepos;	

    return ec;
}



error_code _decb_gs_sector(decb_path_id path, int track, int sector, char *buffer)
{
	error_code	ec = 0;
//	size_t		size;


	/* 1. Seek to the track and sector. */

	_decb_seeksector(path, track, sector);


	/* 2. Get the sector into the buffer. */

//	size = fread(buffer, 1, 256, path->fd);
	fread(buffer, 1, 256, path->fd);

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

	if(path->hdbdos_offset)
	{
		int count;

		for(count = 0; count < 2304; count += 256)
		{
			fread(&buffer[count], 1, 256, path->fd);
			/* skip unused 1/2 of sector */
			fseek(path->fd, 256, SEEK_CUR);
		}
	}
	else
	{
		fread(buffer, 1, 2304, path->fd);
	}
	

	/* 3. Return status. */
	
	return ec;
}
