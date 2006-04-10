/********************************************************************
 * write.c - Native Write routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>

#include "cocotypes.h"
#include "nativepath.h"



error_code _native_write(native_path_id path, void *buffer, u_int *size)
{
    error_code	ec = 0;
    size_t ret_size;


	/* 1. Check the mode. */
	
	if ((path->mode & FAM_DIR) != 0 || (path->mode & FAM_WRITE) == 0)
    {
        /* 1. Must be a directory. */

        return EOS_BMODE;
    }


    ret_size = fwrite(buffer, 1, *size, path->fd);

    *size = ret_size;


    return ec;
}
