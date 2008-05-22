/********************************************************************
 * libcecbgs.c - Cassette BASIC GetStat routines
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "cecbpath.h"

error_code _cecb_gs_fd(cecb_path_id path, cecb_file_stat *stat)
{
    error_code		ec = 0;

	
	stat->file_type = path->dir_entry.file_type;
	stat->data_type = path->dir_entry.ascii_flag;
	stat->gap_flag = path->dir_entry.gap_flag;
	stat->ml_load_address = (path->dir_entry.ml_load_address1 << 8) + path->dir_entry.ml_load_address2;
	stat->ml_exec_address = (path->dir_entry.ml_exec_address1 << 8) + path->dir_entry.ml_exec_address2;

	return ec;
}

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

