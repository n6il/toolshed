/********************************************************************
 * delete.c - Delete file entry for OS-9 filesystems
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "cocotypes.h"
#include "os9path.h"
#include "cococonv.h"


u_char DecrementLinkCount(os9_path_id path, int fd_lsn);
static int _os9_freefile(char *filePath, u_char *bitmap);


error_code _os9_delete(char *pathlist)
{
    error_code	ec = 0;
    os9_path_id parent_path;
    char filename[33];
	int deleted = 0;


    /* 1. Determine is path is a folder. */
	
    ec = _os9_open(&parent_path, pathlist, FAM_DIR | FAM_READ);

    if (ec == 0)
    {
        /* Cannot use this to delete directories */
        /* You must remove the contents of the directory and change
         * the directory file into a regular file
         */

        _os9_close(parent_path);

        return(EOS_IC);
    }
	
    ec = _os9_open_parent_directory( &parent_path, pathlist, FAM_DIR | FAM_WRITE, filename );

    if (ec != 0)
    {
        return(ec);
    }


    /* 2. Return on illegal filename. */
		
    if( !strcasecmp(filename, "." ))
    {
        _os9_close(parent_path);

        return(EOS_IA);
    }
	
    if( !strcasecmp(filename, ".." ))
    {
        _os9_close(parent_path);

        return(EOS_IA);
    }

	
    /* 3. Start reading directory file and search for match. */

    while (_os9_gs_eof(parent_path) == 0)
    {
        int size;
        os9_dir_entry dentry;
        char fname[32];


        size = sizeof(dentry);
        ec = _os9_read(parent_path, &dentry, &size);

        if (ec != 0 || size != sizeof(dentry))
        {
            break;
        }
		
        strncpy(fname, dentry.name, 29);
		
        OS9NameToString(fname);
		
        if (!strcasecmp(fname, filename))
        {			
            /* Decrement link count in file descriptor */

            if (DecrementLinkCount(parent_path, int3(dentry.lsn)) < 1)
            {
                /* Only deallocate file if link count is zero */
				
                /* Deallocate any bits used by the file */
                ec = _os9_freefile( pathlist, parent_path->bitmap );
				
                /* Deallocate the bit used for the file descriptor */
                _os9_delbit( parent_path->bitmap, int3(dentry.lsn) / parent_path->spc, 1 );
            }

            /* Back up file pointer in preparation of updating directory entry */
            _os9_seek( parent_path, -(int)sizeof(dentry), SEEK_CUR );
			
            /* Putting a NULL in the first charcter position identifies the file
                as deleted */
            dentry.name[0] = '\0';
			
            /* Write the directory entry back to the image */
            ec = _os9_write( parent_path, &dentry, &size );

			/* Flag that the file has been deleted. */
			deleted = 1;
			
            break;			
        }
    }
	
    _os9_close(parent_path);
    
	if (deleted == 0)
	{
		ec = EOS_PNNF;
	}
	
	
    return(ec);
}


u_char DecrementLinkCount( os9_path_id path, int fd_lsn )
{
    fd_stats	fdbuf;
    u_char		result;
	
    fseek(path->fd, fd_lsn * path->bps, SEEK_SET);	
    fread(&fdbuf, 1, sizeof(fd_stats), path->fd);
	
    result = fdbuf.fd_lnk = fdbuf.fd_lnk - 1;
	
    fseek(path->fd, fd_lsn * path->bps, SEEK_SET);	
    fwrite(&fdbuf, 1, sizeof(fd_stats), path->fd);

    return result;
}


static int _os9_freefile( char *filePath, u_char *bitmap )
{
    os9_path_id path;
    fd_stats fdbuf;
    Fd_seg	seg;
    int i;
    int	ec = 0;
	
    ec = _os9_open( &path, filePath, FAM_READ );
    ec = _os9_gs_fd( path, sizeof(fd_stats), &fdbuf );
    ec = _os9_close(path);
	
    seg = fdbuf.fd_seg;
	
    for( i = 0; i<NUM_SEGS; i++ )
    {
        if( int3(seg[i].lsn) == 0 )
            break;

        ec = _os9_delbit( bitmap, int3(seg[i].lsn) / path->spc, int2(seg[i].num) / path->spc );
		
        if( ec != 0 )
            return ec;
    }

    return ec;	
}
