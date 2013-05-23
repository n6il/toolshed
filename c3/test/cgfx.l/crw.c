/* crw.c - testing cgfx c read/write functions */

#include "lowio.h"
#include "string.h"

int main( argc, argv )
int argc;
char *argv[];
{
	char *s = "should be only the first word\n";
	char *v = "This ends now.\n";
	int i;
	
/*	_Flush(); /* Turn on buffering */
	
	printf( "Hello\nHello2\n" );
	
	i = cwriteln( STDOUT, s, 6 );
	printf( "(%d)\n", i );
	i = cwrite( STDOUT, v, 50 );
	printf( "(%d)\n", i );
	i = write( STDOUT, v, strlen(v) );
	printf( "(%d)\n", i );
    i = writeln( STDOUT, v, strlen(v) );
	printf( "(%d)\n", i );
    
	return 0;
}