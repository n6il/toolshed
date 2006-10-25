/********************************************************************
 * os9free.c - Disk utilization utility for OS-9
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#ifndef BDS
#include <unistd.h>
#endif
#include <cocotypes.h>
#include <cocopath.h>
#include <queue.h>


static error_code RBFFree(char *file, char *dname, u_int *month, u_int *day, u_int *year, u_int *bps, u_int *total_sectors, u_int *bytes_free, u_int *free_sectors, u_int *largest_free_block, u_int *sectors_per_cluster, u_int *largest_count, u_int *sector_count);

/* Help message */
static char *helpMessage[] =
{
	"Syntax: free {[<opts>]} {<disk> [<...>]} {[<opts>]}\n",
	"Usage:  Displays the amount of free space on an image.\n",
	"Options:\n",
	NULL
};


int os9free(int argc, char *argv[])
{
	error_code	ec = 0;
	char		*p = NULL;
	int		i;

	/* walk command line for options */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (p = &argv[i][1]; *p != '\0'; p++)
			{
				switch(*p)
				{
					case '?':
					case 'h':
						show_help(helpMessage);
						return(0);
	
					default:
						fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
						return(0);
				}
			}
		}
	}

	/* walk command line for pathnames */
	for (i = 1; i < argc; i++)
	{
		char dname[64];
		u_int month, day, year;
		u_int bps, total_sectors, bytes_free, free_sectors, largest_free_block, sectors_per_cluster;
		u_int largest_count, sector_count;
		u_int postfixDivisor = 1;
		char postfix[4];
		
		if (argv[i][0] == '-')
		{
			continue;
		}
		else
		{
			p = argv[i];
		}

		ec = RBFFree(p, dname, &month, &day, &year, &bps, &total_sectors, &bytes_free, &free_sectors, &largest_free_block, &sectors_per_cluster, &largest_count, &sector_count);
		if (ec != 0)
		{
			fprintf(stderr, "%s: error %d opening '%s'\n", argv[0], ec, p);
			return(ec);
		}
		printf("\n\"%s\" created on %02d/%02d/%04d\n",
			dname, month, day, year);
		
		if ((total_sectors * bps) < (1024 * 1024))
		{
			postfixDivisor = 1024;
			strcpy(postfix, "KB");
		}
		else
		if ((total_sectors * bps) < (1024 * 1024 * 1024))
		{
			postfixDivisor = (1024 * 1024);
			strcpy(postfix, "MB");
		}
		else
		{
			postfixDivisor = (1024 * 1024 * 1024);
			strcpy(postfix, "GB");
		}

		printf("Capacity: %d%s (%d sectors) %d-sector clusters\n", 
			(total_sectors * bps) / postfixDivisor, postfix,
			total_sectors, sectors_per_cluster);
		printf("%d Free sectors, largest block %d sectors\n",
			free_sectors, largest_free_block);

		if (bytes_free < (1024 * 1024))
		{
			postfixDivisor = 1024;
			strcpy(postfix, "KB");
		}
		else
		if (bytes_free < (1024 * 1024 * 1024))
		{
			postfixDivisor = (1024 * 1024);
			strcpy(postfix, "MB");
		}
		else
		{
			postfixDivisor = (1024 * 1024 * 1024);
			strcpy(postfix, "GB");
		}

		printf("Free space: %d%s (%d bytes)\n", bytes_free / postfixDivisor,
			postfix, bytes_free);

		printf("\n");
	}

	if (argv[1] == NULL)
	{
		show_help(helpMessage);
		return(0);
	}

	return(0);
}


static error_code RBFFree(char *file, char *dname, u_int *month, u_int *day, u_int *year, u_int *bps, u_int *total_sectors, u_int *bytes_free, u_int *free_sectors, u_int *largest_free_block, u_int *sectors_per_cluster, u_int *largest_count, u_int *sector_count)
{
	error_code	ec = 0;
	int i;
	char os9pathlist[256];
	os9_path_id path;
	int bytes_in_bitmap, sectors_in_bitmap;
	lsn0_sect sector0;
	u_int size;
	
	*bytes_free = 0;
	*free_sectors = 0;
	*largest_free_block = 0;
	*sectors_per_cluster = 0;
	*total_sectors = 0;
	*largest_count = 0;
	*sector_count = 0;

	strcpy(os9pathlist, file);

	/* if the user forgot to add the ',', do it for them */
	if (strchr(os9pathlist, ',') == NULL)
	{
		strcat(os9pathlist, ",.");
	}

	strcat(os9pathlist, "@");

	/* open a path to the device */
	ec = _os9_open(&path, os9pathlist, FAM_READ);
	if (ec != 0)
	{
		return(ec);
	}

	*bps = path->bps;
	
	/* seek to the beginning of the disk */
	_os9_seek(path, 0, SEEK_SET);

	/* read LSN0 */
	size = sizeof(sector0);
	_os9_read(path, &sector0, &size);

	/* get the number of bytes in the bitmap and compute bitmap sectors */
	bytes_in_bitmap = int2(sector0.dd_map);
	sectors_in_bitmap = bytes_in_bitmap / *bps +
		(bytes_in_bitmap % *bps != 0);
	*sectors_per_cluster = int2(sector0.dd_bit);
	*total_sectors = int3(sector0.dd_tot);

	/* walk bitmap for 'bytes_in_bitmap * 8' times */
	for (i = 0; i < bytes_in_bitmap * 8; i++)
	{
		sector_count++;

		if (_os9_ckbit(path->bitmap, i))
		{
			/* bit is set, sector not free */
			if (*largest_count > *largest_free_block)
			{
				*largest_free_block = *largest_count;
			}
			*largest_count = 0;
		}
		else
		{
			/* bit is clear, sector is free */
			(*largest_count)++;
			(*free_sectors)++;
			*bytes_free += *bps * *sectors_per_cluster;
		}
	}

	/* one last check on largest free block (here if last sector is free) */
	if (largest_count > largest_free_block)
	{
		largest_free_block = largest_count;
	}

	strcpy(dname, (char *)sector0.dd_nam);

	*bytes_free = *free_sectors * *bps * *sectors_per_cluster;

	*month = sector0.dd_dat[1];
	*day = sector0.dd_dat[2];
	*year = sector0.dd_dat[0] + 1900;

	OS9StringToCString((u_char *)dname);
	
	_os9_close(path);

	return(0);
}
