/********************************************************************
 * $Id$
 *
 * Endian functions.
 ********************************************************************/
#include <cocopath.h>


unsigned int int4(u_char *a)
{
    return (unsigned int)( (((u_char)*a)<<24) + ((u_char)*(a+1)<<16) + ((u_char)*(a+2)<<8) + (u_char)*(a+3) );
}


unsigned int int3(u_char *a)
{
    return(unsigned int)( (((u_char)*a)<<16) + ((u_char)*(a+1)<<8) + (u_char)*(a+2) );
}


unsigned int int2(u_char *a)
{
    return(unsigned int)( (((u_char)*a)<<8) + (u_char)*(a+1) );
}


unsigned int int1(u_char *a)
{
    return(unsigned int)( ((u_char)*a) );
}


void _int4(unsigned int a, u_char *b)
{
    b[0] = ((a >> 24) & 0xFF); b[1] = ((a >> 16) & 0xFF); b[2] = ((a >> 8) & 0xFF); b[3] = (a & 0xFF);
}


void _int3(unsigned int a, u_char *b)
{
    b[0] = ((a >> 16) & 0xFF); b[1] = ((a >> 8)  & 0xFF); b[2] = (a & 0xFF);
}


void _int2(unsigned int a, u_char *b)
{
    b[0] = ((a >> 8)  & 0xFF); b[1] = (a & 0xFF);
}


void _int1(unsigned int a, u_char *b)
{
    b[0] = (a & 0xFF);
}

unsigned short swap_short(unsigned short in)
{
	unsigned short  out = (in << 8) + (in >> 8);

	return out;
}

unsigned int swap_int(unsigned int in)
{
	unsigned int out = swap_short(in >> 16) + (swap_short(in & 0x0000ffff) << 16);

	return out;
}

/* Read multibyte values stored in little endian format in file */

size_t fread_le_char( unsigned char *ptr, FILE * stream )
{
	size_t count;
	
	count = fread( ptr, 1, 1, stream );
	
	return count;
}

size_t fread_le_short( unsigned short *ptr, FILE * stream )
{
	size_t count;
	
	count = fread( ptr, 1, 2, stream );

#ifdef __BIG_ENDIAN__
	*ptr = swap_short( *ptr );
#endif

	return count;
}

size_t fread_le_sshort( signed short *ptr, FILE * stream )
{
	size_t count;
	
	count = fread( ptr, 1, 2, stream );

#ifdef __BIG_ENDIAN__
	*ptr = swap_short( *ptr );
#endif

	return count;
}

size_t fread_le_int( unsigned int *ptr, FILE * stream )
{
	size_t count;
	
	count = fread( ptr, 1, 4, stream );

#ifdef __BIG_ENDIAN__
	*ptr = swap_int( *ptr );
#endif

	return count;
}

size_t fwrite_le_int(unsigned int data, FILE * stream)
{
#ifdef __BIG_ENDIAN__
	unsigned int    use_data = swap_int(data);
#else
	unsigned int    use_data = data;
#endif

	return fwrite(&use_data, 4, 1, stream);
}

size_t fwrite_le_short(unsigned short data, FILE * stream)
{
#ifdef __BIG_ENDIAN__
	unsigned short  use_data = swap_short(data);
#else
	unsigned short  use_data = data;
#endif

	return fwrite(&use_data, 2, 1, stream);
}

size_t fwrite_le_char(unsigned char data, FILE * stream)
{
	unsigned char  use_data = data;

	return fwrite(&use_data, 1, 1, stream);
}


