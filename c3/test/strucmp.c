/* Test: strucmp.c

	Simple test of strucmp
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "stRing";
	char *b = "StrinG";
	char *c = "LesSiF";
	char *d = "Lessifx";
	int i;
	
	i = strucmp( a, b );
	if( i != 0 )
		return 10;
		
	i = strucmp( b, a );
	if( i != 0 )
		return 20;
		
	i = strucmp( a, c );
	if( i < 0 )
		return 30;
		
	i = strucmp( c, d );
	if( i > 0 )
		return 50;
		
	return 0;
}