/* Test: prgname.c
	Simple test of the prgname.c
*/

int main(argc,argv)
int argc;
char **argv;
{
	if (strcmp( _prgname(), "prgname" ) != 0 )
		return 10;
	
	return 0;
}