/* Test: prerra.c 

	Simple test of the prerr mechnisism.
*/

#define os9in 0
#define os9out 1
#define os9err 2

int main(argc,argv)
int argc;
char **argv;
{
	prerr(os9out,0);

	return 0;
}