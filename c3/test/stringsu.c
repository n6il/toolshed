/* Test: stringsu.c

	Simple test of strucat and strucpy
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define os9out 1


int main(argc,argv)
int argc;
char **argv;
{
	char *a = "stRing ";
	char *b = "LesSiF\0            ";
	char *c = "Lessifx";
	char *d;
	int i;
	
	d = strucpy( a, c );

	
	if( d != a )
		return 10;
	
	if( a[6] != 'X' )
		return 20;

	d = strucat( b, a );
	
	
	if( d != b )
		return 30;

	if( b[12] != 'X' )
		return 40;	

	return 0;
}