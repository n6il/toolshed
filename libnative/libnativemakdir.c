/********************************************************************
 * makdir.c - Create a native directory
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "cocotypes.h"
#include "cococonv.h"
#include "nativepath.h"


error_code _native_makdir(char *pathlist)
{
    error_code	ec = 0;


#ifdef WIN32
    ec = _mkdir(pathlist);
#else
    mode_t myMode =	S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

    ec = mkdir(pathlist, myMode);
#endif

    if (ec != 0)
	{
		ec = errno;
	}
	
	
    return ec;
}
