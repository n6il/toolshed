/********************************************************************
 * libdecbcinconcat.c - Color BASIC binary concatenation routine.
 *
 * $Id$
 ********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "decbpath.h"

#define PREAMBLE 0x00
#define POSTAMBLE 0xff

enum binconcat_mode { FREESPACE, INBLOCK };

error_code _decb_binconcat(unsigned char *in_buffer, int in_size, unsigned char **out_buffer, int *out_size)
{
	int	*buffer;
	int i, type, in_pos, out_pos, length, address, size, size_pointer;
	enum binconcat_mode mode;
	
	/* Initialize psuedo RAM buffer */
	
	buffer = malloc( sizeof(int) * 65536 );
	
	if( buffer == NULL )
	{
		fprintf( stderr, "_decb_binconcat: Out of memory\n" );
		return -1;
	}
	
	/* Mark all addresses as unused */
	
	for( i=0; i<65535; i++ )
		buffer[i] = -1;

	/* Unpack BIN file into psuedo RAM space */
	in_pos = 0;
	type = in_buffer[in_pos++];
	
	while( (type != POSTAMBLE) && (in_pos < in_size) )
	{
		length = in_buffer[in_pos++] << 8;
		length += in_buffer[in_pos++];
		address = in_buffer[in_pos++] << 8;
		address += in_buffer[in_pos++];
		
		//fprintf( stderr, "preamble: address 0x%4.4x, length 0x%4.4x\n", address, length );
		
		for( i=0; i<length; i++ )
			buffer[(address+i) & 0xffff] = in_buffer[in_pos++];
		
		type = in_buffer[in_pos++];
	}
	
	length = in_buffer[in_pos++] << 8;
	length += in_buffer[in_pos++];
	address = in_buffer[in_pos++] << 8;
	address += in_buffer[in_pos++];

	//fprintf( stderr, "postamble: address 0x%4.4x, length 0x%4.4x\n", address, length );
	
	/* Precompute output size buffer */
	
	//fprintf( stderr, "\nPrecomputing size\n" );
	*out_size = 0;
	mode = FREESPACE;
	
	for( i=0; i<65535; i++ )
	{
		switch( mode )
		{
			case INBLOCK:
				if( buffer[i] != -1 )
					*out_size = *out_size + 1;
				else
				{
					//fprintf( stderr, "Found end of block: addr %4.4x, current size: %4.4x\n", i, *out_size );
					mode = FREESPACE;
				}
				break;
				
			case FREESPACE:
				if( buffer[i] != -1 )
				{
					*out_size = *out_size + 5 + 1;
					mode = INBLOCK;
					//fprintf( stderr, "Found start block: addr %4.4x, current size: %4.4x\n", i, *out_size );
				}
				break;
		}
	}
	
	/* Add postamble block */
	*out_size = *out_size + 5;
	
	//fprintf( stderr, "\nPrecomputed size: %4.4x\n", *out_size );
	
	/* Create and populate output buffer */
	
	*out_buffer = (unsigned char *)malloc( *out_size );

	if( *out_buffer == NULL )
	{
		//fprintf( stderr, "_decb_binconcat: Out of memory\n" );
		return -1;
	}
	
	out_pos = 0;
	mode = FREESPACE;
	
	for( i=0; i<65535; i++ )
	{
		switch( mode )
		{
			case INBLOCK:
				if( buffer[i] != -1 )
				{
					//fprintf( stderr, "." );
					(*out_buffer)[out_pos++] = buffer[i];
					size++;
				}
				else
				{
					//fprintf( stderr, "\nfound end: %4.4x\n", i );
					(*out_buffer)[size_pointer++] = (size >> 8) & 0xff;
					(*out_buffer)[size_pointer++] = size & 0xff;
					mode = FREESPACE;
				}
				break;
				
			case FREESPACE:
				if( buffer[i] != -1 )
				{
					//fprintf( stderr, "found start: %4.4x ", i );
					(*out_buffer)[out_pos++] = PREAMBLE;
					size_pointer = out_pos;
					out_pos++; /* Reserve space for size */
					out_pos++;
					(*out_buffer)[out_pos++] = (i >> 8) & 0xff;
					(*out_buffer)[out_pos++] = i & 0xff;
					(*out_buffer)[out_pos++] = buffer[i];
					size = 1;
					mode = INBLOCK;
				}
				break;
		}
	}
	
	if( mode == INBLOCK )
	{
		(*out_buffer)[size_pointer++] = (size >> 8) & 0xff;
		(*out_buffer)[size_pointer++] = size & 0xff;
	}
	
	(*out_buffer)[out_pos++] = POSTAMBLE;
	(*out_buffer)[out_pos++] = 0;
	(*out_buffer)[out_pos++] = 0;
	(*out_buffer)[out_pos++] = (address >> 8) & 0xff;
	(*out_buffer)[out_pos++] = address & 0xff;
	
	free( buffer );

	if( out_pos != *out_size )
	{
		fprintf( stderr, "_decb_binconcat: out_pos (%d) != *out_size (%d)\n", out_pos, *out_size );
		return -1;
	}
		
	return 0;
}