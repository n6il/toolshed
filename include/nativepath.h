/********************************************************************
 * nativepath.h - Native path definitions header file
 *
 * $Id$
 ********************************************************************/
#ifndef	_NATIVEPATH_H
#define	_NATIVEPATH_H

#include <stdio.h>
#include <sys/stat.h>
#include <cocotypes.h>
#include <cococonv.h>
#include <dirent.h>


typedef struct _native_path_id
{
	int			mode;		/* access mode */
	char		pathlist[512];	/* pointer to pathlist */
	FILE		*fd;		/* file path pointer */
	DIR			*dirhandle;
} *native_path_id;


#ifdef __MINGW32__
typedef struct _finddata_t  native_dir_entry;
#else
typedef struct dirent		native_dir_entry;
#endif


/* prototypes */

error_code _native_open(native_path_id *, char *, int);
error_code _native_create(native_path_id *, char *, int, int);
error_code _native_read(native_path_id, void *, int *);
error_code _native_readdir(native_path_id, native_dir_entry *);
error_code _native_seek(native_path_id, int, int);
error_code _native_allbit(u_char *bitmap, int firstbit, int numbits);
error_code _native_delbit(u_char *bitmap, int firstbit, int numbits);
error_code _native_readln(native_path_id, void *, int *);
error_code _native_write(native_path_id, void *, int *);
error_code _native_writeln(native_path_id, char *, int *);
error_code _native_makdir(char *pathlist);
error_code _native_delete(char *pathlist);
error_code _native_rename(char *pathlist, char *new_name);
error_code _native_close(native_path_id);

/* gs.c */
error_code _native_gs_attr(native_path_id, int *);
error_code _native_gs_eof(native_path_id path);
error_code _native_gs_fd(native_path_id, struct stat *);
error_code _native_gs_size(native_path_id path, int *size);

/* ss.c */
error_code _native_ss_attr(native_path_id, int);
error_code _native_ss_fd(native_path_id, struct stat *);
error_code _native_ss_size(native_path_id path, int size);

#include "cocopath.h"

#endif	/* _NATIVEPATH_H */

