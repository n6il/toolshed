/* Test: memset.c

	Simple test of memset
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *s = malloc( 512 ), *c;
	int i;
	
	if( s == NULL )
		return 10;
		
	c = memset( s, 'c', 512 );
	
	if( s != c )
		return 20;
	
	for( i=0; i<512; i++ )
	{
		if( s[i] != 'c' )
			return 30;
	}
	
	free( s );
	
	return 0;
}