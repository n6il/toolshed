/********************************************************************
 * gs.c - CoCo GetStatus routines
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cocotypes.h"
#include "cococonv.h"
#include "cocopath.h"



/*
 * _coco_gs_pathtype()
 *
 * Determines if the passed <image,path> pathlist is native, OS-9 or Disk BASIC.
*/
error_code _coco_gs_pathtype(char *pathlist, _path_type *disk_type)
{
	error_code		ec = 0;
    char *p;
    char *tmppathlist;
	FILE *fp;


    if (strchr(pathlist, ',') == NULL)
    {
        /* 1. No native/coco delimiter in pathlist, it's native. */

		*disk_type = NATIVE;

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
			
			/* First, check out the dir sector for .. and . entries. */
			
			dir_sector_offset = (int3(os9_sector->dd_dir) + 1) * bps;
			
			fseek(fp, dir_sector_offset, SEEK_SET);
			
			if (fread(sector_buffer, 1, 256, fp) < 256)
			{
				*disk_type = DECB;
			}
			else
			{
				if (sector_buffer[0] == 0x2E && sector_buffer[1] == 0xAE &&
					sector_buffer[32] == 0xAE)
				{
					/* 1. This is likely an OS-9 disk image. */
					
					*disk_type = OS9;
				}
				else
				{
					/* 1. This is probably a DECB disk image. */
					
					*disk_type = DECB;
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



error_code _coco_gs_attr(coco_path_id path, int *perms)
{
	error_code		ec = 0;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_gs_attr(path->path.native, perms);
			break;
			
		case OS9:
			ec = _os9_gs_attr(path->path.os9, perms);
			break;
			
		case DECB:
			ec = EOS_BPNAM;
			break;
	}
	
	
	return ec;
}



error_code _coco_gs_eof(coco_path_id path)
{
	error_code		ec = 0;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_gs_eof(path->path.native);
			break;
			
		case OS9:
			ec = _os9_gs_eof(path->path.os9);
			break;
			
		case DECB:
			ec = _decb_gs_eof(path->path.decb);
			break;
	}
	
	
	return ec;
}



error_code _coco_gs_fd(coco_path_id path, coco_file_stat *statbuf)
{
	error_code		ec = 0;
	struct stat		native_stat;
	fd_stats		os9_stat;
	decb_file_stat  decb_stat;
	struct tm		timepak;
	time_t			tp;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_gs_fd(path->path.native, &native_stat);
			statbuf->attributes = 0;
			if (native_stat.st_mode & S_IRUSR) { statbuf->attributes |= FAP_READ; }
			if (native_stat.st_mode & S_IWUSR) { statbuf->attributes |= FAP_WRITE; }
			if (native_stat.st_mode & S_IXUSR) { statbuf->attributes |= FAP_EXEC; }
#ifndef __MINGW32__
			if (native_stat.st_mode & S_IROTH) { statbuf->attributes |= FAP_PREAD; }
			if (native_stat.st_mode & S_IWOTH) { statbuf->attributes |= FAP_PWRITE; }
			if (native_stat.st_mode & S_IXOTH) { statbuf->attributes |= FAP_PEXEC; }
#endif
			if (native_stat.st_mode & S_IFDIR) { statbuf->attributes |= FAP_DIR; }
			statbuf->user_id = native_stat.st_uid;
			statbuf->group_id = native_stat.st_gid;
#ifndef __MINGW32__
#if defined __APPLE__
			statbuf->create_time = native_stat.st_ctimespec.tv_sec;
			statbuf->last_modified_time = native_stat.st_mtimespec.tv_sec;
#else
			statbuf->create_time = native_stat.st_ctim.tv_sec;
			statbuf->last_modified_time = native_stat.st_mtim.tv_sec;
#endif
#else
			statbuf->create_time = native_stat.st_ctime;
			statbuf->last_modified_time = native_stat.st_mtime;
#endif
			break;
			
		case OS9:
			ec = _os9_gs_fd(path->path.os9, sizeof(os9_stat), &os9_stat);
			statbuf->attributes = os9_stat.fd_att;
			statbuf->user_id = os9_stat.fd_own[1];
			statbuf->group_id = os9_stat.fd_own[0];
			memset(&timepak, 0, sizeof(timepak));
			timepak.tm_year = os9_stat.fd_creat[0];
			timepak.tm_mon = os9_stat.fd_creat[1] - 1;
			timepak.tm_mday = os9_stat.fd_creat[2];
			statbuf->create_time = mktime(&timepak);
			timepak.tm_year = os9_stat.fd_dat[0];
			timepak.tm_mon = os9_stat.fd_dat[1] - 1;
			timepak.tm_mday = os9_stat.fd_dat[2];
			timepak.tm_hour = os9_stat.fd_dat[3];
			timepak.tm_min = os9_stat.fd_dat[4];
			statbuf->last_modified_time = mktime(&timepak);
			break;
			
		case DECB:
			ec = _decb_gs_fd(path->path.decb, &decb_stat);
			/* Since Disk BASIC files have no permissions per se, we make our own. */
			statbuf->attributes = FAP_READ | FAP_WRITE | FAP_PREAD;
			if (path->path.decb->filename[0] == '\0')
			{
				statbuf->attributes |= FAP_DIR;
			}
			/* Neither does Disk BASIC have date or time stamps. */
			time(&tp);
			statbuf->create_time = tp;
			statbuf->last_modified_time = tp;
			/* Nor does it have user/group IDs. */
			statbuf->user_id = 0;
			statbuf->group_id = 0;
			break;
	}


	return ec;
}


error_code _coco_gs_size(coco_path_id path, int *size)
{
	error_code		ec = 0;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_gs_size(path->path.native, size);
			break;
			
		case OS9:
			ec = _os9_gs_size(path->path.os9, size);
			break;
			
		case DECB:
			ec = _decb_gs_size(path->path.decb, size);
			break;
	}
	
	
	return ec;
}
