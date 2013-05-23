/* Test: strclr.c

	Simple test of strclr
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "abcdefg\0ighlmnop";
	char *b = "abcdefghoighlmnop";
	char *c;
	
	
	c = strclr( a, 17 );
	
	if( c != a )
		return 10;
		
	if( a[6] != ' ' )
		return 20;
	
	if( a[8] != 'i' )
		return 30;
	
	c = strclr( b, 14 );
	
	if( c != b )
		return 40;
	
	if( b[13] != ' ' )
		return 50;
		
	if( b[16] != 'p' )
		return 60;
		
	return 0;
}