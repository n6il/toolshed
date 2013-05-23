/* Test: utime.c

	Simple test of time(), o2utime(), u2otime(), localtim(), asctime(), ctime()
	
*/

#include <time.h>
#include <utime.h>

int main(argc,argv)
int argc;
char **argv;
{
	long c_time, c_time2;
	struct tm theTM, *theTM2;
	struct sgtbuf sg, sg2;
	
	c_time = time( (long *)0 );
	
	time( &c_time2 );

	if( c_time != c_time2 )
		return 10;
	
	memset( &sg, 0, sizeof sg );
	memset( &sg2, 0, sizeof sg2 );
	memset( &theTM, 0, sizeof theTM );
	
	theTM2 = localtim( &c_time );
	
	u2otime( &sg, theTM2 );

#if 0	
	if( sg.tm_year != theTM.t_year )
		return 20;
		
	if( sg.tm_mon != theTM2->t_month )
		return 30;
		
	if( sg.tm_mday != theTM2->t_day )
		return 40;
		
	if( sg.tm_hour != theTM2->t_hour )
		return 50;
		
	if( sg.tm_min != theTM2->t_minute )
		return 60;
		
	if( sg.tm_sec != theTM2->t_second )
		return 70;		
#endif	
	
	printf( "The date: %s\n", asctime( theTM2 ) );
	printf( "The date: %s\n", ctime( c_time ) );
	
	return 0;	
}
