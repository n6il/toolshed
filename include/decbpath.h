/********************************************************************
 * decbpath.h - Disk BASIC path definitions header file
 *
 * $Id$
 ********************************************************************/
#ifndef	_DECBPATH_H
#define	_DECBPATH_H

#include <stdio.h>
#include <sys/stat.h>
#include <cocotypes.h>
#include <cococonv.h>
#include <dirent.h>


typedef struct _decb_dir_entry
{
	u_char	filename[8];
		/* Left Justified, zero filled
		   If first byte is 00 then that means the directory entry is free
		   If first byte is FF then this entry and all that remain are unused
		*/
	u_char	file_extension[3];
		/* Left justified, zero filled */
	u_char	file_type;
		/* 00 - BASIC
		   01 - BASIC Data
		   02 - Machine Language program
		   03 - Text Editor Source
		*/
	u_char	ascii_flag;
		/* 00 - Binary file
		   FF - ASCII file
		*/
	u_char	first_granule;
		/* Number of first granule in file */
	u_char	last_sector_size[2];
		/* Number of bytes used in last sector of last granule */
	u_char	file_status;
		/* For Color Tdecb (by Robert Kilgus)
		   Bit 0 - On allows reads
		   Bit 1 - On allows writes
		   Bit 2 - On allows file creation
		   Bit 3 - On allows file extension beyond EOF
		   Bit 4 - On means temporary file - delet file when closed
		   Bit 5 - On prevents rewrite of FAT every time a sector is added to the file
		   Bit 6 - On means I/O buffer is shared. Each logical I/O requires a physical I/O
		   Bit 7 - Reserved (Like release space when file is shortened)
		   (All bits off - file closed)
		*/
	u_char	logical_record_size[2];
		/* 0000 - Variable length with record terminated by the delimiter stored below
		   FFFF - Variable length with first two bytes of record containing size of the rest of the record
		   All other values mean fixed length of specified size
		*/
	u_char	record_terminator;
		/* Variable length record terminator */
	u_char	unused[12];
		/* Unused by Color Tdecb */
} decb_dir_entry;
	

typedef struct _decb_path_id
{
	int				mode;			/* access mode */
	char			imgfile[512];	/* pointer to image file */
	char			filename[64];	/* pointer to filename */
	char			extension[64];	/* Pointer to extension */
	int				drive;			/* Drive Number */
	decb_dir_entry  dir_entry;
	unsigned int	this_directory_entry_index;
	unsigned int	directory_entry_index;
	u_char			FAT[256];
	unsigned int	filepos;		/* file position */
	FILE			*fd;			/* file path pointer */
	int				israw;			/* No file I/O possible, just get/set sector and granule */
	int				disk_offset;	/* Offset HDB-DOS */
} *decb_path_id;


/* File descriptor sector */
/* Disk BASIC doesn't have a file descriptor per se, but we use this structure as one. */

typedef struct
{
	u_char	file_type;		/* 0 = BASIC program, 1 = BASIC data file,
	                           2 = M/L program, 3 = Text editor source file */
	u_char	data_type;		/* 0 = Binary, 1 = ASCII */
	int		file_size;		/* file size */
} decb_file_stat, *Decb_file_stat;



/* Disk BASIC Prototypes */

error_code _decb_open(decb_path_id *, char *, int);
error_code _decb_close(decb_path_id);
error_code _decb_create(decb_path_id *path, char *pathlist, int mode, int file_type, int data_type);
error_code _decb_read(decb_path_id path, void *buffer, int *size);
error_code _decb_readln(decb_path_id path, void *buffer, int *size);
error_code _decb_write(decb_path_id path, void *buffer, int *size);
error_code _decb_kill(char *filename);
error_code _decb_seek(decb_path_id, int, int);
error_code _decb_readdir(decb_path_id path, decb_dir_entry *de);
error_code _decb_writedir(decb_path_id path, decb_dir_entry *de);
error_code _decb_seekdir(decb_path_id path, int entry);
error_code _decb_seeksector(decb_path_id path, int track, int sector);
error_code _decb_seekgranule(decb_path_id path, int granule);
error_code _decb_rename(char *pathlist, char *newname);
error_code _decb_gs_size(decb_path_id path, int *size);
error_code _decb_ss_size(decb_path_id path, int size);
error_code _decb_gs_eof(decb_path_id path);
error_code _decb_gs_fd(decb_path_id path, decb_file_stat *stat);
error_code _decb_ss_fd(decb_path_id path, decb_file_stat *stat);
error_code _decb_gs_sector(decb_path_id path, int track, int sector, char *buffer);
error_code _decb_ss_sector(decb_path_id path, int track, int sector, char *buffer);
error_code _decb_gs_granule(decb_path_id path, int granule, char *buffer);
error_code _decb_ss_granule(decb_path_id path, int granule, char *buffer);
error_code _decb_detoken(unsigned char *in_buffer, int in_size, char **out_buffer, int *out_size);
error_code _decb_entoken(unsigned char *in_buffer, int in_size, unsigned char **out_buffer, int *out_size);
error_code _decb_detect_tokenized( unsigned char *in_buffer, int in_size );

/* ERROR CODES */
#define EOS_OM		256		/* Out of memory error */
#define EOS_SN		257		/* Syntax error */

#include <cocopath.h>

#endif	/* _DECBPATH_H */

