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
error_code _coco_gs_pathtype(coco_path_id path, _path_type *disk_type)
{
	error_code		ec = 0;

    *disk_type = path->type;
	
	
	return ec;
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
#if !defined(BDS)
			if (native_stat.st_mode & S_IWUSR)
#endif
			{ statbuf->attributes |= FAP_WRITE; }
			if (native_stat.st_mode & S_IXUSR) { statbuf->attributes |= FAP_EXEC; }
#if !defined(__MINGW32__) && !defined(BDS)
			if (native_stat.st_mode & S_IROTH) { statbuf->attributes |= FAP_PREAD; }
			if (native_stat.st_mode & S_IWOTH) { statbuf->attributes |= FAP_PWRITE; }
			if (native_stat.st_mode & S_IXOTH) { statbuf->attributes |= FAP_PEXEC; }
#endif
			if (native_stat.st_mode & S_IFDIR) { statbuf->attributes |= FAP_DIR; }
			statbuf->user_id = native_stat.st_uid;
			statbuf->group_id = native_stat.st_gid;
#if !defined(__MINGW32__) && !defined(BDS)
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


error_code _coco_gs_fd_pathlist(char *pathlist, coco_file_stat *statbuf)
{
    error_code	ec = 0;
	coco_path_id path;
	
	/* Open a path to the pathlist */
	
	ec = _coco_open(&path, pathlist, FAM_READ);
	
	if (ec != 0)
	{
		ec = _coco_open(&path, pathlist, FAM_READ | FAM_DIR);
		
		if (ec != 0)
		{
			return ec;
		}
	}
	
	
	ec = _coco_gs_fd(path, statbuf);
	
	_coco_close(path);
	

    return ec;
}



error_code _coco_gs_size(coco_path_id path, u_int *size)
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




error_code _coco_gs_size_pathlist(char *pathlist, u_int *size)
{
    error_code	ec = 0;
	coco_path_id path;
	
	/* Open a path to the pathlist */
	
	ec = _coco_open(&path, pathlist, FAM_READ);
	
	if (ec != 0)
	{
		ec = _coco_open(&path, pathlist, FAM_READ | FAM_DIR);
		
		if (ec != 0)
		{
			return ec;
		}
	}
	
	
	ec = _coco_gs_size(path, size);
	
	_coco_close(path);
	

    return ec;
}



error_code _coco_gs_pos(coco_path_id path, u_int *pos)
{
	error_code		ec = 0;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_gs_pos(path->path.native, pos);
			break;
			
		case OS9:
			ec = _os9_gs_pos(path->path.os9, pos);
			break;
			
		case DECB:
			ec = _decb_gs_pos(path->path.decb, pos);
			break;
	}
	
	
	return ec;
}
