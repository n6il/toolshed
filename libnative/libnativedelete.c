/********************************************************************
 * delete.c - Delete file entry for Native filesystems
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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
