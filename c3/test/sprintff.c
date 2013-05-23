/* Test: sprintff.c

	Simple test of sprintf with floats
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
	
	pffinit();
	
	/* All of these values have been checked with the mwcc compiler */
	sprintf( a, "%s %f %d %s", "how", 1.9, 10, "a string" );
	checkit( a, "how 1.900000 10 a string", 10 );
	
	sprintf( a, "%g", 236523325.1415927 );
	checkit( a, "2.365233e+08", 20 );
	
	sprintf( a, "%2.2g", 236523325.1415927 );
	checkit( a, "2.37e+08", 30 );
	
	sprintf( a, "%12.2g", 236523325.1415927 );
	checkit( a, "    2.37e+08", 40 );
	
	sprintf( a, "%-12.2g", 236523325.1415927 );
	checkit( a, "2.37e+08    ", 50 );
	
	sprintf( a, "%f", 236523325.1415927 );
	checkit( a, "236523325.141593", 60 );
	
	sprintf( a, "" );
	checkit( a, "", 70 );
	
	return 0;
}

int checkit( a, b, c )
char *a, *b;
int c;
{
	if( strcmp( a, b ) != 0 )
	{
		printf( "%s\n%s\nError: %d\n", a,b,c );
/*		exit( c ); */
	}
	return 0;
}