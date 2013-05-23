/* Test: strspn.c

	Simple test of strspn and strcspn
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "aaabbbcccdefg";
	char *b = "abc";
	char *c = "xyz";
	int d;
	char *e = "aaabbbcczyx";

	d = strspn( a, b );
	
	if( d != 9 )
		return 10;
		
	d = strspn( a, c );
	
	if( d != 0 )
		return 20;

	d = strcspn( e, c );
	
	if( d != 8 )
		return 30;
	
	d = strcspn( e, b );
	
	if( d != 0 )
		return 40;
		
	return 0;
}