/* Test: memcmp.c

	Simple test of memcmp
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "abcdef\0ghij";
	char *b = "zxykx";
	char *c;
	
	
	c = memchr( a, 'h', 11 );
	
	if( c == NULL )
		return 10;
		
	if( c != a+8 )
		return 20;
	
	c = memchr( b, 'x', 5 );
	
	if( c == NULL )
		return 30;
	
	if( c != b+1 )
		return 40;
	
	c = memchr( a, 'z', 11 );
	
	if( c != NULL )
		return 50;

	return 0;
}