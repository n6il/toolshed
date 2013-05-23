/* Test: sleepa.c 

	Simple test of the floats stuff.
*/

#include <time.h>

int main(argc,argv)
int argc;
char **argv;
{
	struct sgtbuf start_time;
	struct sgtbuf end_time;
	int result, tm_diff;

	result = getime( &start_time );
	if( result ) return 10;
	
	result = sleep( 5 ); /* should sleep for 5 seconds */
	
	result = getime( &end_time );
	if( result ) return 20;
	
	tm_diff = (end_time.t_minute*60+end_time.t_second) -
	                (start_time.t_minute*60+start_time.t_second);
	
	if( tm_diff < 4 || tm_diff > 6 )
		return 30;
		
	return 0;
	
}