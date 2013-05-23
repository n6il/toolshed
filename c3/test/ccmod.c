/* Test: ccmod.c 

	Simple test of the integer moduls.
*/

int main(argc,argv)
int argc;
char **argv;
{
	int i=5;
	int b=2;
	unsigned u=5;
	unsigned x=2;
	
	if( i%b != 1 )
		return 10;
	
	if( u%x != 1 )
		return 20;

	if( i%x != 1 )
		return 30;
	
	if( u%b != 1 )
		return 40;
		
	return 0;
}