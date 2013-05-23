/* Test: strcmp.c

	Simple test of strcmp
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
	
	i = strcmp( a, b );
	if( i != 0 )
		return 10;
		
	i = strcmp( a, b );
	if( i != 0 )
		return 20;
		
	i = strcmp( a, c );
	if( i < 0 )
		return 30;
		
	i = strcmp( c, d );
	if( i > 0 )
		return 40;

	return 0;
}