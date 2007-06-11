/********************************************************************
 * ss.c - Native SetStatus routines
 *
 * $Id$
 * Changed several conditionals for msys compatibility
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
/* Removed a conditional; RG*/
	struct utimbuf tbuff;

	
/* Removed a conditional; RG*/
	tbuff.actime = statbuf->st_ctime;
	tbuff.modtime = statbuf->st_mtime;


	/* 1. Update times. */

/* Removed a conditional; RG*/
	utime(path->pathlist, &tbuff);
/* #endif */


	/* 2. Update permissions. */

/* Removed a conditional; RG */
    chmod(path->pathlist, statbuf->st_mode);



    return ec;
}



error_code _native_ss_size(native_path_id path, int size)
{
    error_code	ec = 0;


	return ec;
}
