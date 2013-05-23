/* Test: memcmp.c

	Simple test of memcmp
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "string";
	char *b = "boo";
	char *c = "felix";
	char *d = "felixing";

	if( memcmp( a, b, 3 ) != 17 )
		return 10;
		
	if( memcmp( a, c, 3 ) != 13 )
		return 20;
		
	if( memcmp( a, d, 3 ) != 13 )
		return 30;
		
	if( memcmp( b, a, 3 ) != -17 )
		return 40;
		
	if( memcmp( b, c, 3 ) != -4 )
		return 50;
		
	if( memcmp( b, d, 3 ) != -4 )
		return 60;
		
	if( memcmp( c, a, 3 ) != -13 )
		return 70;
		
	if( memcmp( c, b, 3 ) != 4 )
		return 80;
		
	if( memcmp( c, d, 3 ) != 0 )
		return 90;
		
		
	return 0;
}