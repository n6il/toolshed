/* Test: sprintf

	Simple test of sprintf with longs
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
	
	pflinit();
	
	sprintf( a, "%s %ld %d %s", "how", 5l, 10, "a string" );
	checkit( a, "how 5 10 a string", 10 );
	
	/* This represents missing functionality from the library.
	   This behaivor matches the MW C manual. But the Krieder
	   man page suggests this should be supported, but isn't. */
	sprintf( a, "%ld %lu %s", 0xffffffffl, 0xffffffffl, "small" );
	checkit( a, "-1 lu small", 20 );

	sprintf( a, "%ld %lx", 0xffffffffl, 0xffffffffl );
	checkit( a, "-1 ffffffff", 30 );

	sprintf( a, "%ld %lX", 0xffffffffl, 0xffffffffl );
	checkit( a, "-1 FFFFFFFF", 40 );

	sprintf( a, "%ld %lo", 0xffffffffl, 0xffffffffl );
	checkit( a, "-1 37777777777", 50 );

	sprintf( a, "" );
	checkit( a, "", 60 );

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