/********************************************************************
 * seek.c - Disk BASIC Seek routine
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>

#include "cocotypes.h"
#include "decbpath.h"


error_code _decb_seek(decb_path_id path, int pos, int mode)
{
    error_code	ec = 0;


    if (path->israw == 1)
    {
        fseek(path->fd, pos, mode);
    }
    else
    {
        switch(mode)
        {
            case SEEK_SET:
                path->filepos = pos;
                break;

            case SEEK_CUR:
                path->filepos = path->filepos + pos;
                break;

            case SEEK_END:
                fprintf(stderr, "_decb_seek(): SEEK_END not implemented.\n");
                break;
        }
    }


    return ec;
}



error_code _decb_seekdir(decb_path_id path, int entry)
{
	error_code  ec = 0;
	
	
	if (entry > 72)
	{
		/* 1. Illegal entry number -- return error. */
		
		ec = EOS_EOF;
	}

	path->directory_entry_index = entry;
	

    return ec;
}




error_code _decb_seeksector(decb_path_id path, int track, int sector)
{
	long	offset;
	
//	assert( (track>= 0) && (track<35) );
//	assert( (sector>0) && (sector<19) );
	
	/* 1. Compute offset. */
	
	offset = (track * 18) + (sector - 1);
	offset *= 256;
	offset += path->disk_offset;
	

	/* 2. Seek to offset. */
	
	fseek(path->fd, offset, SEEK_SET);
	
	
	return 0;
}

error_code _decb_seekgranule(decb_path_id path, int granule)
{
	long	offset;
	
//	assert( (granule>= 0) && (granule<68) );
	
	/* 1. Compute offset. */
	
	offset = granule * 2304;
	

	/* 2. Account for directory (2 granules) */

	if (granule > 33)
	{
		offset += (2304 * 2);
	}
		

	/* 3. Add in disk offset. */
	
	offset += path->disk_offset;
	

	/* 4. Seek to granule. */
	
	fseek(path->fd, offset, SEEK_SET);
	
	
	return 0;
}
