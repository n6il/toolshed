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


	/* 1. Allocate memory for the path id. */
	
	*path = malloc(sizeof(struct _coco_path_id));
	
	if (*path == NULL)
	{
		return EOS_BPNAM;
	}
	
	
    /* 2. Determine the pathlist type. */
	
    ec = _coco_identify_image(pathlist, &(*path)->type);
		
    if (ec != 0)
    {
        return ec;
    }


    /* 3. Call appropriate create function. */
	
	switch ((*path)->type)
	{
		case NATIVE:
			ec = _native_create(&((*path)->path.native), pathlist, mode, perms);
			break;
			
		case OS9:
			ec = _os9_create(&((*path)->path.os9), pathlist, mode, perms);
			break;
			
		case DECB:
			{
				char	file_type = 0, data_type = 0;
				
				ec = _decb_create(&((*path)->path.decb), pathlist, mode, file_type, data_type);
			}
			break;
	}

	
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
	

	/* 1. Allocate memory for the path id. */
	
	*path = malloc(sizeof(struct _coco_path_id));
	
	if (*path == NULL)
	{
		return EOS_BPNAM;
	}
	
	
    /* 2. Determine the pathlist type. */
	
    ec = _coco_identify_image(pathlist, &(*path)->type);
		
    if (ec != 0)
    {
        return ec;
    }


    /* 3. Call appropriate create function. */
	
	switch ((*path)->type)
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


/*
 * _get_pathtype()
 *
 * Determines if the passed <image,path> pathlist is native, OS-9 or Disk BASIC.
 */
error_code _coco_identify_image(char *pathlist, _path_type *type)
{
	error_code		ec = 0;
    char *p;
    char *tmppathlist;
	FILE *fp;
	
	
    if (strchr(pathlist, ',') == NULL)
    {
        /* 1. No native/coco delimiter in pathlist, it's native. */
		
		*type = NATIVE;
		
        return 0;
    }
	
	
    /* 2. Check validity of pathlist. */
	
    tmppathlist = strdup(pathlist);
	
    p = strtok(tmppathlist, ",");
	
    if (p == NULL)
    {
        free(tmppathlist);
		
        return EOS_BPNAM;
    }
	
	
    /* 3. Determine if this is an OS-9 or DECB image. */
	
	fp = fopen(tmppathlist, "r");
	
	if (fp != NULL)
	{
		u_char sector_buffer[256];
		
		
		/* 1. Read sector 0. */
		
		if (fread(sector_buffer, 1, 256, fp) < 256)
		{
			ec = EOS_BPNAM;
		}
		else
		{
			Lsn0_sect   os9_sector = (Lsn0_sect)sector_buffer;
			int dir_sector_offset;
			int bps = 256;
			
			
			/* 1. Look for markers that this is an OS-9 disk image. */
			
			/* First, assume bps value is valid and get it */
			
			if (int1(os9_sector->dd_lsnsize) > 0)
			{
				bps = int1(os9_sector->dd_lsnsize) * 256;
			}
			
			/* Then, check out the dir sector for .. and . entries. */
			
			dir_sector_offset = (int3(os9_sector->dd_dir) + 1) * bps;
			
			fseek(fp, dir_sector_offset, SEEK_SET);
			
			if (fread(sector_buffer, 1, 256, fp) < 256)
			{
				*type = DECB;
			}
			else
			{
				if (sector_buffer[0] == 0x2E && sector_buffer[1] == 0xAE &&
					sector_buffer[32] == 0xAE)
				{
					/* 1. This is likely an OS-9 disk image. */
					
					*type = OS9;
				}
				else
				{
					/* 1. This is probably a DECB disk image. */
					
					*type = DECB;
				}
			}
		}
		
		fclose(fp);
	}
	else
	{
		ec = EOS_BPNAM;
	}
	
	
    free(tmppathlist);
	
	
    return 0;
}

