/* Test: clconvert.c 

	Simple test of the lng converting.
*/

int main(argc,argv)
int argc;
char **argv;
{
	long l=27, o=-27;
	long a=4, b = -4;

	if( (l*a) != 108l )
		return 10;
	
	if( (l*b) != -108l )
		return 20;
	
	if( (o*a) != -108l )
		return 30;
	
	if( (o*b) != 108l )
		return 40;
	

	if( (a*l) != 108l )
		return 50;
	
	if( (b*l) != -108l )
		return 60;
	
	if( (a*o) != -108l )
		return 70;
	
	if( (b*o) != 108l )
		return 80;

	return 0;
}