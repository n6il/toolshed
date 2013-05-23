/* Test: cfloat_div.c 

	Simple test of the floats stuff.
*/

int main(argc,argv)
int argc;
char **argv;
{
	double a=6.25, b=1.5, c=3.0;
	double aa = -1.0, bb = -23.34;
	float d=1.0, e=10.25, f=25.6;
	int i = 5;
	unsigned u = 6;
	char ch = 's';
	long l = 8733;

	c = b/a;
	if( c < 0.23 || c > 0.25 )
		return 10;
		
	c = c * i;
	if( c < 1.1 || c > 1.3 )
		return 20;
		
	d = e/f;
	if( d < 0.3 || d > 0.5 )
		return 30;
	
	d = d * i;
	if( d < 1.9 || d > 2.1 )
		return 50;
	
	return 0;
	
}