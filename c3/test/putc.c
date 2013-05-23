/* Test: putc.c

	Simple test of put.c
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "Match this putc.\n";
	char *b = "Match this putw.\n\n";
	unsigned *c;
	int i;
	
	writeln( 1, a, strlen( a ) );

	for( i=0; i<strlen(a); i++ )
	{
		if( putc( a[i], stdout ) != a[i] )
			return 10;
	}

	writeln( 1, b, strlen( b ) );

	c=b;
	
	for( i=0; i<strlen(b)/2; i++ )
	{
		if( putw( c[i], stdout ) != c[i] )
			return 20;
	}
		
	return 0;
}