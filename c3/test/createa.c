/* Test: createa.c 

	Simple test of the creat() system call.
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
	char *string = "createfile\n";
	char buf[10];
	int path, result;
	long seek;
	
	path = creat( string, S_IREAD | S_IWRITE | S_IOREAD | S_IOWRITE );
	
	if( path == -1 )
		return 10;
	
	result = writeln( path, string, 7 );
	
	if( result == -1 )
		return 20;
	
	if( result != 7 )
		return 30;
	
	seek = lseek( path, 0l, 0 );

	if( seek == -1l )
		return 40;
	
	if( seek != 0l )
		return 50;
		
	result = read( path, buf, 9 );

	if( result == -1 )
		return 60;
	
	if( result != 7 )
		return 70;
	
	if( buf[0] != 'c' )
		return 80;

	seek = lseek( path, -4l, 1 );

	if( seek == -1l )
		return 90;
		
	if( seek != 3l )
		return 100;
		
	result = read( path, buf, 9 );

	if( result == -1 )
		return 110;
	
	if( result != 4 )
		return 120;
		
	if( buf[0] != 'a' )
		return 130;

	seek = lseek( path, -3l, 2 );
	
	if( seek == -1l )
		return 140;
	
	if( seek != 4l )
		return 150;
	
	result = read( path, buf, 9 );
	
	if( result == -1 )
		return 150;
	
	if( result != 3 )
		return 160;
	
	if( buf[0] != 't' )
		return 170;
		
	close( path );

	seek = lseek( path, -4l, 2 );
	
	if( seek != -1l )
		return 180;
	
	result = unlink( string );
	
	if( result == -1 )
		return 190;
	
	path = open( string, 1 );
	
	if( path != -1 )
		return 200;

	return 0;
}
