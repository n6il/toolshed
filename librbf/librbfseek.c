/********************************************************************
 * seek.c - OS-9 Seek routine
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <cocotypes.h>
#include <os9path.h>


error_code _os9_seek(os9_path_id path, int pos, int mode)
{
    error_code	ec = 0;

    if (path->israw == 1)
    {
        fseek(path->fd, pos, mode);
    }
    else
    {
        switch( mode )
        {
            case SEEK_SET:
                path->filepos = pos;
                break;
            case SEEK_CUR:
                path->filepos = path->filepos + pos;
                break;
            case SEEK_END:
                fprintf( stderr, "_os9_seek(): SEEK_END not implemented.\n" );
                exit(0);
                break;
        }
    }

    return(ec);
}
