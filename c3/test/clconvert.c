/* Test: clconvert.c 

	Simple test of the lng converting.
*/

int main(argc,argv)
int argc;
char **argv;
{
	long l;
	int i = 8;
	char c = 8;
	unsigned u = 8;

	l=i;
	
	if( l != 8 )
		return 10;
	
	l=c;
	
	if( l != 8 )
		return 20;
	
	l=u;
	
	if( l != 8 )
		return 30;
		
	i = -8;
	c = -8;
	u = -8;


	l=i;
	
	if( l != -8 )
		return 40;
	
	l=c;
	
	if( l != -8 )
		return 50;
	
	l=u;

	if( l != (unsigned)-8 )
		return 60;

	return 0;
}