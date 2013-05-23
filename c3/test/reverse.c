/* Test: reverse.c

	Simple test of reverse
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "string";
	char *b = "gnirts";
	char *c;
	
	c = reverse( a );
	
	if( c != a )
		return 10;
	
	if( strcmp( a, b ) != 0 )
		return 20;
	
	return 0;
}