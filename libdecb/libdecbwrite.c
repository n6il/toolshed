/********************************************************************
 * write.c - Disk BASIC Write routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <decbpath.h>


static error_code _raw_write(decb_path_id path, void *buffer, int *size);


error_code _decb_write(decb_path_id path, void *buffer, int *size)
{
    error_code	ec = EOS_WRITE;
	

	/* 1. Check the mode. */
	
	if (path->mode & FAM_DIR || path->mode & FAM_WRITE == 0)
    {
        /* 1. Must be a directory. */

        return EOS_BMODE;
    }


    /* 2. Treat raw path differently. */
	
    if (path->israw == 1)
    {
        ec = _raw_write(path, buffer, size);
    }
#if 0
    else
    {
        fd_stats fd_sector;
        fd_seg segptr;
        int i;
        int accum_size = 0;
        int bytes_left;
        char *buf_ptr = buffer;
        int seg_size_bytes, write_size;
        int filesize;

        /* 1. Seek to FD LSN of pathlist */
        fseek(path->fd, path->pl_fd_lsn * path->bps, SEEK_SET);	
	
        /* 2. Read the file descriptor sector */
        fread(&fd_sector, 1, sizeof(fd_stats), path->fd);
	
        /* 3. Point to segment list */
        segptr = (fd_seg)&(fd_sector.fd_seg);
	
        /* 4. Extract file size from FD */
        filesize = int4(fd_sector.fd_siz);

        /* 5. If our file position is greater than the file size, return error */
        if (path->filepos > filesize)
        {
            /* end of file */
            return(EOS_EOF);
        }

        /* 6a. Determine maximum file size by adding up the segment list */
        accum_size = _os9_maximum_file_size( fd_sector, path->bps );

        /* 6b. If there is not enough room, we need to extend the segment list */
        while (accum_size < path->filepos + *size)
        {
            int delta;
			
            /* We need to enlarge the file. Delta will contain the number of bytes added */
            ec = _os9_extendSegList(path, segptr, &delta);
			
            /* If there is an error, time to abort */
            if (ec != 0)
            {
                return(ec);
            }
			
            accum_size += delta;
        }

        /* 7. Determine which segment the offset starts by looping
         *    through each segptr entry until we reach the end
         */
        accum_size = 0;

        for (i = 0; i < NUM_SEGS && int3(segptr[i].lsn) != 0; i++)
        {
            accum_size += int2(segptr[i].num) * path->bps;
            if (accum_size > path->filepos)
            {
                /* this is the sector! */
                accum_size -= int2(segptr[i].num) * path->bps;
                break;
            }
        }
	
        /* 8. make out-of-loop check to insure we exited from the for()
         *    loop due to finding the proper sector, and not because we
         *    ran out of sectors in the segment list to search.
         */
        if (int3(segptr[i].lsn) == 0 || i == NUM_SEGS)
        {
            /* Apparently, the file position in the path was too
             * large, because we couldn't find a sector.
             */
            return 1;
        }

        /* 9. Start copying the user supplied data into the file for 'bytes_left' bytes.
         *
         * i == segment entry to start
         */
        bytes_left = *size;
        while (bytes_left > 0 && i != NUM_SEGS && int3(segptr[i].lsn) != 0)
        {
            accum_size += int2(segptr[i].num) * path->bps;
	
            /* seek to sector where segment starts and compute the segment size */
            fseek(path->fd, int3(segptr[i].lsn) * path->bps, SEEK_SET);
            seg_size_bytes = int2(segptr[i].num) * path->bps;
	
            /* seek within segment to offset */
            fseek(path->fd, path->filepos - (accum_size - seg_size_bytes), SEEK_CUR);
	
            /* compute write size for this segment */
            write_size = accum_size - path->filepos;
            if (write_size > bytes_left)
            {
                write_size = bytes_left;
            }
            fwrite(buf_ptr, 1, write_size, path->fd);
            buf_ptr += write_size;
            path->filepos += write_size;
            bytes_left -= write_size;
            i++;
        }

        /* 10. Update fd_siz if necessary. */
        if( path->filepos > int4(fd_sector.fd_siz) )
        {
            /* Update file size */
            _int4( path->filepos, fd_sector.fd_siz );
        }
	
        /* 11. TODO - Update modification date/time */
		
        /* 12. Write updated file descriptor back to image file */
        fseek(path->fd, path->pl_fd_lsn * path->bps, SEEK_SET);	
        fwrite(&fd_sector, 1, sizeof(fd_stats), path->fd);
    }
#endif

    return ec;
}



static error_code _raw_write(decb_path_id path, void *buffer, int *size)
{
    error_code	ec = 0;
    size_t ret_size;


    ret_size = fwrite(buffer, 1, *size, path->fd);
    *size = ret_size;


    return ec;
}



error_code _decb_writedir(decb_path_id path, decb_dir_entry *dirent)
{
    error_code	ec = 0;
	char buffer[256];
	int sector = (path->directory_entry_index * sizeof(decb_dir_entry)) / 256;
	int entry_in_sector;
	

#if 0
	/* 1. Check the mode. */
	
	if (path->mode & FAM_DIR == 0 || path->mode & FAM_WRITE == 0)
    {
        return EOS_BMODE;
    }
#endif

	entry_in_sector = (path->directory_entry_index * sizeof(decb_dir_entry)) % 256;


	/* 2. Check if we have passed last entry. */
	
	if (path->directory_entry_index == 73)
	{
		/* 1. Yes.  Return error. */
		
		return EOS_DF;
	}
	
	
	ec = _decb_gs_sector(path, 17, 3 + sector, buffer);

	if (ec == 0)
	{
		/* 1. Check if this is an empty entry. */
		
		memcpy(buffer + entry_in_sector, dirent, sizeof(decb_dir_entry));

		ec = _decb_ss_sector(path, 17, 3 + sector, buffer);
	}


    return ec;
}
