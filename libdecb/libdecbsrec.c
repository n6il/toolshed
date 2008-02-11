/********************************************************************
 * libdecbsrec.c - S-Record encode and decode routines.
 *
 * $Id$
 ********************************************************************/
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "decbpath.h"

#define BLOCK_QUANTUM 256
#define PREAMBLE 0x00
#define POSTAMBLE 0xff

#if defined(__linux__) || defined(__CYGWIN__) || defined(BDS)
/* implemented based on OSX man page */
static inline int digittoint(int c)
{
    /* if not 0-9, a-f, or A-F then return 0 */
    if (!isxdigit(c))
        return 0;

    if (isdigit(c))
        return c - '0';

    if (isupper(c))
        return c - 'A' + 10;

    /* not 0-9, not A-F, must be a-f */
    return c - 'a' + 10;
}
#endif

/* Input: Binary segmented machine language file
   Output: S-Record text file
*/

error_code _decb_srec_encode(unsigned char *in_buffer, int in_size, char **out_buffer, u_int *out_size)
{
	error_code ec = 0;
	size_t buffer_size;
	int in_buffer_position;
	int length, address, count;
	u_char checksum, type;
	in_buffer_position = 0;
	*out_buffer = malloc( BLOCK_QUANTUM );
	buffer_size = BLOCK_QUANTUM;
	*out_size = 0;

	type = in_buffer[in_buffer_position++];
	
	/* Output S1 records (data) */
	while( (type != POSTAMBLE) && (in_buffer_position < in_size) )
	{
		int i;
		u_char buffer[32];
		
		length = in_buffer[in_buffer_position++] << 8;
		length += in_buffer[in_buffer_position++];
		address = in_buffer[in_buffer_position++] << 8;
		address += in_buffer[in_buffer_position++];

		while( length > 0 )
		{
			count = 0;
			
			if( (ec = _decb_buffer_sprintf( out_size, out_buffer, &buffer_size, "S1")) != 0 )
				return ec;
		
			while( (count < 32) && (length > 0) )
			{
				buffer[count++] = in_buffer[in_buffer_position++];
				length--;
			}
			
			checksum = 0xff;
			checksum -= count+3;
			if( (ec = _decb_buffer_sprintf( out_size, out_buffer, &buffer_size, "%2.2X", count+3)) != 0 )
				return ec;
			
			checksum -= (address >> 8) & 0xff;
			checksum -= (address >> 0) & 0xff;
			if( (ec = _decb_buffer_sprintf( out_size, out_buffer, &buffer_size, "%4.4X", address)) != 0 )
				return ec;
			
			for( i=0; i<count; i++ )
			{
				checksum -= buffer[i];
				if( (ec = _decb_buffer_sprintf( out_size, out_buffer, &buffer_size, "%2.2X", buffer[i])) != 0 )
					return ec;
			}

			if( (ec = _decb_buffer_sprintf( out_size, out_buffer, &buffer_size, "%2.2X\n", checksum)) != 0 )
				return ec;
			
			address += count;
		}
		
		type = in_buffer[in_buffer_position++];
	}
	
	/* Output S9 record (execution address) */
	
	if( type == POSTAMBLE )
	{
		checksum = 0xff;
		count = 0;
		length = in_buffer[in_buffer_position++] << 8;
		length += in_buffer[in_buffer_position++];
		address = in_buffer[in_buffer_position++] << 8;
		address += in_buffer[in_buffer_position++];
	
		checksum -= count + 0x03;
		checksum -= (address >> 8) & 0xff;
		checksum -= (address >> 0) & 0xff;

		if( (ec = _decb_buffer_sprintf( out_size, out_buffer, &buffer_size, "S9%2.2X%4.4X%2.2X\n", count+3, address, checksum)) != 0 )
			return ec;
	}
	
	return ec;
}

/* Input: Binary single record machine language file
   Output: S-Record text file
*/

error_code _decb_srec_encode_sr(unsigned char *in_buffer, int in_size, int start_address, int exec_address, char **out_buffer, u_int *out_size)
{
	error_code ec = 0;
	u_char *newbuffer;
	
	/* Convert single record file into a single segment multiple record file */
	
	newbuffer = malloc( in_size + 10 );
	
	if( newbuffer == NULL )
	{
		fprintf( stderr, "_decb_srec_encode_sr: memory allocation failed\n" );
		return -1;
	}
	
	newbuffer[0] = PREAMBLE;
	newbuffer[1] = (in_size >> 8) & 0xff;
	newbuffer[2] = (in_size >> 0) & 0xff;
	newbuffer[3] = (start_address >> 8) & 0xff;
	newbuffer[4] = (start_address >> 0) & 0xff;
	memcpy( &(newbuffer[5]), in_buffer, in_size );
	newbuffer[in_size+5] = POSTAMBLE;
	newbuffer[in_size+6] = 0;
	newbuffer[in_size+7] = 0;
	newbuffer[in_size+8] = (exec_address >> 8) & 0xff;
	newbuffer[in_size+9] = (exec_address >> 0) & 0xff;
	
	/* Now send it to multiple record encoding */
	ec = _decb_srec_encode(newbuffer, in_size + 10, out_buffer, out_size);

	free( newbuffer );

	return ec;
}

