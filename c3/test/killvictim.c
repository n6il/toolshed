/* Test: killvictim.c 

	When this program is started by killproc it runs, waiting to be killed.
	When run by signala it wait for two signals to be sent then exits.
*/
#include <signal.h>

int value;

dosig( sig )
int sig;
{
	if( sig == 10 )
	{
		signal( 10, dosig );
		value++;
	}
}

int main(argc,argv)
int argc;
char **argv;
{
	int i;
	
	if( argc == 2 )
	{
		switch( argv[1][0] )
		{
			case 'K': /* used by killproc.c */
				while( 1 )
				{
					i++;
				}
			
				return 20;
			break;
			
			case 'S': /* used by signala.c */
				value = 0;

				signal( 10, dosig );
				
				while( value < 2 )
				{}
				
				return 40;
				break;
			
			case 'P': /* Used by popen */
				printf( "Success!\n" );
				break;
				
			default:
				break;
		}
	}
	
	return 0;
}
