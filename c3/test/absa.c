/* Test: absa.c 

	Simple test of the abs().
*/

int main(argc,argv)
int argc;
char **argv;
{
	int a=-5;
	int b=2;
	int c=-560;
	int d=8029;
	int e=-31854;

	if( abs(a) != 5 )
		return 10;
	
	if( abs(b) != 2 )
		return 20;
	
	if( abs(c) != 560 )
		return 30;
	
	if( abs(d) != 8029 )
		return 40;
	
	if( abs(e) != 31854 )
		return 50;
		
	if( abs(5) != 5 )
		return 60;
	
	if( abs(-2) != 2 )
		return 70;
	
	if( abs(560) != 560 )
		return 80;
	
	if( abs(-8029) != 8029 )
		return 90;
	
	if( abs(31854) != 31854 )
		return 100;
		
	return 0;
}