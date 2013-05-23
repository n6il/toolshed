/* Test: dirxa.c.c 

	Simple test of the dirx_a mechnisism.
*/

int main(argc,argv)
int argc;
char **argv;
{
	char *newpath = "../DEFS";
	char *path = "../SYS";
	char *path2 = "../CMDS";
	int result;
	
	result = chxdir( newpath );
	
	if( result )
		return 10;
	
	result = chxdir( path );
	
	if( result )
		return 20;
	
	result = chxdir( path2 );
	
	if( result )
		return 30;
	
	return 0;	
}