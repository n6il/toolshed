/********************************************************************
 * cocofuse.c - FUSE compatible file system interface for RBF/Disk Basic
 *
 * $Id$
 ********************************************************************/
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <cocopath.h>

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION  26

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#endif

#include <fuse.h>

/* DSK image filename pointer */
static char *dsk;


/*
 * coco_getattr - returns file attributes
 *
 * Notes: code in this routine coverts coco_file_stat values into
 * values appropriate for the struct stat native to FUSE.
 */
static int coco_getattr(const char *path, struct stat *stbuf)
{
	error_code ec = 0;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0)
	{
		/* The root directory of our file system. */
		/* TODO: We really need to get this info from the root directory */
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 1;
    }
	else
    {
		coco_path_id p;
		coco_file_stat fdbuf;
		char buff[1024];

		sprintf(buff, "%s,%s", dsk, path);
		stbuf->st_mode = S_IFDIR;
		if (_coco_open(&p, buff, FAM_READ | FAM_DIR) != 0)
		{
			stbuf->st_mode = S_IFREG;
			if ((ec = -CoCoToUnixError(_coco_open(&p, buff, FAM_READ))) != 0)
			{
				return ec;
			}
		}

		if ((ec = -CoCoToUnixError(_coco_gs_fd(p, &fdbuf))) == 0)
		{
			u_int filesize;

			stbuf->st_mode |= CoCoToUnixPerms(fdbuf.attributes);

        	stbuf->st_nlink = 1;

			if (_coco_gs_size(p, &filesize) != 0)
			{
				filesize = 0;
			}
			stbuf->st_size = int4(filesize);
			#ifdef __linux__
			stbuf->st_ctime = fdbuf.create_time;
			stbuf->st_mtime = fdbuf.last_modified_time;
			#else
			stbuf->st_ctimespec.tv_sec = fdbuf.create_time;
			stbuf->st_mtimespec.tv_sec = fdbuf.last_modified_time;
			#endif
			stbuf->st_uid = getuid();
			stbuf->st_gid = getgid();
		}	

		_coco_close(p);
    }

    return ec;
}


/*
 * coco_mkdir - make a directory (OS-9 only)
 */	
static int coco_mkdir(const char *path, mode_t mode)
{
	error_code ec;
	char buff[1024];

	sprintf(buff, "%s,%s", dsk, path);
	ec = -CoCoToUnixError(_coco_makdir(buff));

	return ec;
}


/*
 * coco_unlink - removes the file specified in the path
 */
static int coco_unlink(const char *path)
{
	error_code ec;
	char buff[1024];

	sprintf(buff, "%s,%s", dsk, path);
	ec = -CoCoToUnixError(_coco_delete(buff));

	return ec;
}


/*
 * coco_rmdir - removes the directory specified in the path
 */
static int coco_rmdir(const char *path)
{
	error_code ec = 0;
	char buff[1024];

	sprintf(buff, "%s,%s", dsk, path);
//	ec = -CoCoToUnixError(_coco_deldir(buff)); //, CoCoToUnixPerm(mode));
	
	return ec;
}


/*
 * coco_rename - renames a file on a path
 *
 * Note: both paths are full pathlists.  It is important to determine if
 * the rename is in the same directory.  If not, then the source file must
 * be deleted (i.e. an mv command is being performed).
 */
static int coco_rename(const char *path, const char *newname)
{
	error_code ec = 0;
#if 0
	char *p1, *p2;
	char buff1[1024];
	int renameonly = 0;
	
	/* 1. Determine if rename is in same dir.
 	 *    - If so just rename.
	 *    - If not, rename then delete orginal.
	 */
	p1 = strrchr(path, '/');
	p2 = strrchr(newname, '/');
	
	if (p1 == NULL || p2 == NULL)
	{
		return -1;
	}
	
	*p1 = '\0'; *p2 = '\0';
	
	if (strcmp(path, newname) == 0)
	{
		renameonly = 1;
	}
	
	*p1 = '/'; *p2 = '/';
	
	sprintf(buff1, "%s,%s", dsk, path);
	ec = -CoCoToUnixError(_coco_rename(buff1, p2 + 1));
#endif
	
	return ec;
}


/*
 * coco_chmod - changes attributes of a file
 */
static int coco_chmod(const char *path, mode_t mode)
{
	error_code ec;
	char buff[1024];
	coco_path_id p;

	sprintf(buff, "%s,%s", dsk, path);
	if ((ec = -CoCoToUnixError(_coco_open(&p, buff, FAM_WRITE))) == 0);
	{
		ec = -CoCoToUnixError(_coco_ss_attr(p, UnixToCoCoPerms(mode)));
		_coco_close(p);
	}
	
	return ec;
}


/*
 * coco_truncate - truncates a file to a specific size
 */
