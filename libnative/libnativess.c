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
#include <utime.h>
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
#if defined(__APPLE__) || defined(__MIGW32__)
	struct timeval tbuff;
#else
	struct utimbuf tbuff;
#endif

	
#if defined(__APPLE__)
	tbuff[0].tv_sec = statbuf->st_ctimespec.tv_sec;
	tbuff[1].tv_sec = statbuf->st_mtimespec.tv_sec;
#elif defined(__MINGW32__)
	tbuff[0].tv_sec = statbuf->st_ctime;
	tbuff[1].tv_sec = statbuf->st_mtime;
#else
	tbuff.actime = statbuf->st_ctime;
	tbuff.modtime = statbuf->st_mtime;
#endif


	/* 1. Update times. */

#if defined(__APPLE__) || defined(__MINGW__)
	utimes(path->pathlist, tbuff);
#else
	utime(path->pathlist, &tbuff);
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
