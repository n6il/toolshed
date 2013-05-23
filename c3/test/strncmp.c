/* Test: strncmp.c

	Simple test of strncmp.c
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "string";
	char *b = "string";
	char *c = "LesSiF";
	char *d = "LesSiFx";
	int i;
	
	i = strncmp( a, b, 10 );
	if( i != 0 )
		return 10;
		
	i = strncmp( a, b, 5 );
	if( i != 0 )
		return 20;
		
	i = strncmp( a, c, 10 );
	if( i < 0 )
		return 30;
		
	i = strncmp( c, d, 6 );
	if( i != 0 )
		return 40;
		
	i = strncmp( c, d, 7 );
	if( i > 0 )
		return 50;
		
	return 0;
}