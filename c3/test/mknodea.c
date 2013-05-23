/* Test: mknodea.c 

	Simple test of the mknode() system call.
*/

#include <modes.h>

long lseek();

#define os9in 0
#define os9out 1
#define os9err 2

int main(argc,argv)
int argc;
char **argv;
{
	char *file = "TEMP/newfile\n";
	char *dir = "TEMP\n";
	char buf[10];
	int path, result;

	result = mknod( dir, S_IREAD | S_IWRITE | S_IOREAD | S_IOWRITE );
	
	if( result == -1 )
		return 10;
		
	path = creat( file, S_IREAD | S_IWRITE | S_IOREAD | S_IOWRITE );
	
	if( path == -1 )
		return 20;
	
	result = writeln( path, file, 7 );
	
	if( result == -1 )
		return 30;
	
	if( result != 7 )
		return 40;
	
	close( path );

	result = unlink( file );
	
	if( result == -1 )
		return 50;

	return 0;
}
