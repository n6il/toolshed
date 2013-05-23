/*
 * types.h - Portable scalar type definitions
 *
 * $Id: types.h,v 1.2 2005/08/14 16:41:36 boisy Exp $
 *
 * (C) 2005 The C^3 Compiler Project
 * http://www.nitros9.org/c3/
 *
 * Notes:
 *
 * Edt/Rev  YYYY/MM/DD  Modified by
 * Comment
 * ------------------------------------------------------------------
 *          2005/08/14  Boisy G. Pitre
 * Created.
 */

#ifndef _TYPES_H
#define _TYPES_H

#define _CHAR8_T
typedef	char				char8_t;
typedef	unsigned char		u_char_t;
typedef	unsigned char		uchar8_t;

#define _INT8_T
typedef	char				int8_t;
typedef	unsigned char		u_int8_t;
typedef	unsigned char		uint8_t;

#define _INT16_T
typedef	int					int16_t;
typedef	unsigned short		u_int16_t;
typedef	unsigned short		uint16_t;

#define _INT32_T
typedef	long				int32_t;
typedef	unsigned long		u_int32_t;
typedef	unsigned long		uint32_t;

#endif				/* _TYPES_H */
