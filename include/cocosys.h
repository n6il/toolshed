/********************************************************************
 * cocosys.h - CoCo system calls
 *
 * $Id$
 ********************************************************************/
#ifndef	_COCOSYS_H
#define	_COCOSYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cocotypes.h>

error_code _os9_allocate_bits(void *, int, int);
error_code _os9_delete_bits(void *, int, int);
error_code _os9_prsnam(char *name);
error_code _decb_prsnam(char *name);
error_code _cecb_prsnam( char *filename );

#ifdef __cplusplus
}
#endif

#endif /* _COCOSYS_H */
