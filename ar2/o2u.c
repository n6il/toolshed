
#ifndef lint
static char *id = "$Id$";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source$
 * $RCSfile$
 * $Revision$
 * $Date$
 * $State$
 * $Author$
 * $Locker$
 *
 *------------------------------------------------------------------
 *
 * Carl Kreider
 * 22305 CR 28
 * Goshen, IN  46526
 * (219) 875-7019
 * (71076.76@compuserve.com, ckreider@skyenet.net, carlk@syscon-intl.com)
 *
 *------------------------------------------------------------------
 * $Log$
 * Revision 1.2  1996/07/20 22:31:13  cc
 * Merged in pwz's unixification (Sunos).
 *
 * Revision 1.1  96/07/20  17:10:39  cc
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

# include "o2u.h"
#ifndef SYSV
# include <time.h>
#else 
# include <sys/time.h>
#endif 

# define YEAR_LENGTH(Y) (((Y) & 3) ? 365 : 366)

typedef struct
        {
        char    os9Flag;
        short   unixFlag;
        } MODE_MAP;

static MODE_MAP modeMap[] =
        {
        { 0x01, S_IREAD   },
        { 0x02, S_IWRITE  },
        { 0x04, S_IEXEC   },
        { 0x08, S_IOREAD  },
        { 0x10, S_IOWRITE },
        { 0x20, S_IOEXEC  },
        { 0x40, 0x0       }         /* S_ISHARE  */
        };

static int	monthLength[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
        };


char	*u2oDate(l)
long	l;
	{
	static char	buf[5];
	struct tm	*tp;

	tp = localtime(&l);
	buf[0] = CHARACTER_OF(tp->tm_year);
	buf[1] = CHARACTER_OF(tp->tm_mon) + 1;
	buf[2] = CHARACTER_OF(tp->tm_mday);
	buf[3] = CHARACTER_OF(tp->tm_hour);
	buf[4] = CHARACTER_OF(tp->tm_min);
	return (buf);
	}


long	o2uDate(buf)
char	*buf;
	{
	register int	i;
	int				year;
	int				month;
	int				day;
	int				hour;
	int				minute;
	long			result;
	extern long		time();

#ifdef NEEC_TIMEZONE
	int				timezone = 5*60*60;   /* I'm in the Eastern Time zone GMT + 5 */
#endif

	/* set up the timezone variable */
	time(&result);
	localtime(&result);

	year = buf[0];
	month = buf[1];
	day = buf[2];
	hour = buf[3];
	minute = buf[4];

	/*
	 * check for legal values
	 */

	if ((month < 1) || (month > 12) || (day < 1)
			|| (day > 31) || (minute < 0) || (minute > 59))
		return (0);

	/*
	 * if we're on the 24th hour, add a day
	 */

	if (hour == 24)
		{
		hour = 0;
		day++;
		}

	if ((hour < 0) || (hour > 23))
		return (0);

	/* 
	 * Add all of the days that have passed in the years since 1970
	 */

	year += 1900;
	for (i = 1970, result = 0; i < year; i++)
		result += YEAR_LENGTH(i);

	/*
	 * If it's a leap year, and it's past February,
	 * add one more day
	 */

	if (((year & 3) == 0) && (month >= 3))
		result++;

	/*
	 * Add all of the days that have passed in the months since Jan 1
	 */

	while (--month)
		result += monthLength[month - 1];

	/*
	 * Add all of the days that have passed this month
	 */

	result += day - 1;

	/*
	 * This is how many hours
	 */

	result = (24 * result) + hour;

	/*
	 * This is how many minutes
	 */

	result = (60 * result) + minute;

	/*
	 * Compute seconds
	 */

	result = (result * 60) + timezone;
	if (localtime(&result)->tm_isdst)
		result -= (60 * 60);

	return (result);
	}


char	u2oFmode(flags)
short	flags;
	{
	char			os9Flag = 0;
	register int	i;

	flags &= ~NON_OS9_FLAGS;
	for (i = 0; i < (sizeof modeMap / sizeof(MODE_MAP)); i++)
		if (flags & modeMap[i].unixFlag)
			{
			os9Flag |= modeMap[i].os9Flag;
			flags &= ~modeMap[i].unixFlag;
			}

	return (os9Flag);
	}


short	o2uFmode(flags)
char	flags;
	{
	short			unixFlag = 0;
	register int	i;

	flags &= ~NON_UNIX_FLAGS;
	if (flags & 0x1)
		unixFlag |= 040;

	if (flags & 0x2)
		unixFlag |= 020;

	for (i = 0; i < (sizeof modeMap / sizeof(MODE_MAP)); i++)
		if (flags & modeMap[i].os9Flag)
			{
			unixFlag |= modeMap[i].unixFlag;
			flags &= ~modeMap[i].os9Flag;
			}

	return (unixFlag);
	}
