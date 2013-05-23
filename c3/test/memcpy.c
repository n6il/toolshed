/* Test: memcpy.c

	Simple test of memcpy
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *s = malloc( 512 ), *b = malloc( 512 ), *c;
	int i;
	
	if( s == NULL )
		return 10;
		
	if( b == NULL )
		return 20;
		
	c = memset( s, 'e', 512 );
	
	if( s != c )
		return 30;
	
	c = memcpy( b, s, 512 );
	
	if( b != c )
		return 40;
	
	for( i=0; i<512; i++ )
	{
		if( b[i] != 'e' )
			return 50;
	}
	
	free( s );
	free( b );
	
	return 0;
}