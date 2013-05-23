/* Test: mktemp.c

	Simple test of mktemp()
*/

#include <string.h>
#include <local.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "file.XXXXX", *b = "file.XXXXX", *c;
	
	c = mktemp(a);
	
	if( c != a )
		return 10;
	
	if( strcmp( a,b ) == 0 )
		return 20;
	
	if( patmatch( "file.*", a, TRUE ) == FALSE )
		return 30;

	return 0;
}