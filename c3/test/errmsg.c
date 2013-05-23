/* Test: errmsg.c
	Simple test of the errmsg()
*/

int main(argc,argv)
int argc;
char **argv;
{
	pffinit();
	
	if ( _errmsg( 10, "Just a Plain error.\n" ) != 10 )
		return 10;

	if ( _errmsg( 20, "Just a %s error.\n", "big" ) != 20 )
		return 20;

	if ( _errmsg( 30, "Just a %s error(%d).\n", "big", 30 ) != 30 )
		return 30;

	if ( _errmsg( 40, "Just a %s error(%d) - %u.\n", "big", 40, 65000 ) != 40 )
		return 40;

	if ( _errmsg( 50, "%s a %s error(%d) - %u.\n", "Hey,", "big", 65000, 50 ) != 50 )
		return 50;

	return 0;
}