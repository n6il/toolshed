/********************************************************************
 * ss.c - OS-9 SetStatus routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cocotypes.h"
#include "os9path.h"
#include "cococonv.h"


error_code _os9_ss_attr(os9_path_id path, int perms)
{
    error_code	ec = 0;
    fd_stats fdbuf;


    ec = _os9_gs_fd(path, sizeof(fd_stats), &fdbuf);
	
    if (ec == 0)
    {
        fdbuf.fd_att = perms;
        ec = _os9_ss_fd(path, sizeof(fd_stats), &fdbuf);
    }


    return ec;
}



error_code _os9_ss_fd(os9_path_id path, int count, fd_stats *fdbuf)
{
    error_code	ec = 0;
    int size;

    {
        /* seek to FD LSN of pathlist */
        fseek(path->fd, path->pl_fd_lsn * path->bps, SEEK_SET);	
	
        /* write the file descriptor sector */
        size = sizeof(fd_stats);
        if (count < size)
        {
            size = count;
        }
        fwrite(fdbuf, 1, size, path->fd);
    }

	
    return(ec);
}


error_code _os9_ss_size(os9_path_id path, int size)
{
    error_code	ec = 0;
    fd_stats fdbuf;


    /* if path is raw, return entire disk as size */
    if (path->israw == 1)
    {
        return(ec);
    }

    ec = _os9_gs_fd(path, sizeof(fdbuf), &fdbuf);
    if (ec != 0)
    {
        return(ec);
    }

    _int4(size, fdbuf.fd_siz);

    ec = _os9_ss_fd(path, sizeof(fdbuf), &fdbuf);

    return(ec);
}
