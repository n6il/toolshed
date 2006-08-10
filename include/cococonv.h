/********************************************************************
 * cococonv.h - CoCo Cross-platform Conversion header file
 *
 * $Id$
 ********************************************************************/
#ifndef	_COCOCONV_H
#define	_COCOCONV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cocotypes.h>

char *UnixToOS9Time(time_t currentTime, char *os9time);
u_char *CStringToOS9String(u_char *f);
int OS9Strlen(u_char *f);
u_char *OS9StringToCString(u_char *f);
int UnixToCoCoError(int ec);
void NativeToDECB(char *buffer, int size, char **newBuffer, u_int *newSize);
void DECBToNative(char *buffer, int size, char **newBuffer, u_int *newSize);
void CStringToDECBString(u_char *filename, u_char *ext, u_char *string);
void DECBStringToCString(u_char *filename, u_char *ext, u_char *string);
void OS9AttrToString(int attr_byte, char *string);

typedef enum _EOL_Type
{
	EOL_OS9 = 0, EOL_DECB, EOL_UNIX, EOL_DOS
} EOL_Type;

#ifdef __cplusplus
}
#endif

#endif

