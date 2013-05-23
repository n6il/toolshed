/* Test: strtok.c

	Simple test of strtok
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "abc,qwe.sw,r";
	char *b = ",.";
    char *d;
    
	d = strtok( a, b );
	
	if( d != a+0 )
		return 10;
	
	if( a[3] != 0 )
		return 20;
		
	d = strtok( NULL, b );
	
	if( d != a+4 )
		return 30;
	
	if( a[7] != 0 )
		return 40;
	
	d = strtok( NULL, b );
	
	if( d != a+8 )
		return 50;
	
	if( a[10] != 0 )
		return 60;

	d = strtok( NULL, b );
	
	if( d != a+11 )
		return 70;
	
	if( a[12] != 0 )
		return 80;

	d = strtok( NULL, b );
	
	if( d != NULL )
		return 90;

	return 0;
}