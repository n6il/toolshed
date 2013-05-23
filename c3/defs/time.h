/*
 * time.h - OS-9 time definitions
 *
 * $Id: time.h,v 1.3 2005/08/13 15:07:11 boisy Exp $
 *
 * (C) 2005 The C^3 Compiler Project
 * http://www.nitros9.org/c3/
 *
 * Notes:
 *
 * Edt/Rev  YYYY/MM/DD  Modified by
 * Comment
 * ------------------------------------------------------------------
 *          2005/08/12  Boisy G. Pitre
 * Brought in from Carl Kreider's CLIB package.
 */

#ifndef _TIME_H
#define	_TIME_H

/* structure for the 'setime()' and 'getime()' calls */

struct sgtbuf
{
	char            t_year,
	                t_month,
	                t_day,
	                t_hour,
	                t_minute,
	                t_second;
};

/* system dependent value */
#ifdef COCO
#define tps		60	/* ticks per second */
#else
#ifdef LEVEL2
#define tps     100		/* ticks per second */
#else
#define tps     10		/* ticks per second */
#endif
#endif

#endif				/* _TIME_H */
