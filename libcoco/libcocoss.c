/********************************************************************
 * ss.c - CoCo SetStatus routines
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cocotypes.h"
#include "cococonv.h"
#include "cocopath.h"


error_code _coco_ss_attr(coco_path_id path, int perms)
{
	error_code		ec = 0;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_ss_attr(path->path.native, perms);
			break;
			
		case OS9:
			ec = _os9_ss_attr(path->path.os9, perms);
			break;
			
		case DECB:
//			ec = _decb_ss_attr(path->path.decb, perms);
			break;
		
		case CECB:
			fprintf( stderr, "_coco_ss_attr not implemented in libcecb yet.\n" );
			ec = -1;
			break;
	}
	
	
	return ec;
}



error_code _coco_ss_fd(coco_path_id path, coco_file_stat *statbuf)
{
	error_code		ec = 0;
	fd_stats		os9_stat;
//	decb_file_stat  decb_stat;
	struct stat		native_stat;
	struct tm		*timepak;	

	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_gs_fd(path->path.native, &native_stat);
			native_stat.st_mode &= ~(S_IRWXU);
#if !defined(__MINGW32__) && !defined(BDS) && !defined(__CYGWIN__)
			native_stat.st_mode &= ~(S_IRWXO);
#endif
			if (statbuf->attributes & FAP_READ) { native_stat.st_mode |= S_IRUSR; }
			if (statbuf->attributes & FAP_WRITE) { native_stat.st_mode |= S_IWUSR; }
			if (statbuf->attributes & FAP_EXEC) { native_stat.st_mode |= S_IXUSR; }
#if !defined(__MINGW32__) && !defined(BDS) && !defined(__CYGWIN__)
			if (statbuf->attributes & FAP_PREAD) { native_stat.st_mode |= S_IROTH; }
			if (statbuf->attributes & FAP_PWRITE) { native_stat.st_mode |= S_IWOTH; }
			if (statbuf->attributes & FAP_PEXEC) { native_stat.st_mode |= S_IXOTH; }
#endif
			native_stat.st_uid = statbuf->user_id;
			native_stat.st_gid = statbuf->group_id;
#if defined(__MINGW32__) || defined(BDS) || defined(__CYGWIN__)
			native_stat.st_ctime = statbuf->create_time;
			native_stat.st_mtime = statbuf->last_modified_time;
#else
#ifdef __APPLE__
			native_stat.st_ctimespec.tv_sec = statbuf->create_time;
			native_stat.st_mtimespec.tv_sec = statbuf->last_modified_time;
#else
			native_stat.st_ctim.tv_sec = statbuf->create_time;
			native_stat.st_mtim.tv_sec = statbuf->last_modified_time;
#endif
#endif
			ec = _native_ss_fd(path->path.native, &native_stat);
			break;
			
		case OS9:
			ec = _os9_gs_fd(path->path.os9, sizeof(os9_stat), &os9_stat);
			os9_stat.fd_att &= ~(FAP_READ | FAP_WRITE | FAP_EXEC | FAP_PREAD | FAP_PWRITE | FAP_PEXEC);
			os9_stat.fd_att |= statbuf->attributes;
			os9_stat.fd_own[1] = statbuf->user_id;
			os9_stat.fd_own[0] = statbuf->group_id;
			timepak = localtime(&(statbuf->create_time));
			os9_stat.fd_creat[0] = timepak->tm_year;
			os9_stat.fd_creat[1] = timepak->tm_mon + 1;
			os9_stat.fd_creat[2] = timepak->tm_mday;
			timepak = localtime(&(statbuf->last_modified_time));
			os9_stat.fd_dat[0] = timepak->tm_year;
			os9_stat.fd_dat[1] = timepak->tm_mon + 1;
			os9_stat.fd_dat[2] = timepak->tm_mday;
			os9_stat.fd_dat[3] = timepak->tm_hour;
			os9_stat.fd_dat[4] = timepak->tm_min;
			ec = _os9_ss_fd(path->path.os9, sizeof(os9_stat), &os9_stat);
			break;
			
		case DECB:
			break;
		
		case CECB:
			fprintf( stderr, "_coco_ss_fd not implemented in libcecb yet.\n" );
			ec = -1;
			break;
	}
	
	
	return ec;
}



error_code _coco_ss_size(coco_path_id path, int size)
{
	error_code		ec = 0;
	
	
    /* 1. Call appropriate function. */
	
	switch (path->type)
	{
		case NATIVE:
			ec = _native_ss_size(path->path.native, size);
			break;
			
		case OS9:
			ec = _os9_ss_size(path->path.os9, size);
			break;
			
		case DECB:
			ec = _decb_ss_size(path->path.decb, size);
			break;
		
		case CECB:
			fprintf( stderr, "_coco_ss_size not implemented in libcecb yet.\n" );
			ec = -1;
			break;
	}
	
	
	return ec;
}
