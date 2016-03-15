/********************************************************************
 * $Id$
 *
 * Parses the passed filename to see if it is legal.
 ********************************************************************/
#include <ctype.h>
#include <string.h>
#include <sys/types.h>

#include <cocopath.h>
#include <cocotypes.h>
#include <os9module.h>


error_code _os9_prsnam(char *filename)
{
    char  *a = filename;
    int	  length = 0;	

    while (*a != 0)
    {
        length++;
        if (length > D_NAMELEN)
        {
            return(EOS_BPNAM);
        }
        if (! (isalnum(*a) || *a == '_' || *a == '.' || *a == '-' || *a == ' ' || *a == '$') )
	{
            return EOS_BPNAM;
	}
        a++;
    }

    return 0;
}



error_code _decb_prsnam(char *filename)
{
	error_code  ec = 0;
	char *dot;

	/* 1. Check if filename contains a dot */
	dot = strchr(filename, '.');

	if (dot != NULL)
	{
		*dot = 0;
		if (strlen(filename) > 8 || strlen(dot + 1) > 3)
		{
			ec = EOS_BPNAM;
		}

		*dot = '.';
	}
	else
	{
		/* Filename with no extension - just check filename length */
		if (strlen(filename) > 8)
		{
			ec = EOS_BPNAM;
		}
	}

	return ec;
}

error_code _cecb_prsnam( char *filename )
{
	error_code	ec = 0;
	int i;
	
	if( strlen(filename) > 8 )
	{
		ec = EOS_BPNAM;
	}
	else
	{
		for( i=0; i<strlen(filename); i++ )
		{
			if( isgraph( filename[i] ) != 0 )
				ec = EOS_BPNAM;
		}
	}
	
	return ec;
}

