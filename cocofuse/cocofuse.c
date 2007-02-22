#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <cocopath.h>

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION  26
#include <fuse.h>

static char *dsk;

static int coco_unlink(const char *path)
{
	error_code ec;
	char buff[1024];

	sprintf(buff, "%s,%s", dsk, path);
	ec = _coco_delete(buff);
	if (ec != 0)
	{
		return -CoCoToUnixError(ec);
	}
}


static int coco_getattr(const char *path, struct stat *stbuf)
{
	error_code ec;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 3;
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
		if ((ec = _coco_open(&p, buff, FAM_READ)) != 0)
		{
			return -CoCoToUnixError(ec);
		}
	}

	if (_coco_gs_fd(p, &fdbuf) == 0)
	{
		u_int filesize;

		if (fdbuf.attributes & FAP_READ)
		{
			stbuf->st_mode |= S_IRUSR;
		}
		if (fdbuf.attributes & FAP_WRITE)
		{
			stbuf->st_mode |= S_IWUSR;
		}
		if (fdbuf.attributes & FAP_EXEC)
		{
			stbuf->st_mode |= S_IXUSR;
		}
		if (fdbuf.attributes & FAP_PREAD)
		{
			stbuf->st_mode |= S_IROTH;
		}
		if (fdbuf.attributes & FAP_PWRITE)
		{
			stbuf->st_mode |= S_IWOTH;
		}
		if (fdbuf.attributes & FAP_PEXEC)
		{
			stbuf->st_mode |= S_IXOTH;
		}

        	stbuf->st_nlink = 1;

		if (_coco_gs_size(p, &filesize) != 0)
		{
			filesize = 0;
		}
		stbuf->st_size = int4(filesize);
                stbuf->st_ctimespec.tv_sec = fdbuf.create_time;
                stbuf->st_mtimespec.tv_sec = fdbuf.last_modified_time;
	}	

	_coco_close(p);
    }

    return 0;
}


static int coco_release(const char *path, struct fuse_file_info *fi)
{
	_coco_close((coco_path_id)fi->fh);

	return 0;
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
	if ((ec = _coco_create(&p, buff, mflags, pflags)) != 0)
	{
		return -CoCoToUnixError(ec);
	}

	fi->fh = (uint64_t)p;

	return 0;
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
	if (_coco_open(&p, buff, mflags) != 0)
	{
		mflags |= FAM_DIR;
		if ((ec = _coco_open(&p, buff, mflags)) != 0)
		{
			return -CoCoToUnixError(ec);
		}
	}

	fi->fh = (uint64_t)p;

	return 0;
}


static int coco_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	error_code ec;
	int _size = size;

	coco_path_id p = (coco_path_id)fi->fh;
	_coco_seek(p, offset, SEEK_SET);
	if ((ec = _coco_read(p, buf, &_size)) != 0)
	{
		return -CoCoToUnixError(ec);
	}

	return size;
}


static int coco_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	error_code ec;
	int _size = size;

	coco_path_id p = (coco_path_id)fi->fh;
	_coco_seek(p, offset, SEEK_SET);
	if ((ec = _coco_write(p, buf, &_size)) != 0)
	{
		return -CoCoToUnixError(ec);
	}

	return size;
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
				filler(buf, OS9StringToCString(e.dentry.os9.name), NULL, 0);
				break;

			case DECB:
				{
					unsigned char cstring[24];

					DECBStringToCString(e.dentry.decb.filename, e.dentry.decb.file_extension, cstring);
					filler(buf, cstring, NULL, 0);
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


static struct fuse_operations coco_filesystem_operations =
{
	.getattr = coco_getattr,
	.open = coco_open,
	.create = coco_create,
	.read = coco_read,
	.write = coco_write,
	.readdir = coco_readdir,
	.release = coco_release,
	.unlink = coco_unlink
};


int main(int argc, char **argv)
{
	dsk = argv[2];
	return fuse_main(argc - 1, argv, &coco_filesystem_operations, NULL);
}
