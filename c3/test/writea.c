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
	char *str = " test\n\l";
	char *strlb = " moof\nmore\n";
	int result;
	
	result = write( os9out, str, 7 );
	if( result == -1)
		return 10;
	
	if( result != 7 )
		return 20;
		
	result = writeln( os9out, strlb, 11 );
	if( result == -1 )
		return 30;
	
	if( result != 6 )
		return 40;
		
	result = write( os9out, str, 7 );
	if( result == -1)
		return 50;
	
	if( result != 7 )
		return 60;
		
	return 0;
}
