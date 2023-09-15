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

#include <toolshed.h>

/* #define DEBUG */

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
# ifdef DEBUG
# include <syslog.h>
# endif
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

#include <fuse.h>

static int coco_open(const char *path, struct fuse_file_info *fi);

/* DSK image filename pointer */
static char dsk[1024];



/*
 * coco_statfs - returns status of the file system
 */
static int coco_statfs(const char *path, struct statvfs *stbuf)
{
	_path_type type;
	char buff[2048], dname[32];
	u_int month, day, year, bps, total_sectors, bytes_free, free_sectors;
	u_int largest_free_block, sectors_per_cluster, largest_count, sector_count;
	
	snprintf(buff, 2047, "%s,%s", dsk, path);
	_coco_identify_image(buff, &type);
	/* Here we revert to RBF or Disk BASIC to get details about the disk */
	switch (type)
	{
		case OS9:
			if (TSRBFFree(buff, dname, &month, &day, &year, &bps, &total_sectors, &bytes_free, &free_sectors, &largest_free_block, &sectors_per_cluster, &largest_count, &sector_count) == 0)
			{
				stbuf->f_bsize = stbuf->f_frsize = bps;
				stbuf->f_blocks = total_sectors;
				stbuf->f_bfree = free_sectors;
				stbuf->f_bavail = free_sectors;
				stbuf->f_files = 1000;
				stbuf->f_ffree = 1000;
				stbuf->f_favail = 1000;
				stbuf->f_fsid = 6809;
				stbuf->f_namemax = 29;
			}
			break;
			
		case DECB:
			{
				/* TODO: Put values here that make sense */
				stbuf->f_bsize = stbuf->f_frsize = 256;
				stbuf->f_blocks = 68;
				stbuf->f_bfree = 68;
				stbuf->f_bavail = 68;
				stbuf->f_files = 1000;
				stbuf->f_ffree = 1000;
				stbuf->f_favail = 1000;
				stbuf->f_fsid = 6809;
				stbuf->f_namemax = 11;
			}
			break;

               default:
                        break;
	}
	
#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_statfs(%s) = %d", path, type);
# else
	syslog(LOG_DEBUG,"coco_statfs(%s) = %d", path, type);
# endif
#endif
	return 0;
}

#if 0
/*
 * coco_fgetattr - returns file attributes
 *
 * Notes: code in this routine coverts coco_file_stat values into
 * values appropriate for the struct stat native to FUSE.
 */
