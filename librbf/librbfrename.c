/********************************************************************
 * rename.c - Rename file entry for OS-9 filesystems
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <cocotypes.h>
#include <os9path.h>
#include <cococonv.h>
#include <cocosys.h>


error_code _os9_rename( char *pathlist, char *new_name )
{
    error_code	ec = 0;
    os9_path_id parent_path;
    char filename[33];

    if( !strcasecmp(new_name, "." ))
        return(EOS_IA);
	
    if( !strcasecmp(new_name, ".." ))
        return(EOS_IA);
	
    ec = _os9_prsnam( new_name );
    if( ec != 0 )
        return EOS_BPNAM;

    ec = _os9_open_parent_directory( &parent_path, pathlist, FAM_DIR | FAM_WRITE, filename );

    if( ec != 0 )
    {
        return( ec );
    }

    /* return on illegal filename */
	
    if( !strcasecmp(filename, "." ))
    {
        _os9_close( parent_path );
        return(EOS_IA);
    }
			
    if( !strcasecmp(filename, ".." ))
    {
        _os9_close( parent_path );
        return(EOS_IA);
    }

    /* Start reading directory file and search for match */

    while (_os9_gs_eof(parent_path) == 0)
    {
        int size;
        os9_dir_entry dentry;
        char fname[32];

        size = sizeof(dentry);
        ec = _os9_read(parent_path, &dentry, &size);

        if( ec != 0 || size != sizeof(dentry) )
        {
            break;
        }
		
        strncpy(fname, dentry.name, 29);
		
        OS9NameToString(fname);
		
        if( !strcasecmp( fname, filename ) )
        {
            /* Found the source, rename it */
			
            strncpy( dentry.name, new_name, 29 );
            StringToOS9Name( dentry.name );
			
            /* Back up file pointer in preparation of updating directory entry */
            _os9_seek( parent_path, -(int)sizeof(dentry), SEEK_CUR );
			
            /* Write the directory entry back to the image */
            ec = _os9_writedir(parent_path, &dentry);
			
            break;			
        }
    }
	
    _os9_close(parent_path);

    return(0);
}


