/* Test: findstr.c

	Simple test of findstr and findnstr
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = " a long string here string";
	char *b = "string";
	char *c = "wump";
	char *d = " a long string \0ere string";
	int i;
		
	i = findstr( 1, a, b );
	
	if( i != 9 )
		return 10;
		
	i = findstr( 15, a, b );
	
	if( i != 21 )
		return 20;

	i = findstr( 1, a, c );
	
	if( i != 0 )
		return 30;

	i = findnstr( 1, d, b, 28 );
	
	if( i != 9 )
		return 10;
		
	i = findnstr( 15, d, b, 28 );
	
	if( i != 21 )
		return 20;

	i = findnstr( 1, d, c, 28 );
	
	if( i != 0 )
		return 30;

	return 0;
}