/* Input: S-Record text file
   Output: Segmented binary machine language file
*/

error_code _decb_srec_decode(unsigned char *in_buffer, int in_size, u_char **out_buffer, u_int *out_size)
{
	error_code ec = 0;
	int in_buffer_position, out_buffer_position;
	int type, count, address, checksum;
	int start_address = -1;
	
	/* Preflight S Record to determine output buffer size */
	
	*out_size = 0;
	in_buffer_position = 0;
	
	while( in_buffer_position < in_size )
	{
		/* Skip past new lines and carrage returns */
		while( (in_buffer[in_buffer_position] == '\n') || (in_buffer[in_buffer_position] == '\r') )
		{
			in_buffer_position++;
			
			if( in_buffer_position > in_size )
				break;
		}
	
		if( in_buffer_position > in_size)
			continue;
			
		/* Skip past 'S' at start of line */
		in_buffer_position++;
		
		type = digittoint(in_buffer[in_buffer_position++]);
		
		count = digittoint(in_buffer[in_buffer_position++]);
		count <<= 4;
		count += digittoint(in_buffer[in_buffer_position++]);
		
		if( type == 1 )
			*out_size += 5 + (count - 3);
		else if( type == 9 )
			break;

		in_buffer_position += count*2;
	}

	if( *out_size == 0 )
	{
		*out_buffer = NULL;
		fprintf( stderr, "_decb_srec_decode: zero size binary file.\n" );
		return -1;
	}
	
	*out_size += 5; /* Postamble block */
	
	*out_buffer = malloc( *out_size );
	
	if( *out_buffer == NULL )
	{
		fprintf( stderr, "_decb_srec_decode: memory allocation failed.\n" );
		return -1;
	}
	
	/* Build multiple segement binary file */

	in_buffer_position = 0;
	out_buffer_position = 0;
	
	while( in_buffer_position < in_size )
	{
		/* Skip past new lines and carrage returns */
		while( (in_buffer[in_buffer_position] == '\n') || (in_buffer[in_buffer_position] == '\r') )
		{
			in_buffer_position++;
			
			if( in_buffer_position > in_size )
				break;
		}
	
		if( in_buffer_position > in_size)
			continue;
			
		/* Skip past 'S' at start of line */
		in_buffer_position++;
		
		type = digittoint(in_buffer[in_buffer_position++]);
		
		count = digittoint(in_buffer[in_buffer_position++]);
		count <<= 4;
		count += digittoint(in_buffer[in_buffer_position++]);

		address = digittoint(in_buffer[in_buffer_position++]);
		address <<= 4;
		address += digittoint(in_buffer[in_buffer_position++]);
		address <<= 4;
		address += digittoint(in_buffer[in_buffer_position++]);
		address <<= 4;
		address += digittoint(in_buffer[in_buffer_position++]);
		
		/* Record first address, in case there is no S9 record */
		if( start_address == -1 )
			start_address = address;
			
		if( type == 1 )
		{
			int i;
			
			(*out_buffer)[out_buffer_position++] = PREAMBLE;
			(*out_buffer)[out_buffer_position++] = ((count-3) >> 8) & 0xff;
			(*out_buffer)[out_buffer_position++] = ((count-3) >> 0) & 0xff;
			(*out_buffer)[out_buffer_position++] = ((address) >> 8) & 0xff;
			(*out_buffer)[out_buffer_position++] = ((address) >> 0) & 0xff;
			
			for( i=0; i<count-3; i++ )
			{
				u_char byte;
				
				byte = digittoint(in_buffer[in_buffer_position++]);
				byte <<= 4;
				byte += digittoint(in_buffer[in_buffer_position++]);
				
				(*out_buffer)[out_buffer_position++] = byte;
			}

		}
		else if( type == 9 )
		{
			(*out_buffer)[out_buffer_position++] = POSTAMBLE;
			(*out_buffer)[out_buffer_position++] = 0;
			(*out_buffer)[out_buffer_position++] = 0;
			(*out_buffer)[out_buffer_position++] = ((address) >> 8) & 0xff;
			(*out_buffer)[out_buffer_position++] = ((address) >> 0) & 0xff;
			return ec;
		}
		else
			out_buffer_position += count*2;

		checksum = digittoint(in_buffer[in_buffer_position++]);
		checksum <<= 4;
		checksum += digittoint(in_buffer[in_buffer_position++]);
	}

	(*out_buffer)[out_buffer_position++] = POSTAMBLE;
	(*out_buffer)[out_buffer_position++] = 0;
	(*out_buffer)[out_buffer_position++] = 0;
	(*out_buffer)[out_buffer_position++] = ((start_address) >> 8) & 0xff;
	(*out_buffer)[out_buffer_position++] = ((start_address) >> 0) & 0xff;
	
	return ec;
}
