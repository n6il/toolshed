/* Test: argv copy.c

	Simple test of the directory utility mechnisim.
*/

#include <stdio.h>
#include <dir.h>

int main(argc,argv)
int argc;
char **argv;
{
	DIR *aDir;
	DIRECT *anEntry;
	
	aDir = opendir( "/dd/fakedir\n" );
	
	if( aDir != NULL )
		return 10;
	
	aDir = opendir( "/dd/CMDS\n" );
	
	if( aDir == NULL )
		return 20;
	
	anEntry = readdir( aDir );
	
	while( anEntry != NULL )
	{
		printf( "%s, ", anEntry->d_name );
		anEntry = readdir( aDir );
	}
	
	printf( "\n" );
	
	rewinddir( aDir );
	
	anEntry = readdir( aDir );
	
	while( anEntry != NULL )
	{
		printf( "%s, ", anEntry->d_name );
		anEntry = readdir( aDir );
	}
	
	printf( "\n" );

	closedir( aDir );
		
	return 0;
}