static int coco_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	error_code ec = 0;
	coco_path_id p = (coco_path_id)(int32_t)fi->fh;
	
        memset(stbuf, 0, sizeof(struct stat));

	coco_file_stat fdbuf;

	/* Disk BASIC check -- strip off S_IFDIR from mode */
	if (p->type == DECB)
	{
		stbuf->st_mode &= ~S_IFDIR;
		stbuf->st_mode = S_IFREG;
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
		stbuf->st_size = int4((u_char *)filesize);
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

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_fgetattr(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_fgetattr(%s) = %d", path, ec);
# endif
#endif

    return ec;
}
#endif

/*
 * coco_getattr - returns file attributes
 *
 * Notes: code in this routine coverts coco_file_stat values into
 * values appropriate for the struct stat native to FUSE.
 */
static int coco_getattr(const char *path, struct stat *stbuf)
{
	error_code ec = 0;
	coco_file_stat fdbuf;
	char buff[2048];
	
    memset(stbuf, 0, sizeof(struct stat));
	snprintf(buff, 2047, "%s,%s", dsk, path);
	if ((ec = -CoCoToUnixError(_coco_gs_fd_pathlist(buff, &fdbuf))) == 0)
	{
		u_int filesize;

		stbuf->st_mode |= CoCoToUnixPerms(fdbuf.attributes);

       	stbuf->st_nlink = 1;

		if (_coco_gs_size_pathlist(buff, &filesize) == 0)
		{
			stbuf->st_size = filesize;
		}

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

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_getattr(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_getattr(%s) = %d", path, ec);
# endif
#endif

    return ec;
}


/*
 * coco_mkdir - make a directory (OS-9 only)
 */	
static int coco_mkdir(const char *path, mode_t mode)
{
	error_code ec;
	char buff[2048];

	snprintf(buff, 2047, "%s,%s", dsk, path);
	ec = -CoCoToUnixError(_coco_makdir(buff));

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_makdir(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_makdir(%s) = %d", path, ec);
# endif
#endif

	return ec;
}


/*
 * coco_unlink - removes the file specified in the path
 */
static int coco_unlink(const char *path)
{
	error_code ec;
	char buff[2048];

	snprintf(buff, 2047, "%s,%s", dsk, path);
	ec = -CoCoToUnixError(_coco_delete(buff));

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_unlink(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_unlink(%s) = %d", path, ec);
# endif
#endif

	return ec;
}


/*
 * coco_rmdir - removes the directory specified in the path
 */
static int coco_rmdir(const char *path)
{
	error_code ec = 0;
	char buff[2048];

	snprintf(buff, 2047, "%s,%s", dsk, path);
//	ec = -CoCoToUnixError(_coco_deldir(buff)); //, CoCoToUnixPerm(mode));
#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_rmdir(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_rmdir(%s) = %d", path, ec);
# endif
#endif
	
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
#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_rename(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_rename(%s) = %d", path, ec);
# endif
#endif

	return ec;
}


/*
 * coco_chmod - changes attributes of a file
 */
static int coco_chmod(const char *path, mode_t mode)
{
	error_code ec;
	char buff[2048];
	coco_path_id p;

	snprintf(buff, 2047, "%s,%s", dsk, path);
	if ((ec = -CoCoToUnixError(_coco_open(&p, buff, FAM_WRITE))) == 0)
	{
		ec = -CoCoToUnixError(_coco_ss_attr(p, UnixToCoCoPerms(mode)));
		_coco_close(p);
	}
	
#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_chmod(%s, $%X) = %d", path, mode, ec);
# else
	syslog(LOG_DEBUG,"coco_chmod(%s, $%X) = %d", path, mode, ec);
# endif
#endif

	return ec;
}


/*
 * coco_truncate - truncates a file to a specific size
 */
static int coco_truncate(const char *path, off_t size)
{
	error_code ec = 0;
	char buff[2048];
	coco_path_id p;

	snprintf(buff, 2047, "%s,%s", dsk, path);
	ec = -CoCoToUnixError(_coco_open(&p, buff, FAM_WRITE));
	if (ec == 0)
	{
		ec = -CoCoToUnixError(_coco_ss_size(p, size));
		_coco_close(p);
	}
	
#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_truncate(%s, %d) = %d", path, size, ec);
# else
	syslog(LOG_DEBUG,"coco_truncate(%s, %ld) = %d", path, size, ec);
# endif
#endif

	return ec;
}


static int coco_open(const char *path, struct fuse_file_info *fi)
{
	error_code ec;
	coco_path_id p;
	char buff[2048];
	int mflags = FAM_READ;

	snprintf(buff, 2047, "%s,%s", dsk, path);

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
	{
		mflags |= FAM_WRITE;
	}
	if ((ec =  -CoCoToUnixError(_coco_open(&p, buff, mflags))) == 0)
	{
		fi->fh = (uint64_t)p;
	}

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_open(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_open(%s) = %d", path, ec);
# endif
#endif

	return ec;
}


static int coco_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	error_code ec;
	uint32_t _size = size;

	coco_path_id p = (coco_path_id)fi->fh;
	_coco_seek(p, offset, SEEK_SET);
	if ((ec = -CoCoToUnixError(_coco_read(p, buf, &_size))) != 0)
	{
		return ec;
	}

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_read(%s, $%X, %d) = %d", path, buf, size, ec);
# else
	syslog(LOG_DEBUG,"coco_read(%s, %p, %ld) = %d", path, buf, size, ec);
# endif
#endif

	return size;
}


static int coco_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	error_code ec;
	uint32_t _size = size;

	coco_path_id p = (coco_path_id)fi->fh;
	_coco_seek(p, offset, SEEK_SET);
	if ((ec = -CoCoToUnixError(_coco_write(p, (char *)buf, &_size))) != 0)
	{
		return ec;
	}

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_write(%s, $%X, %d) = %d", path, buf, size, ec);
# else
	syslog(LOG_DEBUG,"coco_write(%s, %p, %ld) = %d", path, buf, size, ec);
# endif
#endif

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
	
	ec = -CoCoToUnixError(_coco_close((coco_path_id)fi->fh));
	
#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_release(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_release(%s) = %d", path, ec);
# endif
#endif

	return ec;
}


static int coco_create(const char *path, mode_t perms, struct fuse_file_info * fi)
{
	error_code ec = 0;
	coco_path_id p;
	char buff[2048];
	coco_file_stat fstat;
	
	int mflags = FAM_READ | FAM_WRITE;
	fstat.perms = FAP_READ | FAP_WRITE;
	
	snprintf(buff, 2047, "%s,%s", dsk, path);

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
	{
		fstat.perms |= FAM_WRITE;
	}

	if ((ec = -CoCoToUnixError(_coco_create(&p, buff, mflags, &fstat))) != 0)
	{
		return ec;
	}

	fi->fh = (uint64_t)p;

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_create(%s, $%X) = %d", path, perms, ec);
# else
	syslog(LOG_DEBUG,"coco_create(%s, $%X) = %d", path, perms, ec);
# endif
#endif

	return ec;
}


static int coco_opendir(const char *path, struct fuse_file_info *fi)
{
	error_code ec;
	coco_path_id p;
	char buff[2048];
	int mflags = FAM_READ;

	snprintf(buff, 2047, "%s,%s", dsk, path);

	mflags |= FAM_DIR;

	if ((fi->flags & O_ACCMODE) != O_RDONLY)
	{
		mflags |= FAM_WRITE;
	}
	if ((ec =  -CoCoToUnixError(_coco_open(&p, buff, mflags))) == 0)
	{
		fi->fh = (uint64_t)p;
	}

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_opendir(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_opendir(%s) = %d", path, ec);
# endif
#endif

	return ec;
}


static int coco_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	error_code ec = 0;
	coco_path_id p;
	coco_dir_entry e;
	char buff[2048];

#if !0
	snprintf(buff, 2047, "%s,%s", dsk, path);
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
				if (e.dentry.decb.filename[0] != 0 && e.dentry.decb.filename[0] != 255 )
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

#ifdef DEBUG
# if defined(__APPLE__)
	NSLog(@"coco_readdir(%s) = %d", path, ec);
# else
	syslog(LOG_DEBUG,"coco_readdir(%s) = %d", path, ec);
# endif
#endif

	return ec;
}


static int coco_utimens(const char *path, const struct timespec *tv)
{
	return 0;
}

#ifndef COCOFUSE_MAC
static struct fuse_operations coco_filesystem_operations =
{
        .statfs = coco_statfs,
        .truncate = coco_truncate,
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
 	.utimens = coco_utimens
};

void usage(char* name)
{
	printf("cocofuse from Toolshed " TOOLSHED_VERSION "\n");
	printf("Usage: %s: dskimage mountpoint [FUSE options]\n", name);
	exit(1);
}


int make_absolute( const char *path )
{
        if(path[0] == '/') 
        {
                /* absolute path - use as-is */
                strcpy(dsk, path);
        }
        else 
        {
                /* relative path */
                if (getcwd(dsk, 1024)==NULL) return -1;
                /* Allow one for terminating null and 1 for separator
                   slash */
                if((1024 - strlen(dsk)) < (strlen(path)+2)) return -1;
                strcat(dsk, "/");
                strcat(dsk, path);
        }
        return 0;
}

int main(int argc, char **argv)
{
	if(argc < 3)
		usage(argv[0]);

        int rc;
        if(make_absolute(argv[1])<0)
        {
                fprintf(stderr, "Disk image path too long\n");
                rc = 1;
        }
        else 
        {
#ifdef DEBUG
                openlog("cocofuse", LOG_PID, LOG_DAEMON);
#endif        
                argv[1] = argv[0];
                rc = fuse_main(argc - 1, &argv[1], &coco_filesystem_operations, NULL);
#ifdef DEBUG
                closelog();
#endif
        }
        return rc;
}
#endif  /* COCOFUSE_MAC */
