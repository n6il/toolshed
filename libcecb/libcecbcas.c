/********************************************************************
 * libcebccas.c - Cassette BASIC CAS file routines
 *
 * $Id$
 ********************************************************************/

#include "cecbpath.h"

/*
 * _cecb_read_bits_cas()
 *
 * Read 'count' bits and return the result.
 *
 * Count must be from 1 to 8.
 * Returned bit are left aligned in unsigned char.
 */

error_code _cecb_read_bits_cas( cecb_path_id path, int count, unsigned char *result )
{
	error_code ec = 0;
	unsigned char data;
	
	data = 0;
	
	while( count > 0 )
	{
		if( path->cas_current_bit == 0 )
		{
			path->cas_current_byte++;
			if( fread( &(path->cas_byte), 1, 1, path->fd ) != 1 )
			{
				*result = data;
				return EOS_EOF;
			}
			
			path->cas_current_bit = 0x01;
		}
			
		data >>= 1;
		
		if( (path->cas_byte & path->cas_current_bit) == path->cas_current_bit )
			data |= 0x80;
		
		path->cas_current_bit <<= 1;
		count--;
	}
	
	*result = data;
	
	return ec;
}

/*
 * _cecb_parse_cas()
 *
 * Fill in CAS elements
 *
 */

error_code _cecb_parse_cas( cecb_path_id path )
{
	path->tape_type = CAS;
	
	path->cas_start_byte = path->cas_current_byte = path->play_at / 8;
	path->cas_start_bit = path->cas_current_bit = 2 ^ (path->play_at % 8);
	
	fseek( path->fd, path->cas_start_byte, SEEK_SET );
	
	return 0;
}

/*
 * _cecb_write_cas_data()
 *
 * Write CAS data
 *
 */

error_code _cecb_write_cas_data( cecb_path_id path, char *buffer, int total_length)
{
	error_code ec = 0;
	
	
	
	return ec;

}

