/********************************************************************
 * gs.c - native GetStatus routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cocotypes.h"
#include "nativepath.h"
#include "cococonv.h"



error_code _native_gs_attr(native_path_id path, int *perms)
{
    error_code	ec = 0;
    struct stat statbuf;


    ec = _native_gs_fd(path, &statbuf);

    if (ec == 0)
    {
        *perms = statbuf.st_mode;
    }


    return ec;
}



error_code _native_gs_eof(native_path_id path)
{
    error_code	ec = 0;
	unsigned char c;


	if (path->mode & FAM_DIR)
	{
#ifdef __MINGW32__
#else
		off_t offset;

		/* get offset */
		offset = telldir(path->dirhandle);

		/* read the next entry, if any... */
		if (readdir(path->dirhandle) == NULL)
		{
			ec = EOS_EOF;
		}
		else
		{
			/* restore the offset we had before we read the test entry */
			seekdir(path->dirhandle, offset);
		}
#endif

		return ec;
	}
        
	fread(&c, 1, 1, path->fd);

	if (feof(path->fd) != 0)
	{
		ec = EOS_EOF;
	}

	ungetc(c, path->fd);


	return ec;
}



error_code _native_gs_fd(native_path_id path, struct stat *statbuf)
{
    error_code	ec = 0;
		

#if defined(__APPLE__) || defined(__MINGW32__)
	if (fstat(path->fd->_file, statbuf) < 0)
#else
	if (fstat(path->fd->_fileno, statbuf) < 0)
#endif
	{
		ec = EOS_FNA;
	}


    return ec;
}



error_code _native_gs_size(native_path_id path, int *size)
{
    error_code	ec = 0;
	struct stat statbuf;

        
#if defined(__APPLE__) || defined(__MINGW32__)
	if (fstat(path->fd->_file, &statbuf) < 0)
#else
	if (fstat(path->fd->_fileno, &statbuf) < 0)
#endif
	{
		ec = EOS_FNA;
	}
	else
	{
        *size = statbuf.st_size;
	}


	return ec;
}
