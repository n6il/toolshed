/* Test: atofa.c 

	Simple test of the atof library functions.
*/

#include <stdlib.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *s1 = "10.2";
	char *s2 = "-5.3";
	char *s3 = "  -3.0";
	char *s4 = "23.8x";
	char *s5 = "20.000";
	
	pffinit();
	
	printf( "%f\n", atof( s1 ) );
	printf( "%f\n", atof( s2 ) );
	printf( "%f\n", atof( s3 ) );
	printf( "%f\n", atof( s4 ) );
	printf( "%f\n", atof( s5 ) );
	
	if( atof(s1) != 10.2 )
		return 10;
		
	if( atof(s2) != -5.3 )
		return 20;
		
	if( atof(s3) != -3.0 )
		return 30;

	if( atof(s4) != 23.8 )
		return 50;
		
	if( atof(s5) != 20.0 )
		return 60;
		
	return 0;	
}
