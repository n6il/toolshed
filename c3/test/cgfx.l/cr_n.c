/* crw_read.c - testing cgfx c read/write functions */

#include <lowio.h>

int main( argc, argv )
int argc;
char *argv[];
{
	char s[101];
	char v[100];
	int i;
	
_Flush(); /* Turn on buffering */

	i = cread( STDIN, s, 100 );

	printf( "cread (%d): %s\n", i, s );
	
	i = creadln( STDIN, s, 100 );
	
	printf( "creadln (%d): %s\n", i, s );
	
	i = read( STDIN, s, 100 );

	sprintf( v, "readln (%d)", i );
	_dump( v, s, 100 );
	
	i = readln( STDIN, s, 100 );
	
	sprintf( v, "readln (%d)", i );
	_dump( v, s, 100 );
	
	return 0;
}