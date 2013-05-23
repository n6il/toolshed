/* Test: ccdiv.c 

	Simple test of the integer division.
*/

int main(argc,argv)
int argc;
char **argv;
{
	int i=4;
	int b=2;
	unsigned u=4;
	unsigned x=2;
	
	if( i/b != 2 )
		return 10;
	
	if( u/x != 2 )
		return 20;

	if( i/x != 2 )
		return 30;
	
	if( u/b != 2 )
		return 40;
		
	return 0;
}