/********************************************************************
 * rename.c - Rename file entry for OS-9 filesystems
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "cocotypes.h"
#include "cococonv.h"
#include "cocosys.h"
#include "nativepath.h"



error_code _native_rename(char *pathlist, char *new_name)
{
    error_code	ec = 0;
    char		*a;
    char		*dest = malloc(strlen(pathlist) + strlen(new_name));


    if (dest == NULL)
    {
        return 1;
    }
	

    /* Construct destination path name */
	
    a = strrchr(pathlist, '/');
	
    if (a == NULL)
    {
        strcpy(dest, new_name);
    }
    else
    {
        strncpy(dest, pathlist, a - pathlist + 1);
        strcat(dest, new_name);
    }

    rename(pathlist, dest);
	
    free(dest);
	
    if (ec != 0)
	{
        ec = errno;
	}
	
	
    return ec;
}
