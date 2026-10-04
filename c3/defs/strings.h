/*
 * strings.h - BSD-compatibility alias.
 *
 * Some sources (e.g. prep/dues.c) include the historical <strings.h>.  The c3
 * library declares the string routines (including the BSD index()/rindex()) in
 * <string.h>, so just pull that in.
 */
#ifndef _STRINGS_H
#define _STRINGS_H

#include <string.h>

#endif				/* _STRINGS_H */
