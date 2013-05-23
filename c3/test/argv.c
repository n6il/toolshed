/* Test: argv.c 

	Simple test of the argc mechnisism.
*/

int main(argc,argv)
int argc;
char **argv;
{
	if( argc == 1 )
		return 0;
	
	return 10;
}