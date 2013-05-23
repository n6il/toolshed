/* Test: cshifts.c 

	Simple test of the shifting.
*/

int main(argc,argv)
int argc;
char **argv;
{
	int i=2;
	int b=1;
	unsigned u=2;
	
	if( i<<b != 4 )
		return 10;
	
	if( i>>b != 1 )
		return 20;

	if( u<<b != 4 )
		return 30;
	
	if( u>>b != 1 )
		return 50;
		
	return 0;
}