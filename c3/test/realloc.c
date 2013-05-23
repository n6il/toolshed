/* Test: realloc.c 

	Simple test of realloc.
*/

#include <stdlib.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *b1 = malloc( 64 );
	char *b2;
	
	if( b1 == NULL )
		return 10;
	
	b1[0] = 0x23;
	
	b2 = realloc( b1, 128 );
	
	if( b2[0] != 0x23 )
		return 20;
	
	free( b2 );
	
	return 0;
}