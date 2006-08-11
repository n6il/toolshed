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
