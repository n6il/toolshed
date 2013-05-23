/* Test: unbuffered.c

	Simple test of unbuffered output
	
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int printfd();

int main(argc,argv)
int argc;
char **argv;
{
	char a[BUFSIZ];
	FILE *f;
	
	fprintf( stderr, "This is unbuffered error output.\n" );
	
	f = fopen( "newfile2", "w" );
	
	if( f == NULL ) return 10;
	
	setbuf( f, NULL );

	fputs( "Woa\n", f );
	fprintf( f, "My new hero.%d\n", 10 );
	
	fclose( f );
	
	return 0;
}
