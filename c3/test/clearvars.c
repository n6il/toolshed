/* Test: clearvars.c

	all variables should contain zeros when initialized.
*/

int a;
unsigned b;
char c;
float f;
long l;
double d;

int main(argc,argv)
int argc;
char **argv;
{
	if( a != 0 )
		return 10;
		
	if( b != 0 )
		return 20;
		
	if( c != 0 )
		return 30;
		
	if( f != 0.0 )
		return 40;
		
	if( l != 0l )
		return 50;
		
	if( d != 0.0 )
		return 60;
		
	return 0;
}