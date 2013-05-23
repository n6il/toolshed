/*
 * signal.h - OS-9 signal definitions
 *
 * $Id: signal.h,v 1.3 2005/08/13 15:07:11 boisy Exp $
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

#ifndef _SIGNAL_H
#define	_SIGNAL_H

/* OS-9 signals */
#define        SIGKILL 0	/* sytem abort (cannot be caught or ignored) */
#define        SIGWAKE 1	/* wake up */
#define        SIGQUIT 2	/* keyboard abort */
#define        SIGINT  3	/* keyboard interrupt */

/* special addresses */
#define        SIG_DFL 0
#define        SIG_IGN 1

#endif				/* _SIGNAL_H */
