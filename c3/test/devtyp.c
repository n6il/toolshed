/* Test: devtyp.c

	Simple test of devtyp() and isatty()
	
*/

#include <ctype.h>
#include <sets.h>
#include <local.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	int a;
	FILE *f;
	
	/* Normally stdin is from a tty, but when run from a script
	   standard in is the script file.
	   It's a weird OS-9ism. */
	if( isatty(stdin->_fd) == TRUE )
		return 10;
		
	a = devtyp( stdout->_fd );
	
	if( a != 0 )
		return 20;
	
	f = fopen( "newfile", "w" );
	
	putc( 'c', f );
	
	a = devtyp( f->_fd );
	if( a != 1 )
		return 30;
	
	fclose( f );
	
	return 0;	
}
