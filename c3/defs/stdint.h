/*
 * stdint.h - fixed-width integer types for the c3 (MC6809) target.
 *
 * Mapped onto c3's type system, which has no `unsigned char`/`short`/`long`
 * (only `unsigned`, i.e. unsigned int).  Widths on the 6809: char = 8,
 * int = 16, long = 32 bits.  The 8- and 32-bit unsigned types are therefore
 * signed underneath (c3 cannot express them); code that needs true unsigned
 * 8/32-bit semantics must mask explicitly.
 */
#ifndef _STDINT_H
#define _STDINT_H

typedef char            int8_t;
typedef char            uint8_t;        /* no unsigned char in c3 */
typedef int             int16_t;
typedef unsigned        uint16_t;
typedef long            int32_t;
typedef long            uint32_t;       /* no unsigned long in c3 */

#endif
