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
	char nfilename[8], nextension[3];
	decb_path_id path;
	char *old_name;

	/* 1. Test if path is native. */
	if ((old_name = strchr(pathlist, ',')) == NULL)
	{
		return EOS_BPNAM;
	}
	old_name++;

	/* 2. Validate old and new name */
	ec = _decb_prsnam(old_name);
	if (ec != 0)
	{
		return ec;
	}

	ec = _decb_prsnam(new_name);
	if (ec != 0)
	{
		return ec;
	}

	/* 3. Convert filename to format required by dentry */
	CStringToDECBString(filename, extension, old_name);
	CStringToDECBString(nfilename, nextension, new_name);

	/* 4. Open a path to the file */
	ec = _decb_open(&path, pathlist, FAM_READ| FAM_WRITE);

	if (ec != 0)
	{
		return ec;
	}

	/* 5. Start reading directory file and search for match */
	_decb_seekdir(path, 0, SEEK_SET);

	/* Presume error for now */
	ec = EOS_BPNAM;

	/* See if another file in this directory has the same name as our destination */
	while (_decb_readdir(path, dirent) == 0)
	{
		if (!strncmp(dirent->filename, nfilename, 8) && !strncmp(dirent->file_extension, nextension, 3))
		{
			ec = EOS_FAE;

			break;
		}
	}

	if (ec == EOS_FAE)
	{
		_decb_close(path);

		return ec;
	}

	_decb_seekdir(path, 0, SEEK_SET);

	/* Start reading directory file and search for match */

	while (_decb_readdir(path, dirent) == 0)
	{
		if (!strncmp(dirent->filename, filename, 8) && !strncmp(dirent->file_extension, extension, 3))
		{
			/* 1. Found the source, rename it. */
			memcpy(dirent->filename, nfilename, 8);
			memcpy(dirent->file_extension, nextension, 3);

			/* 2. Write the directory entry back to the image. */
			_decb_seekdir(path, -1, SEEK_CUR);
			ec = _decb_writedir(path, dirent);

			break;
		}
	}

	_decb_close(path);

	return ec;
}
