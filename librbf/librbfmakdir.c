/********************************************************************
 * makdir.c - Create an os9 directory
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "cocotypes.h"
#include "os9path.h"
#include "cococonv.h"



error_code _os9_makdir( char *pathlist )
{
    error_code		ec = 0;
    os9_path_id 	parent_path, dir_path;
    char 			filename[32];
    int				parentLSN;
    os9_dir_entry   newEntry;
    char			t_path[256];
	

    /* 1. Store LSN of parent. */
	
    strcpy(t_path, pathlist);
	
    ec = _os9_open_parent_directory(&parent_path, t_path, FAM_DIR | FAM_WRITE, filename);

    if (ec == 0)
	{
		parentLSN = parent_path->pl_fd_lsn;
	
		ec = _os9_close(parent_path);

		if (ec == 0)
		{
			/* Create new directory */

			strcpy(t_path, pathlist);

			ec = _os9_create(&dir_path, t_path, FAM_NOCREATE | FAM_DIR | FAM_WRITE, FAP_DIR | FAP_READ | FAP_WRITE | FAP_EXEC | FAP_PREAD | FAP_PWRITE | FAP_PEXEC);

			if (ec == 0)
			{
				/* Fill new directory with initial entries */
				/* Create '..' directory */

				memset(&newEntry, 0, sizeof(os9_dir_entry));
	
				strcpy( newEntry.name, "..");
				StringToOS9Name(newEntry.name);
				_int3(parentLSN, newEntry.lsn);
	
				ec = _os9_writedir(dir_path, &newEntry);

				if (ec == 0)
				{
					/* Create '.' directory */

					memset(&newEntry, 0, sizeof(os9_dir_entry));
	
					strcpy(newEntry.name, ".") ;
					StringToOS9Name( newEntry.name);
					_int3(dir_path->pl_fd_lsn, newEntry.lsn);

					ec = _os9_writedir(dir_path, &newEntry);

					if (ec == 0)
					{
						ec = _os9_close(dir_path);
					}
				}
			}
		}
	}
	

    return ec;
}
