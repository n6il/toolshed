/********************************************************************
 * cocotypes.h - CoCoTools type definitions
 *
 * $Id$
 ********************************************************************/
#ifndef	_COCOTYPES_H
#define	_COCOTYPES_H


#if !defined(CYGWIN) && !defined(sun)
#include <stdint.h>
#endif

#include <sys/types.h>

#ifdef  WIN32
//#include <gmon.h>
typedef unsigned char u_char;
typedef unsigned int u_int;
#else
#if !defined(__u_char_defined) && !defined(__APPLE__) && !defined(sun)
typedef unsigned char u_char;
typedef unsigned int u_int;
#endif

#endif
typedef int error_code;


/* necessary byte order conversion prototypes */
unsigned int int4(u_char *a);
unsigned int int3(u_char *a);
unsigned int int2(u_char *a);
unsigned int int1(u_char *a);

void _int4(unsigned int a, u_char *b);
void _int3(unsigned int a, u_char *b);
void _int2(unsigned int a, u_char *b);
void _int1(unsigned int a, u_char *b);

#endif /* _COCOTYPES_H */
