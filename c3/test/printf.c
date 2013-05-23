/* Test: findstr.c

	Simple test of findstr and findnstr
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	unsigned a = htoi( "ffff" );
	
	printf( "String, no args.\n" );
	printf( "10\n%d\n", 10 );
	printf( "10 Super\n%d %s\n", 10, "Super" );
	printf( "a\n%x\n", 10 );
	printf( "A\n%X\n", 10 );
	printf( "-1\n%d\n", a );
	printf( "65535\n%u\n", a );
	
	return 0;
}