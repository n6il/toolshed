/* Test: cfloat.c 

	Simple test of the floats stuff.
*/

int main(argc,argv)
int argc;
char **argv;
{
	double a=1.0, b=1.5, c=3.0;
	float d=1.0, e=10.25, f=25.6;
	int i = 5;
	unsigned u = 6;
	char ch = 's';
	long l = 8733;

	a++;

 	if( (a < 1.9) && (a > 2.1 ) )
		return 10;
	a--;

	if( a != 1.0 )
		return 20;
	
	d++;
	if( (d < 1.9) && (d > 2.1 ) )
		return 30;
	
	d--;
	if( d != 1.0 )
		return 35;
	
	if( a != b )
	{}
	else
		return 50;
	
	if( d != e )
	{}
	else
		return 60;
	
	if( ((a+b) < 2.4) && ((a+b) > 2.6) )
		return 70;
		
	a = e;

	if( a != e )
		return 80;
	
	f = b;
	
	if( (f != b) != 0 )
		return 90;

	i = a;
	if( i != 10 )
		return 100;
	
	l = f;
	if( l != 1l )
		return 110;
	
	b = u;

	if( b != 6.0 )
		return 120;
	
	d = u;
	if( d != 6.0 )
		return 130;
	
	ch = c;
	if( ch != 3 )
		return 140;
	
	e = l;
	if( e != 1.0 )
		return 150;
	
	d = i;
	
	if( d != 10 )
	{}
	else
		return 160;
	
	if( (a*b) != 61.5 )
		return 170;

	if( (a/b) < 1.70 || (a/b) > 1.71)
		return 180;
	
	if( (a-b) != 4.25 )
		return 190;
	
	if( (-b) != -6.0 )
		return 200;
		
	return 0;
	
}