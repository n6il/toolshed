/* Test: strnucmp.c

	Simple test of strnucmp
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
	
	i = strnucmp( a, b, 10 );
	if( i != 0 )
		return 10;
		
	i = strnucmp( a, b, 5 );
	if( i != 0 )
		return 20;
		
	i = strnucmp( a, c, 10 );
	if( i < 0 )
		return 30;
		
	i = strnucmp( c, d, 6 );
	if( i != 0 )
		return 40;
		
	i = strnucmp( c, d, 7 );
	if( i > 0 )
		return 50;
		
	return 0;
}