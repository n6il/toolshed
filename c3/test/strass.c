/* Test: strass.c 

	Simple test of strass.
*/

typedef struct
{
	int a;
	long b;
	float c;
} t_s;

int main(argc,argv)
int argc;
char **argv;
{
	t_s a, b;
	
	a.a = 1;
	a.b = 2l;
	a.c = 1.2;

	_strass( &b, &a, sizeof a );
	
	if( b.a != 1 )
		return 10;
	
	if( b.b != 2l )
		return 20;
	
	if( b.c < 1.19 || b.c > 1.21 )
		return 30;
	
	return 0;
}