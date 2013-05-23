/* Test: chowna.c 

	Simple test of the chown() mechnisism.
*/

#include <modes.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *name = "/dd/chownafile\n";
	int result, path;
		
	path = creat( name, S_IREAD | S_IWRITE | S_IEXEC );

	if( path == -1 )
		return 10;
	
	close( path );

	result = chown( name, 10 );
	
	if( result == -1 )
		return 20;

	result = setuid( 2 );
	if( result == -1 )
		return 30;

	result = chown( name, 20 );
	
	if( result != -1 )
		return 40;
	
	return 0;	
}