/********************************************************************
 * ss.c - Native SetStatus routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#ifdef BDS
#include <time.h>
#include <utime.h>
#include <io.h>
#else
#include <sys/time.h>
#endif
#include <sys/stat.h>

#include "cocotypes.h"
#include "cococonv.h"
#include "cocopath.h"



error_code _native_ss_attr(native_path_id path, int perms)
{
    error_code	ec = 0;
    struct stat statbuf;


    ec = _native_gs_fd(path, &statbuf);

    if (ec == 0)
    {
        statbuf.st_mode = perms;

        ec = _native_ss_fd(path, &statbuf);
    }


    return ec;
}



error_code _native_ss_fd(native_path_id path, struct stat *statbuf)
{
    error_code	ec = 0;
#ifdef BDS
	struct utimbuf tbuff;
#else
	struct timeval tarray[2];
#endif

	
#if defined(__APPLE__)
	tarray[0].tv_sec = statbuf->st_ctimespec.tv_sec;
	tarray[1].tv_sec = statbuf->st_mtimespec.tv_sec;
#elif defined(__MINGW32__)
	tarray[0].tv_sec = statbuf->st_ctime;
	tarray[1].tv_sec = statbuf->st_mtime;
#elif defined(BDS)
	tbuff.actime = statbuf->st_ctime;
	tbuff.modtime = statbuf->st_mtime;
#else
	tarray[0].tv_sec = statbuf->st_ctim.tv_sec;
	tarray[1].tv_sec = statbuf->st_mtim.tv_sec;
#endif


	/* 1. Update times. */

#if defined(__APPLE__)
	utimes(path->pathlist, tarray);
#elif defined(BDS)
	utime(path->pathlist, &tbuff);
#else
	utime(path->pathlist, tarray[0]);
#endif


	/* 2. Update permissions. */

#if defined(__APPLE__) || defined(__MINGW32__)
	chmod(path->pathlist, statbuf->st_mode);
#else
	chmod(path->pathlist, statbuf->st_mode);
#endif


    return ec;
}



error_code _native_ss_size(native_path_id path, int size)
{
    error_code	ec = 0;


	return ec;
}
