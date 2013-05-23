/* Test: timea.c 

	Simple test of the settime() and gettime() system calls.
*/

#include <time.h>

int main(argc,argv)
int argc;
char **argv;
{
	struct sgtbuf save_time;
	struct sgtbuf adj1_time;
	struct sgtbuf adj2_time;
	int result;
	
	result = getime( &save_time );
	if( result ) return 10;
	
	result = getime( &adj1_time );
	if( result ) return 20;
	
	adj1_time.t_year = 2;
	
	result = setime( &adj1_time);
	if( result ) return 30;
	
	result = getime( &adj2_time );
	if( result ) return 40;
	
	if( adj2_time.t_year != 2 )
		return 50;
		
	result = setime( &save_time);
	if( result ) return 60;
	
	return 0;
}



