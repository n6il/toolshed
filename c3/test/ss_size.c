/* Test: ss_size.c 

	Simple test of the creat() system call.
*/

#include <modes.h>

#include <os9.h>
#include <sgstat.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *string = "/dd/ss_sizefile\n";
	int path, result;
	long size;
	
	path = creat( string, S_IREAD | S_IWRITE | S_IOREAD | S_IOWRITE );
	
	if( path == -1 )
		return 10;
	
	result = writeln( path, string, 16 );
	
	if( result == -1 )
		return 20;
	
	if( result != 16 )
		return 30;

	result = getstat(SS_SIZE, path, &size);
	
	if( result == -1 )
		return 40;
		
	if( size != 16 )
		return 50;

	result = _ss_size( path, 255l );
	
	if( result == -1 )
		return 60;
	
	result = getstat(SS_SIZE, path, &size);
	
	if( result == -1 )
		return 70;
		
	if( size != 255l )
		return 80;
		
	close( path );

	return 0;
}
