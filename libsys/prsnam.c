/********************************************************************
 * prsnam.c - CoCo filename parsing
 *
 * $Id$
 ********************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <cocopath.h>
#include <cocotypes.h>
#include <os9module.h>


error_code _os9_prsnam( char *filename )
{
    char	*a = filename;
    int	length = 0;	

    while( *a != 0 )
    {
        length++;
        if (length > 28)
        {
            return(EOS_BPNAM);
        }
        if( isalnum( *a ) )
        {}
        else if ( *a == '_' )
        {}
        else if ( *a == '.' )
        {}
        else if ( *a == '-' )
        {}
        else
            return(EOS_BPNAM);
		
        a++;
    }

    return 0;
}



error_code _decb_prsnam(char *filename)
{
	error_code  ec = 0;


	/* 1. Check if length is > 8.3. */
	
	if (strlen(filename) > 12)
	{
		ec = EOS_BPNAM;
	}
	else
	{
		/* 1. Check if '.' exists, and if so, if extension is > 3 chars. */
		
		char *p = strchr(filename, '.');
		
		if (p != NULL && strlen(p + 1) > 3)
		{
			ec = EOS_BPNAM;
		}
	}
	

    return ec;
}


