/********************************************************************
 * $Id$
 *
 * Functions to facilitate line-ending, error number, string and
 * time conversions between platforms.
 *
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <cococonv.h>
#include <cocopath.h>
#include <cocotypes.h>

static EOL_Type DetermineEOLType(char *buffer, int size);


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
u_char *CStringToOS9String(u_char *f)
{
    u_char *p;
    int len = strlen((char *)f);

	if (len > 0)
	{
		p = f + len - 1;
		*p |= 0x80;
	}

	return(f);
}


int OS9Strlen(u_char *f)
{
	int count = 0;

	do
	{
		count++;
	}
	while (!(*f++ & 0x80));

	return(count);
}


/* Converts an OS-9 string to a regular C string.
 * OS-9 stores filename with the last character in the name as with bit 7
 * set. This is also used in LSN0 for the disk name.
 */
u_char *OS9StringToCString(u_char *f)
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


/* Converts a C string to a Disk BASIC filename padded with spaces.
   NOTE: We presume that the passed string is in 8.3 format
 */
void CStringToDECBString(u_char *filename, u_char *ext, u_char *string)
{
	unsigned char *fp, *fpp;

	memset(filename, 32, 8);
	memset(ext, 32, 3);

	fp = string;
	fpp = filename;
	while (*fp != '.' && *fp != '\0')
	{
		*fpp = *fp;
		fp++;
		fpp++;
	}

	if (*fp == '.')
	{
		fpp = ext;
		fp++;
		while (*fp != '\0')
		{
			*fpp = *fp;
			fp++;
			fpp++;
		}
	}
}


/* Converts a Disk BASIC filename to a regular C string.
 */
void DECBStringToCString(u_char *filename, u_char *ext, u_char *string)
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

	*(string) = '\0';
	
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



/*
 * Scan a buffer to determine the type of end-of-line termination it has.
 *
 * Returns EOL_DOS, EOL_UNIX or EOL_OS9
 */
static EOL_Type DetermineEOLType(char *buffer, int size)
{
    EOL_Type eol = 0;
    int i;
    
    
    /* Scan to determine EOL ending type */
    
    for (i = 0; i < size; i++)
    {
        if (i < size - 1 && (buffer[i] == 0x0D && buffer[i + 1] == 0x0A))
        {
            /* We have DOS/Windows line endings (0D0A)... */
            eol = EOL_DOS;

            break;
        }

        if (buffer[i] == 0x0A)
        {
            /* We have unix line endings. */
            eol = EOL_UNIX;

            break;
        }
        
        if (buffer[i] == 0x0D)
        {
            /* We have OS-9 line endings. */
            eol = EOL_OS9;
            
            break;
        }
    }


    return eol;
}



/*
 * Converts a buffer containing native EOLs to one with Disk BASIC EOLs.
 *
 * The caller must free the returned buffer in 'newBuffer' once
 * finished with the buffer.
 */
void NativeToDECB(char *buffer, int size, char **newBuffer, u_int *newSize)
{
    EOL_Type	eolMethod;
    int		i;


    eolMethod = DetermineEOLType(buffer, size);
    
    switch (eolMethod)
    {
        case EOL_UNIX:
            /* Change all occurences of 0x0A to 0x0D */
            
            for(i = 0; i < size; i++)
            {
                if (buffer[i] == 0x0A)
                {
                    buffer[i] = 0x0D;
                }
            }
            *newBuffer = (char *)malloc(size);
            if (*newBuffer == NULL)
            {
                return;
            }
            
            memcpy(*newBuffer, buffer, size);
            
            *newSize = size;
            
            break;

        case EOL_DOS:
            /* Things are a bit more involved here. */
            
            /* We will strip all 0x0As out of the buffer, leaving the 0x0Ds. */
            
            {
                int dosEOLCount = 0;
                char *newP;
                int i;


                /* 1. First we count up the number of 0x0A line endings. */

                for (i = 0; i < size; i++)
                {
                    if (buffer[i] == 0x0A)
                    {
                        dosEOLCount++;
                    }
                }


                /* 2. Now we allocate a buffer to hold the current size -
                    'dosEOLCount' bytes.
                */

                *newSize = size - dosEOLCount;
                
                *newBuffer = (char *)malloc(*newSize);
                
                if (*newBuffer == NULL)
                {
                    return;
                }

                newP = *newBuffer;
                
                for (i = 0; i < size; i++)
                {
                    if (buffer[i] != 0x0A)
                    {
                        *newP = buffer[i];
                        newP++;
                    }
                }
            }
            break;

        default:
            return;
    }

    
    return;
}


void DECBToNative(char *buffer, int size, char **newBuffer, u_int *newSize)
{
#ifdef _WIN32
    int dosEOLCount = 0;
    char *newP;
    int		i;


    /* Things are a bit more involved here. */
            
    /* We will add 0x0As after all 0x0Ds. */
            

    /* 1. First we count up the number of 0x0D Disk BASIC line endings. */

    for (i = 0; i < size; i++)
    {
        if (buffer[i] == 0x0D)
        {
            dosEOLCount++;
        }
    }


    /* 2. Now we allocate a buffer to hold the current size +
        'dosEOLCount' bytes.
    */
    
    *newSize = size + dosEOLCount;
    *newBuffer = (char *)malloc(*newSize);
                
    if (*newBuffer == NULL)
    {
        return;
    }

    newP = *newBuffer;
    
    for (i = 0; i < size; i++)
    {
        *newP = buffer[i];
        newP++;

        if (buffer[i] == 0x0D)
        {
            *newP = 0x0A;
            newP++;
        }
    }
#else
    int		i;


    /* Change all occurences of 0x0D to 0x0A */
            
    for(i = 0; i < size; i++)
    {
        if (buffer[i] == 0x0D)
        {
            buffer[i] = 0x0A;
        }
    }

    *newBuffer = (char *)malloc(size);
    if (*newBuffer == NULL)
    {
        return;
    }
            
    memcpy(*newBuffer, buffer, size);
            
    *newSize = size;
#endif


	return;
}

/*
 * OS9AttrToString()
 *
 * Returns textual representation of file attributes to standard output
 */
void OS9AttrToString(int attr_byte, char *string)
{
	int i;
	char *attrs = "dsewrewr";

	/* print attributes */
	for (i = 0; i < 8; i++)
	{
		if (attr_byte & (1 << (7- i)))
		{
			string[i] = attrs[i];
		}
		else
		{
			string[i] = '-';
		}
	}
}
