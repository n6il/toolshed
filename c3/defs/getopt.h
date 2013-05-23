/*
 * getopt.h - Option parsing header file
 *
 * $Id: getopt.h,v 1.2 2005/08/13 15:07:11 boisy Exp $
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

#ifndef _GETOPT_H
#define _GETOPT_H

extern int      getopt();	/* Function to get command line options */

extern char    *optarg;		/* Set by GETOPT for options expecting
				 * arguments */
extern int      optind;		/* Set by GETOPT; index of next ARGV to be
				 * processed. */
extern int      opterr;		/* Disable (== 0) or enable (!= 0) error
				 * messages written to standard error */

#define NONOPT  (-1)		/* Non-option - returned by GETOPT when it
				 * encounters a non-option argument. */

#endif				/* _GETOPT_H */
