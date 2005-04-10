/********************************************************************
 * cococonv.h - CoCo Cross-platform Conversion header file
 *
 * $Id$
 ********************************************************************/
#ifndef	_COCOCONV_H
#define	_COCOCONV_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cocotypes.h>


char *UnixToOS9Time(time_t currentTime, char *os9time);
char *StringToOS9Name(u_char *f);
int OS9NameLen(u_char *f);
char *OS9NameToString(u_char *f);
int UnixToCoCoError(int ec);

void DECBNameToString(u_char *filename, u_char *ext, u_char *string);
void DECBToNative(char *buffer, int size, char **newBuffer, int *newSize);

typedef enum _EOL_Type
{
    EOL_OS9 = 0, EOL_DECB, EOL_UNIX, EOL_DOS
} EOL_Type;

#endif
