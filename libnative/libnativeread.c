/********************************************************************
 * read.c - Native Read routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>

#include "cocotypes.h"
#include "nativepath.h"


error_code _native_read(native_path_id path, void *buffer, u_int *size)
{
    error_code	ec = 0;
    size_t ret_size;


	/* 1. Check the mode. */
	
	if ((path->mode & FAM_DIR) != 0 || (path->mode & FAM_READ) == 0)
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


error_code _native_readdir(native_path_id path, native_dir_entry *dirent)
{

/*
	On MINGW dirent is:   struct _finddata_t
	On VS dirent is:      WIN32_FIND_DATA
	On *nix dirent is:    struct dirent
*/	

    error_code	ec = 0;

#if defined(__MINGW32__)
	struct _finddata_t dp;
#endif

	/* 1. Check the mode. */
	
	if (path->mode & FAM_DIR == 0 || path->mode & FAM_READ == 0)
    {
        /* 1. Must be a directory. */

        return EOS_BMODE;
    }


#if defined(__MINGW32__)
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

error_code _native_ncpy_name( native_dir_entry e, u_char *name, size_t len )
{
	error_code ec = 0;

#if defined(__MINGW32__)
	/* typedef struct _finddata_t  native_dir_entry; */
	strncpy( (char *)name, e.name, len );
#else
	/* Copy name from dir entry to suppilied buffer */
	strncpy( (char *)name, e.d_name, len );
#endif

	return ec;

}
