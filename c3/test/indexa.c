/* Test: indexa.c

	Simple test of strchr, strrchr
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "abacadaeafag";
	char *b = "abcdedcba";
	char *c;
	
	c = strchr( a, 'f' );
	
	if( c != a+9 )
		return 10;
	
	c = strchr( a, 'z' );
	
	if( c != NULL )
		return 20;

	
	c = strrchr( b, 'a' );
	
	if( c != b+8 )
		return 30;
	
	c = strrchr( b, 'z' );
	
	if( c != NULL )
		return 40;
	
	return 0;
}