static int coco_truncate(const char *path, off_t size)
{
	error_code ec = 0;
	char buff[1024];
	coco_path_id p;

	sprintf(buff, "%s,%s", dsk, path);
	ec = -CoCoToUnixError(_coco_open(&p, buff, FAM_WRITE));
	if (ec == 0)
	{
		ec = -CoCoToUnixError(_coco_ss_size(p, size));
		_coco_close(p);
	}
	
	return ec;
}


static int coco_open(const char *path, struct fuse_file_info *fi)
{
	error_code ec;
	coco_path_id p;
	char buff[1024];
	int mflags = FAM_READ;

	sprintf(buff, "%s,%s", dsk, path);

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
	{
		mflags |= FAM_WRITE;
	}
	if ((ec =  -CoCoToUnixError(_coco_open(&p, buff, mflags))) == 0)
	{
		fi->fh = (uint32_t)p;
	}

	return ec;
}


static int coco_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	error_code ec;
	uint32_t _size = size;

	coco_path_id p = (coco_path_id)(uint32_t)fi->fh;
	_coco_seek(p, offset, SEEK_SET);
	if ((ec = -CoCoToUnixError(_coco_read(p, buf, &_size))) != 0)
	{
		return ec;
	}

	return size;
}


static int coco_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	error_code ec;
	uint32_t _size = size;

	coco_path_id p = (coco_path_id)(uint32_t)fi->fh;
	_coco_seek(p, offset, SEEK_SET);
	if ((ec = -CoCoToUnixError(_coco_write(p, (char *)buf, &_size))) != 0)
	{
		return ec;
	}

	return size;
}


/*
 * coco_release - releases a previously opened path
 *
 * Notes: the path opened in coco_open is simply closed, which releases
 * internal file handles and memory.
 */
static int coco_release(const char *path, struct fuse_file_info *fi)
{
	error_code ec;
	
	ec = -CoCoToUnixError(_coco_close((coco_path_id)(int32_t)fi->fh));
	
	return ec;
}


static int coco_create(const char *path, mode_t perms, struct fuse_file_info * fi)
{
	error_code ec;
	coco_path_id p;
	char buff[1024];
	int mflags = FAM_READ | FAM_WRITE;
	int pflags = FAP_READ | FAP_WRITE;

	sprintf(buff, "%s,%s", dsk, path);

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
	{
		mflags |= FAM_WRITE;
	}

	if ((ec = -CoCoToUnixError(_coco_create(&p, buff, mflags, pflags))) != 0)
	{
		return ec;
	}

	fi->fh = (uint32_t)p;

	return 0;
}


static int coco_opendir(const char *path, struct fuse_file_info *fi)
{
	error_code ec;
	coco_path_id p;
	char buff[1024];
	int mflags = FAM_READ;

	sprintf(buff, "%s,%s", dsk, path);

	mflags |= FAM_DIR;

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
	{
		mflags |= FAM_WRITE;
	}
	if ((ec =  -CoCoToUnixError(_coco_open(&p, buff, mflags))) == 0)
	{
		fi->fh = (uint32_t)p;
	}

	return ec;
}


static int coco_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	coco_path_id p;
	coco_dir_entry e;
	char buff[1024];

#if !0
	sprintf(buff, "%s,%s", dsk, path);
	if (_coco_open(&p, buff, FAM_READ | FAM_DIR) != 0)
	{
		/* DECB doesn't use FAM_DIR */
		if (_coco_open(&p, buff, FAM_READ) != 0)
		{
			return -ENOENT;
		}
	}
#else
	p = (coco_path_id)fi->fh;
#endif

	while (_coco_readdir(p, &e) == 0)
	{
		switch (e.type)
		{
			case OS9:
				if (e.dentry.os9.name[0] != '\0')
				{
					/* entry is not empty, add it */
					filler(buf, (char *)OS9StringToCString(e.dentry.os9.name), NULL, 0);
				}
				break;

			case DECB:
				{
					u_char cstring[24];

					DECBStringToCString(e.dentry.decb.filename, e.dentry.decb.file_extension, cstring);
					filler(buf, (char *)cstring, NULL, 0);
				}
				break;

			default:
				break;
		}
	}	

#if !0
	_coco_close(p);
#endif

	return 0;
}


static int coco_utimens(const char *path, const struct timespec tv[2])
{
	return 0;
}

static struct fuse_operations coco_filesystem_operations =
{
	.getattr = coco_getattr,
	.mkdir = coco_mkdir,
	.unlink = coco_unlink,
	.rmdir = coco_rmdir,
	.rename = coco_rename,
	.chmod = coco_chmod,
	.readdir = coco_readdir,
	.open = coco_open,
	.read = coco_read,
	.write = coco_write,
	.release = coco_release,
	.create = coco_create,
	.opendir = coco_opendir,
	.releasedir = coco_release,
	#ifdef __linux__
	.utime = coco_utimens
	#else
	.utimens = coco_utimens
	#endif
};


int main(int argc, char **argv)
{
	dsk = argv[2];
	return fuse_main(argc - 1, argv, &coco_filesystem_operations, NULL);
}
