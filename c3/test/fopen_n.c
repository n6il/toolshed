/* Test: fopen.c

	Simple test of fopen(), freopen(), fdopen();
	
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int printfd();

int main(argc,argv)
int argc;
char **argv;
{
	FILE *f, *g;
	char c;

	f = fopen( "startup", "r" );
	
	if( f == NULL ) return 10;
	
	printfd( f );
	
	g = fopen( "newfile", "w" );

	if( g == NULL ) return 20;

	printfd( g );
	
	while( (c = getc( f )) != -1)
	{
		putc( c, g );
	};
	
	printf( "\n" );
	
	fclose( f );

	fclose( g );
	
	f = fopen( "newfile", "a" );
	if( f == NULL ) return 30;

	printfd( f );

	fprintf( f, "This is an additional line.\n" );
	fprintf( f, "This is another additional line.\n" );
	
	fclose( f );
	
	return 0;
}

int printfd( f )
FILE *f;
{
	printf( "FILE Pointer: %4.4x\n", f );
	printf( "       buffer pointer: %4.4x\n", f->_ptr );
	printf( "  buffer base address: %4.4x\n", f->_base );
	printf( "   buffer end address: %4.4x\n", f->_end );
	printf( "          flag status: %4.4x\n", f->_flag );
	printf( "     file path number: %4.4x\n", f->_fd );
	printf( "           unget save:   %2.2x\n", f->_save );
	printf( "          buffer size: %4.4x\n\n", f->_bufsiz );
}