/********************************************************************
 * read.c - Native Read routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>

#include "cocotypes.h"
#include "nativepath.h"


error_code _native_read(native_path_id path, void *buffer, int *size)
{
    error_code	ec = 0;
    size_t ret_size;


	/* 1. Check the mode. */
	
	if (path->mode & FAM_DIR || path->mode & FAM_READ == 0)
	{
		/* 1. Caller must use '_native_readdir' to read directories. */
		
		ec = EOS_BMODE;
	}
	else
	{
		/* 1. Perform the read. */
	
		ret_size = fread(buffer, 1, *size, path->fd);
	
		if (ret_size <= 0 && *size > 0)
		{
			/* 1. We have reached end of file. */

			ec = EOS_EOF;
		}

		*size = ret_size;
	}


    return ec;
}



#ifdef __MINGW32__
error_code _native_readdir(native_path_id path, struct _finddata_t *dirent)
#else
error_code _native_readdir(native_path_id path, struct dirent *dirent)
#endif
{
    error_code	ec = 0;
#ifdef __MINGW32__
	struct _finddata_t dp;
#endif

	/* 1. Check the mode. */
	
	if (path->mode & FAM_DIR == 0 || path->mode & FAM_READ == 0)
    {
        /* 1. Must be a directory. */

        return EOS_BMODE;
    }


#ifdef __MINGW32__
	if (path->dirhandle == 0)
	{
		path->dirhandle = (DIR *)_findfirst("*", dirent);
	}
	else
	{
		ec = _findnext((int)path->dirhandle, &dp);
	}
	if (ec == -1)
#else
	dirent = readdir(path->dirhandle);

	if (dirent == NULL)
#endif
	{
		return EOS_EOF;
	}


    return ec;
}
