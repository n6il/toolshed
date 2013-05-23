/* Test: sets.c

	Simple test of set manipulation
	
*/

#include <ctype.h>
#include <sets.h>
#include <local.h>
#include <stdio.h>

char e[256];
char g[256];

int main(argc,argv)
int argc;
char **argv;
{
	char *a, *b, *c, *d, *f;
	char *test = "cdestringzwbuffer";
	char *test1 = "e";
	int i;
	
	for( i=0; i<256; i++ )
	{
		if( e[i] != 0 )
			return 10;
	}
	
	a = allocset();
	
	if( a == NULL )
		return 20;

	addc2set( a, 'c' );
	addc2set( a, 'd' );
	addc2set( a, 'e' );
	adds2set( a, "string" );
	
	b = allocset();
	if( b == NULL )
		return 30;
	
	addc2set( b, 'z' );
	addc2set( b, 'w' );
	addc2set( b, 'q' );
	adds2set( b, "buffer" );
	rmfmset( b, 'q' );

	sunion( a, b );

	for( i=0; i<256; i++ )
	{
		if( e[i] != 0 )
			return 40;
	}

	for( i=0; i<strlen(test); i++ )
		e[test[i]] = TRUE;
	
	for( i=0; i<256; i++ )
	{

		if( logictest( smember( a, i ), e[i]) )
		{
			printf( "1. error at %d (%c)\n", i, i );
			return 30;
		}
	}
	
	f = dupset( a );
	
	if( f == a )
		return 40;
		
	for( i=0; i<256; i++ )
	{

		if( logictest( smember( f, i ), e[i]) )
		{
			printf( "1. error at %d (%c)\n", i, i );
			return 50;
		}
	}
	
	return 0;
}

int logictest( a, b )
int a, b;
{
	if( (a==FALSE) && (b==FALSE ) )
		return FALSE;
	
	if( (a!=FALSE) && (b!=FALSE ) )
		return FALSE;
	
	return TRUE;
}

int printset( s )
char *s;
{
	int i;
	
	printf( "address: %4.4x\n", s );
	
	for( i=0; i<256; i++ )
	{
		printf( "%c", smember( s, i ) ? i : '.' );
	}
	
	printf( "\n" );
}
