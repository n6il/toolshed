/********************************************************************
 * delete.c - Delete file entry for OS-9 filesystems
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "cocotypes.h"
#include "cocopath.h"


error_code _coco_delete(char *pathlist)
{
	error_code		ec = 0;
	_path_type		disk_type;
	
	
	/* 1. Determine the path type. */
	
	_coco_gs_pathtype(pathlist, &disk_type);
	
	
    /* 2. Call appropriate function. */
	
	switch (disk_type)
	{
		case NATIVE:
			ec = _native_delete(pathlist);
			break;
			
		case OS9:
			ec = _os9_delete(pathlist);
			break;
			
		case DECB:
			ec = _decb_kill(pathlist);
			break;
	}
	
	
	return ec;
}
