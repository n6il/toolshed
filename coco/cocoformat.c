/********************************************************************
 * os9format.c - OS-9 disk format utility
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <cocopath.h>
#include <cocotypes.h>
#include <cocosys.h>


#define BUFFSIZ	256

static int do_format(char **argv, char *vdisk, int os968k, int quiet, int tracks, int sectorsPerTrack, int heads, int sectorSize, int clusterSize, char *diskName, int sectorAllocationSize, int tpi, int density, int bytesPerSector, int formatEntire);

/* Help message */
static char *helpMessage[] =
{
	"Syntax: format {[<opts>]} <disk> {[<...>]} {[<opts>]}\n",
	"Usage:  Create a disk image of a given size.\n",
	"Options:\n",
	"     -bsX = bytes per sector (default = 256)\n",
	"     -cX  = cluster size\n",
	"     -e   = format entire disk (make full sized image)\n",
	"     -k   = make OS-9/68K LSN0\n",
	"     -nX  = disk name\n",
	"     -q   = quiet; do not report format summary\n",
	" Floppy Options:\n",
	"     -4   = 48 tpi (default)\n",
	"     -9   = 96 tpi\n",
	"     -sa  = sector allocation size (SAS)\n",
	"     -sd  = single density\n",
	"     -dd  = double density (default)\n",
	"     -ss  = single sided (default)\n",
	"     -ds  = double sided\n",
	"     -tX  = tracks (default = 35)\n",
	"     -stX = sectors per track (default = 18)\n",
	" Hard Drive Options:\n",
	"     -lX  = number of logical sectors\n",
	NULL
};


int os9format(int argc, char **argv)
{
	error_code	ec = 0;
	char *p = NULL;
	int i;
	int tracks = 35;
	int heads = 1;
	int sectorsPerTrack = 18;
	int bytesPerSector = 256;
	int tpi = 48;		/* 48 tpi */
	int density = 1;	/* double density */
	int logicalSectors = 0;
	char *diskName = "CoCo Disk";
	int quiet = 0;		/* assume chatter */
	int clusterSize = 0;	/* assume no cluster size */
	int sectorAllocationSize = 8;	/* default */
	int os968k = 0;		/* assume OS-9/6809 LSN0 */
	int formatEntire = 0;	/* format entire disk image */

	/* if no arguments, show help and return */
	if (argv[1] == NULL)
	{
		show_help(helpMessage);
		return(0);
	}

	/* walk command line for options */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (p = &argv[i][1]; *p != '\0'; p++)
			{
				switch(*p)
				{
					case '4':	/* 48 tpi */
						tpi = 48;
						break;

					case '9':	/* 96 tpi */
						tpi = 96;
						break;

					case 'c':
						/* TODO: verify clusterSize is a power of 2! */
						clusterSize = atoi(p+1);
						while (*(p + 1) != '\0') p++;
						break;

					case 'e':
						formatEntire = 1;
						break;

					case 'k':
						os968k = 1;
						break;

					case 'q':
						quiet = 1;
						break;

					case 'b':	/* bytes/sector */
						switch (*(p+1))
						{
							case 's':
								bytesPerSector = atoi(p+2);
								if (bytesPerSector % 256 != 0)
								{
									fprintf(stderr, "%s: bytes per sector must be a multiple of 256\n", argv[0]);
									return(0);
								}
								break;

							default:
								fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
								return(0);
						}
						while (*(p + 1) != '\0') p++;
						break;

					case 'd':	/* double density or sides */
						switch (*(p+1))
						{
							case 'd':
								density = 1;
								break;

							case 's':
								heads = 2;
								break;

							default:
								fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
								return(0);
						}
						while (*(p + 1) != '\0') p++;
						break;

					case 's':	/* single density or sides */
						switch (*(p+1))
						{
							case 'a':
								sectorAllocationSize = atoi(p+2);
								while (*(p + 1) != '\0') p++;
								break;

							case 'd':
								density = 0;
								break;

							case 's':
								heads = 1;
								break;

							case 't':
								sectorsPerTrack = atoi(p+2);
								break;

							default:
								fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
								return(0);
						}
						while (*(p + 1) != '\0') p++;
						break;

					case 't':	/* tracks */
						tracks = atoi(p+1);
						while (*(p + 1) != '\0') p++;
						break;

					case 'n':	/* disk name */
						diskName = p + 1;
						while (*(p + 1) != '\0') p++;
						break;

					case 'l':	/* logical sectors */
						logicalSectors = atoi(p+1);
						while (*(p + 1) != '\0') p++;
						break;

					case '?':
						show_help(helpMessage);
						return(0);

					default:
						fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
						return(0);
				}
			}
		}
	}

	/* if logicalSectors != 0, then format as a hard drive image */
	if (logicalSectors != 0)
	{
		density = 0;	/* density is irrelavent */
		tpi = 0;	/* tracks per inch is irrelavent */
		tracks = 1;
		heads = 1;
		sectorsPerTrack = 1;

		/* compute c/h/s based on sectors desired */
		for (i = 512; i < 65536; i++)
		{
			if (logicalSectors % i == 0)
			{
				tracks = logicalSectors / i;
				logicalSectors = i;
				i = 65536;
			}
		}

		for (i = 255; i > 0; i--)
		{
			if (logicalSectors % i == 0)
			{
				sectorsPerTrack = logicalSectors / i;
				heads = i;
				i = 0;
			}
		}	
	}

	/* walk command line for pathnames */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			continue;
		}
		else
		{
			do_format(argv, argv[i], os968k, quiet, tracks, sectorsPerTrack, heads, bytesPerSector, clusterSize, diskName, sectorAllocationSize, tpi, density, bytesPerSector, formatEntire);
		}
	}

	return(ec);
}



