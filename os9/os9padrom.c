/********************************************************************
 * os9padrom.c - OS-9 ROM padding utility
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <cocopath.h>
#include <cocotypes.h>


#define BUFFSIZ	256

static int do_padrom(char **argv, char *file, int padSize, char padChar);
// static int pow(int x, int p);


/* Help message */
static char *helpMessage[] =
{
    "Syntax: padrom {[<opts>]} <padsize> {<file> [<...>]} {[<opts>]}\n",
    "Usage:  Pad a file to a specific length.\n",
    "Options:\n",
    "     -c[=]<n>    character to pad (n=dec, %bin, 0oct or $hex)\n",
    NULL
};


int os9padrom(int argc, char **argv)
{
    error_code ec = 0;
    char *p = NULL;
    int i;
    int padSize = 0;
    char padChar = '\xff';
    char *file = NULL;


    /* 1. If no arguments, show help and return. */
	
    if (argv[1] == NULL)
    {
        show_help(helpMessage);

        return 0;
    }


    /* 2. Walk command line for options. */
	
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (p = &argv[i][1]; *p != '\0'; p++)
            {
                char *q;


                switch(*p)
                {
                    case 'c':
                        if (*(++p) == '=')
                        {
                            p++;
                        }
                        q = p + strlen(p) - 1;
                        padChar = StrToInt(p);
                        p = q;
                        break;
	
                    case '?':
                    case 'h':
                        show_help(helpMessage);

                        return 0;
	
                    default:
                        fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
                        return 0;
                }
            }
        }
    }


    /* 3. Walk command line for pathnames. */
	
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            continue;
        }
        else
        {
            if (padSize == 0)
            {
                padSize = StrToInt(argv[i]);
                if (padSize <= 0)
                {
                    fprintf(stderr, "%s: illegal pad size\n", argv[0]);

                    return 1;
                }
            }
            else
            {
                file = argv[i];
                do_padrom(argv, file, padSize, padChar);
            }
        }
    }


    return ec;
}



static int do_padrom(char **argv, char *file, int padSize, char padChar)
{
    error_code	ec = 0;
    coco_path_id path;
    int j;
    u_int fileSize;


    ec = _coco_open(&path, file, FAM_WRITE);

    if (ec != 0)
    {
        fprintf(stderr, "%s: failed to open '%s'\n", argv[0], file);

        return ec;
    }


    ec = _coco_gs_size(path, &fileSize);

    if (ec != 0)
    {
        fprintf(stderr, "%s: cannot get file size of '%s'\n", argv[0], file);
        _coco_close(path);

        return ec;
    }

    if (padSize <= fileSize)
    {
        fprintf(stderr, "%s: padrom size insufficient\n", argv[0]);
        _coco_close(path);

        return 1;
    }

    _coco_seek(path, fileSize, SEEK_SET);

    for (j = 0; j < padSize - fileSize; j++)
    {
        int size = 1;

        _coco_write(path, &padChar, &size);
    }

    _coco_close(path);


    return 0;
}
