/********************************************************************
 * os9conv.c - Functions to facilitate host<->OS-9 transfers
 *
 * $Id$
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <cococonv.h>
#include <cocopath.h>
#include <cocotypes.h>


char *UnixToOS9Time(time_t currentTime, char *os9time)
{
    struct tm *x;

    x = localtime(&currentTime);
    os9time[0] = x->tm_year;
    os9time[1] = x->tm_mon + 1;
    os9time[2] = x->tm_mday;
    os9time[3] = x->tm_hour;
    os9time[4] = x->tm_min;

    return(os9time);
}


/* Converts a regular string to an OS-9 filename. OS-9 stores filename with the last
 * character in the name as with bit 7 set. This is also used in LSN0 for
 * the disk name.
 */
u_char *StringToOS9Name(u_char *f)
{
    u_char *p;
    int len = strlen((char *)f);

    p = f + len - 1;
    *p |= 0x80;

    return(f);
}


int OS9NameLen(u_char *f)
{
    int count = 0;

    do
    {
        count++;
    }
    while (!(*f++ & 0x80));

    return(count);
}


/* Converts an OS-9 filename to a regular C string. OS-9 stores filename with the last
 * character in the name as with bit 7 set. This is also used in LSN0 for
 * the disk name.
 */
u_char *OS9NameToString(u_char *f)
{
    u_char *p;

    p = f;
    while (*p != '\0')
    {
        if (*p & 0x80)
        {
            *p &= ~0x80;
            *(p + 1) = '\0';
        }
        p++;
    }

    return(f);
}



/* Converts a Disk BASIC filename to a regular C string.
 */
void DECBNameToString(u_char *filename, u_char *ext, u_char *string)
{
	int count = 0;
	
	
	/* 1. Copy filename. */
	
	while (*filename != ' ' && count++ < 8)
	{
		*(string++) = *(filename++);
	}


	/* 2. If an extension exists, add it. */
	
	if (ext[0] != ' ')
	{
		*(string++) = '.';
		
		count = 0;
	
		/* 1. Copy extension. */
	
		while (*ext != ' ' && count++ < 3)
		{
			*(string++) = *(ext++);
		}
	}
	
	
	return;
}



int UnixToCoCoError(int ec)
{
    switch (ec)
    {
		case ENOTDIR:
        case EPERM:
        case EACCES:
            return(EOS_FNA);

        case ENOENT:
            return(EOS_PNNF);

        case EIO:
            return(EOS_PNNF);

        case EBADF:
            return(EOS_BMODE);

        case EEXIST:
            return(EOS_FAE);

        case ENFILE:
        case EMFILE:
            return(EOS_PTHFUL);

        case ENOSPC:
            return(EOS_DF);

        case EROFS:
            return(EOS_BMODE);

        case ENAMETOOLONG:
            return(EOS_BPNAM);

        default:
            return(ec);
    }
}
