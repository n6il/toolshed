/* Test: argv.c 

	Simple test of the argc mechnisism.
*/

#include <math.h>
#include <stdlib.h>

int main(argc,argv)
int argc;
char **argv;
{
	double d;
	
	pffinit();
	
	if( argc != 2 )
	{
		printf( "Usage: %s 1.23\nWill display the sin of the argument\n", argv[0] );
		return 0;
	}
	
	d = atof( argv[1] );

	printf( "sin(%s) = %f\n", argv[1], sin( d ) );
	
	return 0;
}