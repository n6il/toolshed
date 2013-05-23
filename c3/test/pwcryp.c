/* Test: pwcryp.c

	Simple test of the pwcryp() function
*/

int main(argc,argv)
int argc;
char **argv;
{
	char *s = "password";
	
	s = pwcryp( s );
	
	if( strcmp( s, "064B46" ) != 0 )
		return 10;
	
	return 0;
}