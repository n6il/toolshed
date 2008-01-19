/********************************************************************
 * delete.c - Delete file entry for Native filesystems
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#ifndef BDS
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <errno.h>

#include "cocotypes.h"
#include "cococonv.h"
#include "nativepath.h"


error_code _native_delete(char *pathlist)
{
    error_code	ec = 0;
	
	
    ec = unlink(pathlist);
	
    if (ec != 0)
	{
		ec = errno;
	}
	
	
	return ec;
}



error_code _native_delete_directory(char *pathlist)
{
    error_code	ec = 0;
	
	
    ec = rmdir(pathlist);
	
    if (ec != 0)
	{
		ec = errno;
	}
	
	
	return ec;
}



error_code _native_truncate(char *pathlist, off_t length)
{
    error_code	ec = 0;
	
	
    ec = truncate(pathlist, length);
	
    if (ec != 0)
	{
		ec = errno;
	}
	
	
	return ec;
}
