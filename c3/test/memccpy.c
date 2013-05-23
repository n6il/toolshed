/* Test: memccpy.c

	Simple test of memccpy
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "abcdefghighlmnop";
	char *b = malloc( 64 );
	char *c;
	
	if( b == NULL )
		return 10;
	
	c = memset( b, 0, 64 );
	
	if( c != b )
		return 20;
		
	if( b[5] != 0 )
		return 25;

	c = memccpy( b, a, 'h', 16 );
	
	if( c != a+8 )
		return 30;
	
	if( b[7] != 'h' )
		return 40;
	
	c = memset( b, 0, 64 );
	
	if( c != b )
		return 50;
	
	if( b[5] != 0 )
		return 60;
		
	c = memccpy( b, a, 'z', 16 );
	
	if( c != NULL )
		return 70;
	
	if( b[15] != 'p' )
		return 80;
	
	return 0;
}