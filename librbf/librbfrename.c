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
#include <util.h>


error_code _os9_rename(char *pathlist, char *new_name)
{
	os9_dir_entry dentry;

	return _os9_rename_ex(pathlist, new_name, &dentry);
}


error_code _os9_rename_ex(char *pathlist, char *new_name, os9_dir_entry *dentry)
{
    error_code	ec = 0;
    os9_path_id parent_path;
	char filename[33];

	if (strcasecmp(new_name, "." ) == 0 || (new_name[0] == '.' && new_name[1] == '.'))
	{
		return(EOS_IA);
	}

	ec = _os9_prsnam(new_name);

	if (ec != 0)
	{
		return ec;
	}

	ec = _os9_open_parent_directory(&parent_path, pathlist, FAM_DIR | FAM_WRITE, filename);

	if (ec != 0)
	{
		return(ec);
	}

	/* Return on illegal filename */

	if (strcasecmp(filename, ".") == 0 || strcasecmp(filename, ".." ) == 0)
	{
		_os9_close(parent_path);

        return(EOS_IA);
    }

	/* See if another file in this directory has the same name as our destination */

	while ((ec = _os9_gs_eof(parent_path)) == 0)
	{
		int size;
		char fname[32];

		size = sizeof(dentry);
		ec = _os9_readdir(parent_path, dentry);

		if (ec != 0 || size != sizeof(dentry))
		{
			break;
		}

		strncpy(fname, (char *)dentry->name, 29);

		OS9StringToCString((u_char *)fname);

		if (strcasecmp(fname, new_name) == 0 && strcasecmp(fname, filename) != 0)
		{
			ec = EOS_FAE;

			break;
		}
	}

	if (ec == EOS_FAE)
	{
		_os9_close(parent_path);

		return ec;
	}

	_os9_seek(parent_path, 0, SEEK_SET);

	/* Start reading directory file and search for match */

	while ((ec = _os9_gs_eof(parent_path)) == 0)
	{
		int size;
		char fname[32];

		size = sizeof(dentry);
		ec = _os9_readdir(parent_path, dentry);

		if (ec != 0 || size != sizeof(dentry))
		{
			break;
		}

		strncpy(fname, (char *)dentry->name, 29);

		OS9StringToCString((u_char *)fname);

		if (strcasecmp(fname, filename) == 0)
		{
			/* Found the source, rename it */
			strncpy((char *)dentry->name, new_name, 29);
			CStringToOS9String(dentry->name);

			/* Back up file pointer in preparation of updating directory entry */
			_os9_seek(parent_path, -(int)sizeof(*dentry), SEEK_CUR);

			/* Write the directory entry back to the image */
			ec = _os9_writedir(parent_path, dentry);

			break;
		}
	}

	_os9_close(parent_path);

	return(ec);
}
