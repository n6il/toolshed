/* Test: strncat.c

	Simple test of strncat
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "Put here \0                            ";
	char *b = "I am usual ";
    char *d;
    
	d = strncat( a, b, 5 );
	
	if( d != a )
		return 10;
	
	if( a[12] != 'm' )
		return 20;
		
	d = strncat( a, b, 40 );
	
	if( d != a )
		return 30;
	
	if ( a[23] != 'l' )
		return 40;

	return 0;
}
