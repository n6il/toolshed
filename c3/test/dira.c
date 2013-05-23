/* Test: dira.c 

	Simple test of the dir_a mechnisism.
*/

int main(argc,argv)
int argc;
char **argv;
{
	char *newpath = "DEFS";
	char *path = "../SYS";
	char *path2 = "..";
	int result;
	
	result = chdir( newpath );
	
	if( result )
		return 10;
	
	result = chdir( path );
	
	if( result )
		return 20;
	
	result = chdir( path2 );
	
	if( result )
		return 30;
	
	return 0;	
}