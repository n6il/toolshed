/********************************************************************
 * read.c - CoCo Read routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>

#include "cocotypes.h"
#include "cocopath.h"



error_code _coco_read(coco_path_id path, void *buffer, int *size)
{
	error_code		ec = 0;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_read(path->path.native, buffer, size);
			break;
			
		case OS9:
			ec = _os9_read(path->path.os9, buffer, size);
			break;
			
		case DECB:
			ec = _decb_read(path->path.decb, buffer, size);
			break;
	}
	
	
	return ec;
}



error_code _coco_readdir(coco_path_id path, coco_dir_entry *e)
{
	error_code		ec = 0;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_readdir(path->path.native, e->dentry.native);
			break;
			
		case OS9:
			ec = _os9_readdir(path->path.os9, e->dentry.os9);
			break;
			
		case DECB:
			ec = _decb_readdir(path->path.decb, e->dentry.decb);
			break;
	}
	
	
	return ec;
}


