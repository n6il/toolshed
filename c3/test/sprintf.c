/* Test: sprintf

	Simple test of sprintf
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int checkit();

int main(argc,argv)
int argc;
char **argv;
{
	char a[256];
	
	sprintf( a, "Hello, this is a string.\n" );
	checkit( a, "Hello, this is a string.\n", 10 );

	sprintf( a, "Hello: %d\n", 10 );
	checkit( a, "Hello: 10\n", 20 );

	sprintf( a, "Hello: %d %s\n", 10, "Super" );
	checkit( a, "Hello: 10 Super\n", 30 );

	sprintf( a, "Hello: %x\n", 10 );
	checkit( a, "Hello: a\n", 40 );

	sprintf( a, "Hello: %X\n", 10 );
	checkit( a, "Hello: A\n", 50 );

	sprintf( a, "%5s", "abc" );
	checkit( a, "  abc", 60 );

	sprintf( a, "%5d", 10 );
	checkit( a, "   10", 70 );

	sprintf( a, "%-5d", 10 );
	checkit( a, "10   ", 80 );

	sprintf( a, "%5.5s ", "abcedgh" );
	checkit( a, "abced ", 90 );

	sprintf( a, "%5.5d", 10 );
	checkit( a, "   10", 100 );

	sprintf( a, "%-5.5d", 10 );
	checkit( a, "10   ", 110 );

	sprintf( a, "%5.5x", 10 );
	checkit( a, "    a", 120 );
	
/*  The man pages for Krieder's printf suggest * is supported
    But that appears to be wrong.
	sprintf( a, "%-*.5d", 5, 10 );
	checkit( a, "10   ", 130 );

	sprintf( a, "%*.*x", 5, 5, 10 );
	checkit( a, "    a", 140 );
	*/
	sprintf( a, "%-5.5X", 10 );
	checkit( a, "A    ", 150 );
	
	/* Test long and float printing without including the glue */
	sprintf( a, "%s %ld %d", "how", 5l, 10 );
	checkit( a, "how  10", 160 );

	sprintf( a, "%s %f %d", "how", 1.5, 10 );
	checkit( a, "how  10", 170 );

	sprintf( a, "" );
	checkit( a, "", 180 );

	return 0;
}

int checkit( a, b, c )
char *a, *b;
int c;
{
	if( strcmp( a, b ) != 0 )
	{
		printf( "%s\n%s\n", a,b );
		exit( c );
	}
	return 0;
}