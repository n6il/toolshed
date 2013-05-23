/* Test: chain.c 

	Simple test of the chain() system call.
*/

#include <os9.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *modname = "echo";
	int paramsize = 16;
	char *paramptr = " Chain() worked\n";
	int type = 0x01;
	int lang = 0x01;
	int datasize = 0;
	
	chain( modname, paramsize, paramptr, type, lang, datasize );
	
	/* If chain works, we should never get here */
	return 10;
}

