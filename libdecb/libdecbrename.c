/********************************************************************
 * rename.c - Rename file entry for Disk BASIC
 *
 * $Id$
 ********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "cocotypes.h"
#include "cocosys.h"
#include "decbpath.h"
#include "cococonv.h"
#include "util.h"


error_code _decb_rename(char *pathlist, char *new_name)
{
	decb_dir_entry dirent;

	return _decb_rename_ex(pathlist, new_name, &dirent);
}



error_code _decb_rename_ex(char *pathlist, char *new_name, decb_dir_entry *dirent)
{
	error_code	ec = 0;
	char filename[8], extension[3];
	decb_path_id path;
	char *old_name;

	/* 1. Test if path is native. */
	if ((old_name = strchr(pathlist, ',')) == NULL)
	{
		return EOS_BPNAM;
	}
	old_name++;

	/* 2. Validate new name */
	ec = _decb_prsnam(new_name);
	if (ec != 0)
	{
		return ec;
	}

	/* 3. Convert filename to format required by dentry */
	CStringToDECBString(filename, extension, old_name);

	/* 4. Open a path to the file */
	ec = _decb_open(&path, pathlist, FAM_READ| FAM_WRITE);

	if (ec != 0)
	{
		return ec;
	}

	/* 5. Start reading directory file and search for match */
	_decb_seekdir(path, 0);

	/* Presume error for now */
	ec = EOS_BPNAM;

	while (_decb_readdir(path, dirent) == 0)
	{
		if (!strncasecmp(dirent->filename, filename, 8) && !strncasecmp(dirent->file_extension, extension, 3))
		{
			/* 1. Found the source, rename it. */
			CStringToDECBString(filename, extension, new_name);
			memcpy(dirent->filename, filename, 8);
			memcpy(dirent->file_extension, extension, 3);

			/* 2. Write the directory entry back to the image. */

			ec = _decb_writedir(path, dirent);

			break;
		}
	}

	_decb_close(path);

	return ec;
}
