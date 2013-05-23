/* Test: form.c 

	Simple test of the fork() system call.
*/

int waithere();

int main(argc,argv)
int argc;
char **argv;
{
	char *modname = "echo\n";
	int paramsize = 15;
	char *paramptr = " Fork() forked\n";
	int type = 0x01;
	int lang = 0x01;
	int datasize = 0;
	int	status;
	
	os9fork( modname, paramsize, paramptr, type, lang, datasize );
	
	return waithere();
}

int waithere()
{
	int status;
	
	wait( &status );
	
	return status;
}