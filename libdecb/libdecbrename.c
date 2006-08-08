/********************************************************************
 * rename.c - Rename file entry for Disk BASIC
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "cocotypes.h"
#include "cocosys.h"
#include "decbpath.h"
#include "cococonv.h"
#include "util.h"


error_code _decb_rename(char *pathlist, char *new_name)
{
	decb_dir_entry dirent;

	return _decb_rename_ex(pathlist, new_name, &dirent);
}



error_code _decb_rename_ex(char *pathlist, char *new_name, decb_dir_entry *dirent)
{
	error_code	ec = 0;
    char filename[33];
	decb_path_id path;


	/* 1. Test if path is native. */
	
	if (strchr(pathlist, ',') == NULL)
    {
        return EOS_BPNAM;
	}
	

    ec = _decb_open(&path, pathlist, FAM_READ| FAM_WRITE);

    if (ec != 0)
	{
		return ec;
	}


    /* 2. Start reading directory file and search for match */

	_decb_seekdir(path, 0);

	while (_decb_readdir(path, dirent) == 0)
	{
        char fname[32];


        if (!strcasecmp(fname, filename))
        {
			/* 1. Found the source, rename it. */
			
            /* 2. Write the directory entry back to the image. */

			_decb_writedir(path, dirent);
			
			break;
        }
    }

	_decb_close(path);


    return 0;
}
