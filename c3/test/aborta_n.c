/* Test: aborta.c 

	Simple test of the creat() system call.
*/

int i=1;
int b=2;
int c=3;
int d=4;

int main(argc,argv)
int argc;
char **argv;
{
	abort();
	
	return 0;
}
