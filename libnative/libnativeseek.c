/********************************************************************
 * seek.c - Native Seek routine
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>

#include "cocotypes.h"
#include "nativepath.h"



error_code _native_seek(native_path_id path, int pos, int mode)
{
    error_code	ec = 0;


    fseek(path->fd, pos, mode);


    return ec;
}
