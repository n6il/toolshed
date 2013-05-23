/* Test: rand.c 

	Simple test of the rand and srand functions.
*/

int rand();

int main(argc,argv)
int argc;
char **argv;
{
	srand( 16235 );
	
	if( rand() != 19247 )
		return 10;
	
	if( rand() != 12754 )
		return 20;
	
	if( rand() != 19714 )
		return 30;
	
	if( rand() != 27745 )
		return 40;
	
	if( rand() != 15066 )
		return 50;

	if( rand() == 0 )
		return 60;
	
	return 0;
	
}