/********************************************************************
 * open.c - CoCo open/create routines
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>

#include "cococonv.h"
#include "cocosys.h"
#include "cocotypes.h"
#include "cocopath.h"


/*
 * _coco_create()
 *
 * Create a file
 */
error_code _coco_create(coco_path_id *path, char *pathlist, int mode, int perms)
{
    error_code	ec = 0;
	_path_type type;


	/* 1. Allocate memory for the path id. */
	
	*path = malloc(sizeof(struct _coco_path_id));
	
	if (*path == NULL)
	{
		return EOS_BPNAM;
	}
	
	
    /* 2. Determine the pathlist type. */
	
    ec = _coco_gs_pathtype(pathlist, &type);
		
    if (ec != 0)
    {
        return ec;
    }


    /* 3. Call appropriate create function. */
	
	switch (type)
	{
		case NATIVE:
			ec = _native_create(&((*path)->path.native), pathlist, mode, perms);
			break;
			
		case OS9:
			ec = _os9_create(&((*path)->path.os9), pathlist, mode, perms);
			break;
			
		case DECB:
			ec = _decb_create(&((*path)->path.decb), pathlist, mode, perms, perms);
			break;
	}

	(*path)->type = type;

	
	return ec;
}



/*
 * _coco_open()
 *
 * Open a path to a file or directory.
 *
 * Legal pathnames are:
 *
 * 1. imagename,@     (considered to be a 'raw' open of the image)
 * 2. imagename,      (considered to be an open of the root directory)
 * 3. imagename,file  (considered to be a file or directory open within the image)
 * 4. imagename       (considered to be a native open) 
 *
 * The presence of a comma in the pathlist indicates that at the least, a non-native open will
 * be performed.
 */
error_code _coco_open(coco_path_id *path, char *pathlist, int mode)
{
    error_code	ec = 0;
	_path_type type;
	

	/* 1. Allocate memory for the path id. */
	
	*path = malloc(sizeof(struct _coco_path_id));
	
	if (*path == NULL)
	{
		return EOS_BPNAM;
	}
	
	
    /* 2. Determine the pathlist type. */
	
    ec = _coco_gs_pathtype(pathlist, &type);
		
    if (ec != 0)
    {
        return ec;
    }


    /* 3. Call appropriate create function. */
	
	switch (type)
	{
		case NATIVE:
			ec = _native_open(&((*path)->path.native), pathlist, mode);
			break;
			
		case OS9:
			ec = _os9_open(&((*path)->path.os9), pathlist, mode);
			break;
			
		case DECB:
			ec = _decb_open(&((*path)->path.decb), pathlist, mode);
			break;
			
	}

	(*path)->type = type;
	
	
	return ec;
}



/*
 * _coco_close()
 *
 * Close a file
 */
error_code _coco_close(coco_path_id path)
{
    error_code	ec = 0;


    /* 1. Call appropriate close function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_close(path->path.native);
			break;

		case OS9:
			ec = _os9_close(path->path.os9);
			break;
			
		case DECB:
			ec = _decb_close(path->path.decb);
			break;
			
	}
	
	
	free(path);
	
	
	return ec;
}

