/********************************************************************
 * libcebcread.c - Cassette BASIC read routines
 *
 * $Id$
 ********************************************************************/

#include "cecbpath.h"

error_code _cecb_read(cecb_path_id path, void *buffer, u_int *size)
{
	error_code		ec = 0;

	/* 1. Check the mode. */
	
    if ((path->mode & FAM_READ) == 0)
    {
        return EOS_FNA;
    }


     /* 2. Treat raw path differently. */
	
    if (path->israw == 1)
    {
		return EOS_FNA;
	}
	
	/* fufil request from buffer */
	
	/* buffer empty, get new block */
	
	/* repeat until out of blocks */
	
}

error_code _cecb_read_next_dir_entry( cecb_path_id path, cecb_dir_entry *dir_entry )
{
	error_code ec = 0;
	unsigned char data[256];
	unsigned char block_type, block_length;
	
	while( ec == 0 )
	{
		ec = _cecb_read_next_block( path, &block_type, &block_length, data  );

		if( (ec == EOS_CRC) || (ec == 0) )
		{
			if( (block_type == 0) && (block_length == sizeof(cecb_dir_entry)) )
			{
				memcpy( dir_entry, data, sizeof(cecb_dir_entry) );
				break;
			}
		}
	}
	
	return ec;
}

error_code _cecb_read_next_block( cecb_path_id path, unsigned char *block_type, unsigned char *block_length, unsigned char *data  )
{
	error_code ec = 0;
	unsigned short find_block;
	unsigned char checksum, checksum_ck;
	int i;
	
	find_block = 0;

	while( find_block != 0x3c55 )
	{
		unsigned char newbit;
		
		find_block >>= 1;
		
		ec = _cecb_read_bits( path, 1, &newbit );
		
		if( ec != 0 )
			return ec;
		
		find_block |= (unsigned short)newbit<<8;
		
		//printf( "find_block: %4.4x\n", find_block );
	}
	
	ec = _cecb_read_bits( path, 8, block_type );
	ec = _cecb_read_bits( path, 8, block_length );
	
	checksum = *block_type + *block_length;
	
	for( i=0; i<*block_length; i++ )
	{
		ec = _cecb_read_bits( path, 8, &(data[i]) );
		checksum += data[i];
	}
	
	ec = _cecb_read_bits( path, 8, &checksum_ck );
	
	if( checksum != checksum_ck )
		ec = EOS_CRC;
		
	return ec;
}

error_code _cecb_read_bits( cecb_path_id path, int count, unsigned char *result )
{
	error_code ec;

	if( path->tape_type == CAS )
		ec = _cecb_read_bits_cas( path, count, result );
	else if( path->tape_type == WAV )
		ec = _cecb_read_bits_wav( path, count, result );
	else
		ec = EOS_IA;

	return ec;
}
