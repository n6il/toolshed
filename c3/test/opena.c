/* Test: writea.c 

	Simple test of the write() system call.
*/

#define os9in 0
#define os9out 1
#define os9err 2

int main(argc,argv)
int argc;
char **argv;
{
	char *string = "/dd/startup\n";
	char buf[4];
	int path, result;
	
	path = open( string, 1 );
	
	if( path == -1 )
		return 10;
	
	result = read( path, buf, 1 );
	
	if( result == -1 )
		return 20;
	
	if( result != 1 )
		return 30;
	
	if( buf[0] != 'e' )
		return 40;
	
	result = close( path );

	if( result == -1 )
		return 50;
		
	return 0;
}
