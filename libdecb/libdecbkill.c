/********************************************************************
 * kill.c - Kill Disk BASIC file
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <cocotypes.h>
#include <decbpath.h>
#include <cococonv.h>


error_code _decb_kill(char *pathlist)
{
    error_code	ec = 0;
    decb_path_id path;
	int curr_granule;
	int sector_number, entry_offset;
	char sector[256];
	

    /* 1. Open a path. */
	
    ec = _decb_open(&path, pathlist, FAM_DIR | FAM_READ | FAM_WRITE);

    if (ec != 0)
    {
        return ec;
    }

	
    /* 3. Read directory sector that contains our directory entry. */

	sector_number = (path->directory_entry_index * sizeof(decb_dir_entry)) / 256;
	entry_offset = (path->directory_entry_index * sizeof(decb_dir_entry)) % 256;
	
	_decb_gs_sector(path, 17, 3 + sector_number, sector);
	
	
	/* 4. Erase the entry by writing a nul as the first byte and write out sector. */
	
	sector[entry_offset] = '\0';
	
	_decb_ss_sector(path, 17, 3 + sector_number, sector);
	
	
	/* 5. Clear the granules in the FAT used by this entry. */
	
	curr_granule = path->dir_entry.first_granule;
	
	while (path->FAT[curr_granule] < 0xC0)
	{
			int		next_granule;


			next_granule = path->FAT[curr_granule];

			path->FAT[curr_granule] = 0xFF;
			
			curr_granule = next_granule;
	}

	path->FAT[curr_granule] = 0xFF;
	
	
	/* 6. Close the path. */
	
    _decb_close(path);
    
	
    return(ec);
}
