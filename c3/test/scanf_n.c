/* Test: scanf_n.c 

	Simple test of the scanf mechnisism.
*/
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	int d;
	unsigned u;
	
	
	printf( "Type a decimal integer: " );
	scanf( "%d", &d );
	
	printf( "Integer: %d\n", d );
	fflush( stdin );
	
	printf( "Type a unsigned integer: " );
	scanf( "%u", &u );
	
	printf( "Integer: %u\n", u );
	fflush( stdin );
	
	printf( "Type a octal integer: " );
	scanf( "%o", &d );
	
	printf( "Integer: %d\n", d );
	fflush( stdin );
	
	printf( "Type a hex integer: " );
	scanf( "%x", &d );
	
	printf( "Integer: %d\n", d );
	fflush( stdin );
	
	return 0;
}