static int do_format(char **argv, char *vdisk, int os968k, int quiet, int tracks, int sectorsPerTrack, int heads, int sectorSize, int clusterSize, char *diskName, int sectorAllocationSize, int tpi, int density, int bytesPerSector, int formatEntire)
{
	error_code	ec = 0;
	os9_path_id path;
	lsn0_sect s0;
	unsigned int totalSectors, totalBytes, sectorsLeft;
	int i, b;
	unsigned int sectorsToAlloc = 0, bitmapSectors = 0;
	unsigned int rootSects;

	/* open a path to the virtual disk */
	ec = _os9_open(&path, vdisk, FAM_WRITE);
	if (ec != 0)
	{
		ec = _os9_create(&path, vdisk, FAM_WRITE, FAP_READ | FAP_WRITE);

		if (ec != 0)
		{
			fprintf(stderr, "%s: cannot open virtual disk\n", argv[0]);

			return(ec);
		}
	}

	_os9_seek(path, 0, SEEK_SET);

	/**** Build LSN0 *****/
	memset(&s0, 0, sizeof(s0));

	totalSectors = tracks * sectorsPerTrack * heads;
	_int3(totalSectors, s0.dd_tot);

	totalBytes = totalSectors * bytesPerSector;
	
	/* Determine appropriate cluster size */
	if (clusterSize == 0)
	{
		clusterSize = 1;

		b = ((totalSectors - 1) / (65535 * 8));
		for (i = 0; i < b; i++)
		{
			clusterSize *= 2;
		}
	}

#define	MAX_SECTORS 0xFFFFFF

	if (int3(s0.dd_tot) > MAX_SECTORS)
	{
		fprintf(stderr, "Disk too large\n");

		return(1);
	}
	else
	if (int3(s0.dd_tot) < 4)
	{
		fprintf(stderr, "Too few sectors\n");

		return(1);
	}

	sectorsLeft = int3(s0.dd_tot);

	_int1(sectorsPerTrack, s0.dd_tks);
	_int2((int3(s0.dd_tot) / (8 * clusterSize)), s0.dd_map);
	if (int3(s0.dd_tot) == 0) _int3(1, s0.dd_tot);
	_int2(clusterSize, s0.dd_bit);

	/* Compute starting location of root directory */
	_int3((int2(s0.dd_map) + sectorSize) / sectorSize + 
		(int2(s0.dd_map) % sectorSize != 0), s0.dd_dir);

	_int2(0, s0.dd_own);
	_int1(0xFF, s0.dd_att);
	_int2(0x0180, s0.dd_dsk);	// Disk ID, is this random???

	/* Build DD.FMT byte */
	b = 0;
	switch (tpi)
	{
		case 48:
			b |= 0;
			break;

		case 96:
			b |= 4;
			break;
	}

	switch (heads)
	{
		case 1:
			b |= 0;
			break;

		case 2:
			b |= 1;
			break;
	}

	switch (density)
	{
		case 0:
			b |= 0;
			break;

		case 1:
			b |= 2;
			break;
	}

	_int1(b, s0.dd_fmt); // B2 = tpi, B1 = density, B0 = SS or DS

	_int2(sectorsPerTrack, s0.dd_spt);
	_int2(0, s0.dd_res);
	_int3(0, s0.dd_bt);
	_int2(0, s0.dd_bsz);
	UnixToOS9Time(time(NULL), s0.dd_dat);

	{
		int i;

		for (i = 0; i < strlen(diskName) - 1; i++)
		{
			s0.dd_nam[i] = diskName[i];
		}
		s0.dd_nam[i] = diskName[i] + 128;
	}

	_int1( DT_RBF, s0.pd_dtp );
	_int1( 1, s0.pd_drv );
	_int1( 0, s0.pd_stp );
	_int1( 0x20, s0.pd_typ );
	_int1( 1, s0.pd_dns );
	
	_int2( tracks, s0.pd_cyl );

	_int1( heads, s0.pd_sid );
	_int1( 0, s0.pd_vfy );

	_int2( sectorsPerTrack, s0.pd_sct );
	_int2( sectorsPerTrack, s0.pd_t0s );

	_int1( 3, s0.pd_ilv );
	_int1( sectorAllocationSize, s0.pd_sas );

	if (os968k == 1)
	{
		/* Build LSN0 for OS-9/68K */

		/* put sync bytes */
		_int4(0x4372757A, s0.dd_sync);
		/* put bitmap starting sector number */
		_int4( 1, s0.dd_maplsn);
		/* put sector 0 version ID */
		_int2( 1, s0.dd_versid);
	}

	/* put bytes per sector */
	_int1( bytesPerSector / 256, s0.dd_lsnsize);

	/***** Write LSN0 *****/
	{
		int size = sizeof(s0);
		int size2 = sectorSize - size;

		/* write LSN0 structure */
		_os9_write(path, &s0, &size);

		/* fill in rest of sector with zeros */
		while (size2--)
		{
			int size = 1;
			char nil = '\0';

			_os9_write(path, &nil, &size);
		}
	}

	/***** Write Bitmap Sector(s) *****/
	{
		u_char *bitmap;
		int size;

		bitmapSectors = (int2(s0.dd_map) / sectorSize + (int2(s0.dd_map) % sectorSize != 0));

		if (bitmapSectors == 0) { bitmapSectors++; }

		size = NextHighestMultiple(bitmapSectors * sectorSize, clusterSize);
		bitmap = (u_char *)malloc(size);
		if (bitmap == NULL)
		{
			return(1);
		}

		/* Clear out bitmap sectors (assume all space available) */
		memset(bitmap, 0, size);

		/* Allocate bits after s0.dd_dot sectors to end of bitmap */
		{
			int extraBitStart = int3(s0.dd_tot) / clusterSize;

			_os9_allbit(bitmap, extraBitStart, (size * 8) - extraBitStart);
		}

		/* Allocate LSN0, Bitmap Sectors, Root FD and Root Dir */
		{
			int clusters;

			sectorsToAlloc++;	/* LSN0 */
			sectorsToAlloc += bitmapSectors;	/* Bitmap sectors */

			rootSects = 1;				/* Root FD Sector */
			rootSects += sectorAllocationSize;	/* Root dirent sectors */

			sectorsToAlloc += rootSects;	/* Total sectors so far -- will be a multiple of cluster size */

			/* Round up sectors to allocate to next highest multiple
			 * of cluster size.
			 * This gives the root directory the fullest possible
			 * number of sectors remaining in the cluster.
			 */
			{
				unsigned int sectorsToAllocOld = sectorsToAlloc;

				sectorsToAlloc = NextHighestMultiple(sectorsToAlloc, clusterSize);
				rootSects += sectorsToAlloc - sectorsToAllocOld;
			}

			if ((clusters = sectorsToAlloc / clusterSize) < 1)
			{
				clusters = 1;
			}
			_os9_allbit(bitmap, 0, clusters);

			sectorsLeft -= sectorsToAlloc;
		}

		_os9_write(path, bitmap, &size);

		free(bitmap);
	}

	/* Write Root Directory FD and Root Sectors */
	{
		char *allocedSectors;
		int totalSectors;
		int totalBytes;

		totalSectors = sectorsToAlloc - 1 - bitmapSectors;
		totalBytes = totalSectors * sectorSize;

		allocedSectors = (char *)malloc(totalBytes);
		if (allocedSectors == NULL)
		{
			return(1);
		}

		memset(allocedSectors, 0, totalBytes);

		/* Build FD sector */
		{
			Fd_stats statSector = (Fd_stats)allocedSectors;

			char lastModifiedDate[5], createDate[5];

			UnixToOS9Time(time(NULL), lastModifiedDate);
			UnixToOS9Time(time(NULL), createDate);

			statSector->fd_att = FAP_DIR | FAP_READ | FAP_WRITE | FAP_EXEC | FAP_PREAD | FAP_PWRITE | FAP_PEXEC;
			_int2(0, statSector->fd_own);
			memcpy(statSector->fd_dat, lastModifiedDate, 5);
			statSector->fd_lnk = 1;
			_int4(sizeof(fd_dentry) * 2, statSector->fd_siz);
			memcpy(statSector->fd_creat, lastModifiedDate, 3);

			_int3(int3(s0.dd_dir) + 1, statSector->fd_seg[0].lsn);
			_int2(rootSects-1, statSector->fd_seg[0].num);
		}

		/* Build directory sector */
		{
			Fd_dentry d = (Fd_dentry)&allocedSectors[sectorSize];

			/* Create '..' */
			strcpy(d->name, "..");
			StringToOS9Name(d->name);
			_int3(int3(s0.dd_dir), d->lsn);

			/* Create '.' */
			d++;

			strcpy(d->name, ".");
			StringToOS9Name(d->name);
			_int3(int3(s0.dd_dir), d->lsn);
		}


		_os9_write(path, allocedSectors, &totalBytes);

		free(allocedSectors);
	}

	/* Write Rest of disk as empty sectors */
	if (formatEntire == 1)
	{
		char *oneEmptySector;

		oneEmptySector = (char *)malloc(sectorSize);

		if (oneEmptySector != NULL)
		{
			memset(oneEmptySector, 0, sectorSize);

			while (sectorsLeft--)
			{
				int size = sectorSize;

				_os9_write(path, oneEmptySector, &size);
			}

			free(oneEmptySector);
		}
	}

	_os9_close(path);

	/* Print summary */
	if (!quiet)
	{
		printf("Format Summary\n");
		printf("--------------\n");
		printf("Geometry Data:\n");
		printf("      Cylinders: %d\n", tracks);
		printf("          Heads: %d\n", heads);
		printf("  Sectors/track: %d\n", sectorsPerTrack);
		printf("    Sector size: %d\n", bytesPerSector);
		printf("\nLogical Data:\n");
		printf("  Total sectors: %d\n", totalSectors);
		printf("  Size in bytes: %d\n", totalBytes);
		printf("   Cluster size: %d\n", clusterSize);
	}

	return(ec);
}
