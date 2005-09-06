/********************************************************************
 * os9free.c - Disk utilization utility for OS-9
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cocotypes.h>
#include <cocopath.h>
#include <queue.h>


static int do_free(char **argv, char *p);

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
		if (argv[i][0] == '-')
		{
			continue;
		}
		else
		{
			p = argv[i];
		}

		ec = do_free(argv, p);

		if (ec != 0)
		{
			fprintf(stderr, "%s: error %d opening '%s'\n", argv[0], ec, p);
			return(ec);
		}
	}

	if (argv[1] == NULL)
	{
		show_help(helpMessage);
		return(0);
	}

	return(0);
}


static int do_free(char **argv, char *p)
{
	error_code	ec = 0;
	int i;
	char os9pathlist[256];
	os9_path_id path;
	int bytes_in_bitmap, sectors_in_bitmap;
	lsn0_sect sector0;
	int size;
	unsigned int bytes_free = 0, free_sectors = 0;
	unsigned int largest_free_block = 0, sectors_per_cluster = 0;
	unsigned int total_sectors = 0;
	unsigned int largest_count = 0;
	unsigned int sector_count = 0;
	unsigned int postfixDivisor = 1;
	char postfix[4];

	strcpy(os9pathlist, p);

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
		fprintf(stderr, "%s: error %d opening '%s'\n",
			argv[0], ec, os9pathlist);
		return(ec);
	}

	/* seek to the beginning of the disk */
	_os9_seek(path, 0, SEEK_SET);

	/* read LSN0 */
	size = sizeof(sector0);
	_os9_read(path, &sector0, &size);

	/* get the number of bytes in the bitmap and compute bitmap sectors */
	bytes_in_bitmap = int2(sector0.dd_map);
	sectors_in_bitmap = bytes_in_bitmap / path->bps +
		(bytes_in_bitmap % path->bps != 0);
	sectors_per_cluster = int2(sector0.dd_bit);
	total_sectors = int3(sector0.dd_tot);

	/* walk bitmap for 'bytes_in_bitmap * 8' times */
	for (i = 0; i < bytes_in_bitmap * 8; i++)
	{
		sector_count++;

		if (_os9_ckbit(path->bitmap, i))
		{
			/* bit is set, sector not free */
			if (largest_count > largest_free_block)
			{
				largest_free_block = largest_count;
			}
			largest_count = 0;
		}
		else
		{
			/* bit is clear, sector is free */
			largest_count++;
			free_sectors++;
			bytes_free += path->bps * sectors_per_cluster;
		}
	}

	/* one last check on largest free block (here if last sector is free) */
	if (largest_count > largest_free_block)
	{
		largest_free_block = largest_count;
	}

	p = strdup((char *)sector0.dd_nam);
	printf("\n\"%s\" created on %02d/%02d/%04d\n",
		OS9NameToString((u_char *)p), sector0.dd_dat[1],
		sector0.dd_dat[1], sector0.dd_dat[0] + 1900);
	if ((total_sectors * path->bps) < (1024 * 1024))
	{
		postfixDivisor = 1024;
		strcpy(postfix, "KB");
	}
	else
	if ((total_sectors * path->bps) < (1024 * 1024 * 1024))
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
		(total_sectors * path->bps) / postfixDivisor, postfix,
		total_sectors, sectors_per_cluster);
	printf("%d Free sectors, largest block %d sectors\n",
		free_sectors, largest_free_block);

	bytes_free = free_sectors * path->bps * sectors_per_cluster;

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

	_os9_close(path);

	return(0);
}
