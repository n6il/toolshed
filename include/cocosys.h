/********************************************************************
 * cocosys.h - CoCo system calls
 *
 * $Id$
 ********************************************************************/
#ifndef	_COCOSYS_H
#define	_COCOSYS_H

#include <cocotypes.h>

error_code _os9_allocate_bits(void *, int, int);
error_code _os9_delete_bits(void *, int, int);
error_code _os9_prsnam(char *name);

error_code _decb_prsnam(char *name);

#endif /* _COCOSYS_H */
