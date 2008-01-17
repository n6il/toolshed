/********************************************************************
 * write.c - CoCo Write routines
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>

#include "cocotypes.h"
#include "cocopath.h"


error_code _coco_write(coco_path_id path, void *buffer, u_int *size)
{
	error_code		ec = 0;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_write(path->path.native, buffer, size);
			break;
			
		case OS9:
			ec = _os9_write(path->path.os9, buffer, size);
			break;
			
		case DECB:
			ec = _decb_write(path->path.decb, buffer, size);
			break;
		
		case CECB:
			fprintf( stderr, "_coco_write not implemented in libcecb yet.\n" );
			ec = -1;
			break;
	}
	
	
	return ec;
}
