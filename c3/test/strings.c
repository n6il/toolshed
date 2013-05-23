/* Test: strings.c

	Simple test of strcat, strcpy, strend
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "string                            ";
	char *b = " the ";
	char *c = "way.";
	char *d = "New begining";
	char *e = "New begining the way.";
	char *f;
	
	f = strcpy( a, d );

	if( f != a )
		return 10;
		
	f = strcat( a, b );

	if( f != a )
		return 20;
		
	f = strcat( a, c );

	if( f != a )
		return 30;
			
	if( strcmp( a, e ) != 0 )
		return 40;
	
	f = strend( a );
	
	if ( f != a+21 )
		return 50;
		
	return 0;
}