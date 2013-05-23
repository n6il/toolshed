/* Test: getdrive.c

*/

char *getdrive();

int main(argc,argv)
int argc;
char **argv;
{
	if( strcmp( getdrive(), "/DD" ) != 0 )
		return 10;
	
	return 0;
}
