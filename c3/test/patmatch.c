/* Test: patmatch.c

	Simple test of patmatch()
*/

#include <string.h>
#include <local.h>

int main(argc,argv)
int argc;
char **argv;
{
	char a[50];
	
	strcpy( a, "BETH.AR" );
	
	if (patmatch("*.ar", a, TRUE) != TRUE )
		return 10;
		
	if (patmatch("*.ar", a, FALSE) != FALSE )
		return 20;
	
	if (patmatch("b?th.ar", a, TRUE) != TRUE )
		return 30;
		
	if (patmatch("b?th.ar", a, FALSE) != FALSE )
		return 40;
	
	if (patmatch("b?th.*", a, TRUE) != TRUE )
		return 50;
		
	if (patmatch("b?th.*", a, FALSE) != FALSE )
		return 60;
	
	return 0;
}