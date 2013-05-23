/* status.c - testing cgfx status functions */

#include <lowio.h>
#include <cgfx/buffs.h>
#include <stdio.h>

#define MAP_IN 1
#define MAP_OUT 0

int main( argc, argv )
int argc;
char *argv[];
{
	int i;
	char s[255], v[100];
	FILE *f;
	char *arrow;
	int size;
	
/*	system( "merge /dd/sys/stdptrs >/term" );*/
	
	printf( "Loading pointers\n" );
	
	f = fopen( "/dd/sys/stdptrs", "r" );
	
	if ( f == NULL )
	{
		fprintf( stderr, "Could not open /dd/sys/stdptrs\n" );
		return 1;
	}
	
	size = 0;
	while( (i = fread( s, STDOUT, 255, f )) != 0 )
	{
		size += i;
		write( STDOUT, s, i );
	}
	
	fclose( f );

	printf( "Pointers file size: %d\n", size );
	
	printf( "Mapping in arrow pointer\n" );
	
	arrow = _ss_mgpb( STDOUT, GRP_PTR, PTR_ARR, MAP_IN, &size );
	sprintf( v, "arrow (%d)", size );
	
	if( arrow == 0 )
		fprintf( stderr, "Mapping error: %x\n", errno );
	else
		_dump( v, arrow, size, stdout );

	return 0;
}