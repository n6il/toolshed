/* Test: reada_n.c 

	Simple test of the read() system call.
*/

#define os9in 0
#define os9out 1
#define os9err 2

int main(argc,argv)
int argc;
char **argv;
{
	char string[4];
	
	while( read( os9in, string, 1 ) != -1 )
	{
		writeln( os9out, string, 1 );
	}

	return 0;
}
