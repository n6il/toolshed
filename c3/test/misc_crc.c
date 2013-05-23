/* Test: misc_crc.c 

	Simple test of the crc() system call.
*/

#include <os9.h>

int main(argc,argv)
int argc;
char **argv;
{
	unsigned accum[3];
	char *buf = "This is a buffer";
	
	accum[0] = accum[1] = 0xffff;
	
	crc( buf, 16, accum );

	if( accum[0] == 0xb3a7 )
	{
		if( accum[1] == 0xccff )
		{
			return 0;
		}
	}
	
	return 10;
}