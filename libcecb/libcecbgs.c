/********************************************************************
 * libcecbgs.c - Cassette BASIC GetStat routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cecbpath.h"

error_code _cecb_gs_eof(cecb_path_id path)
{
    error_code		ec = 0;

	if( (path->eof_flag == 1) && (path->current_pointer == path->length) )
			ec = EOS_EOF;

    return ec;
}

error_code _cecb_gs_pos(cecb_path_id path, u_int *pos)
{
    error_code	ec = 0;

	*pos = path->filepos;

    return ec;
}

