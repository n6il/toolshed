/* Test: calloc.c

	Simple test of Calloc and free.
*/

#include <stdlib.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *b1 = calloc( 128, 1 );
	char *b2 = calloc( 512, 1 );
	char *b3 = calloc( 128, 1 );
	char *b4 = calloc( 512, 1 );
	int i;
	
	if( b1 == NULL )
		return 10;
		
	if( b2 == NULL )
		return 20;
		
	if( b3 == NULL )
		return 30;
		
	if( b4 == NULL )
		return 40;
		
	for( i=0; i<128; i++ )
		b1[i] = 0x55;
	 
	for( i=0; i<512; i++ )
		b2[i] = 0x65;
	 
	for( i=0; i<128; i++ )
		b3[i] = 0x45;
	 
	for( i=0; i<512; i++ )
		b4[i] = 0x35;
	 
	if( *b1 != 0x55 )
		return 50;
	
	if( *b2 != 0x65 )
		return 60;
	
	if( *b3 != 0x45 )
		return 70;

	if( *b4 != 0x35 )
		return 80;

	free( b1 );
	free( b2 );
	free( b3 );
	free( b4 );
	
	/* This loop should eventually exit without trashing memory */
	while ( (b1 = calloc( 2, 2 )) != NULL )
	{
		b1[0] = 0;
		b1[2] = 0;
		b1[2] = 0;
		b1[3] = 0;
	}

	return 0;
}