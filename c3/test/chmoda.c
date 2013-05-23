/* Test: chmoda.c 

	Simple test of the chmod mechnisism.
*/

#include <modes.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *name = "startup\n";
	char *notname = "notstartup\n";
	int result;
	
	result = chmod( name, S_IREAD | S_IWRITE | S_IOREAD | S_IOWRITE );
	
	if( result == -1 )
		return 10;

	result = chmod( notname, S_IREAD | S_IWRITE | S_IOREAD | S_IOWRITE );
	
	if( result != -1 )
		return 20;
	
	return 0;	
}