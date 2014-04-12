/********************************************************************
 * os9dump.c - OS-9 dump utility
 *
 * $Id$
 ********************************************************************/

#include "util.h"
#include "cocopath.h"
#include "cocotypes.h"
#include "cococonv.h"


#define BUFFSIZ	256

static u_int dumpchunk;

static void dump(u_char *buffer, size_t num_bytes, int format);
static void dump_line(u_char *buffer, int count, int format);
static int do_dump(char **argv, char *file, int format);
static void dump_header(int format);
static char *binary(char s);

/* Help message */
static char const * const helpMessage[] =
{
    "Syntax: dump {[<opts>]} {<file> [<...>]} {[<opts>]}\n",
    "Usage:  Display the contents of a file in hexadecimal.\n",
    "Options:\n",
    "     -a    dump output in assembler format (hex)\n",
    "     -b    dump output in assembler format (binary)\n",
    "     -c    don't display ASCII character data\n",
    "     -h    don't display header\n",
    "     -l    don't display line label/count\n",
    NULL
};


static int assemblerFormat;
static int displayASCII;
static int displayHeader;
static int displayLabel;


int os9dump(int argc, char **argv)
{
    error_code	ec = 0;
    char *p = NULL;
    int i;

    assemblerFormat = 0;
    displayASCII = 1;
    displayHeader = 1;
    displayLabel = 1;
    dumpchunk = 16;

    if (argv[1] == NULL)
    {
            show_help(helpMessage);
            return(0);
    }

    /* walk command line for options */
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (p = &argv[i][1]; *p != '\0'; p++)
            {
                switch(*p)
                {
                    case 'a':
                        assemblerFormat = 1;
                        dumpchunk = 8;
                        break;

                    case 'b':
                        assemblerFormat = 2;
                        dumpchunk = 1;
                        break;

                    case 'c':
                        displayASCII = 0;
                        break;
                        
                    case 'h':
                        displayHeader = 0;
                        break;
                        
                    case 'l':
                        displayLabel = 0;
                        break;
                        
                    case '?':
                        show_help(helpMessage);
                        return(0);
	
                    default:
                        fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
                        return(0);
                }
            }
        }
    }

    /* walk command line for pathnames */
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            continue;
        }
        else
        {
            p = argv[i];
        }
        
        ec = do_dump(argv, p, assemblerFormat);

        if (ec != 0)
        {
            fprintf(stderr, "%s: error %d opening '%s'\n", argv[0], ec, p);
            return(ec);
        }
    }

    return(0);
}


static int byte_count;

static int do_dump(char **argv, char *file, int format)
{
    error_code	ec = 0;
    u_char buffer[BUFFSIZ];
    coco_path_id path;


    byte_count = 0;

    /* 1. Open a path to the file. */
	
    ec = _coco_open(&path, file, FAM_READ);

    if (ec != 0)
    {
        ec = _coco_open(&path, file, FAM_DIR | FAM_READ);

        if (ec != 0)
        {
            fprintf(stderr, "%s: cannot open file\n", argv[0]);
            return(ec);
        }
    }

    while (1)
    {
        u_int num_bytes = BUFFSIZ;

        ec = _coco_read(path, buffer, &num_bytes);
        if (ec != 0)
        {
            break;
        }
        dump(buffer, num_bytes, format);
    }

    printf("\n");

    ec = _coco_close(path);


    return ec;
}


static void dump(u_char *buffer, size_t num_bytes, int format)
{
    u_int i;

    for (i = 0; i < num_bytes; i += dumpchunk)
    {
        if (byte_count % BUFFSIZ == 0)
        {
            dump_header(format);
        }

        /* print line header */
        if (format == 0)
        {
            if (displayLabel == 1)
            {
                printf("\n%08x  ", byte_count);
            }
            else
            {
                printf("\n");
            }
        }
        else
        {
            if (displayLabel == 1)
            {
                printf("\nL%04X    fcb   ", byte_count);
            }
            else
            {
                printf("\n         fcb   ");
            }
        }
        if (num_bytes - i > dumpchunk)
        {
            dump_line(&buffer[i], dumpchunk, format);
            byte_count += dumpchunk;
        }
        else
        {
            dump_line(&buffer[i], num_bytes - i, format);
            byte_count += num_bytes - i;
        }
    }

    return;
}


static void dump_line(u_char *buffer, int count, int format)
{
    int i;
    int carry = 0;

    if (count % 2 != 0)
    {
        count--;
        carry = 1;
    }
	
    for (i = 0; i < count; i += 2)
    {
        switch (format)
        {
            case 0:
                printf("%02x%02x ", buffer[i], buffer[i + 1]);
                break;

            case 1:
                if (i == count - 2 && carry == 0)
                {
                    printf("$%02X,$%02X", buffer[i], buffer[i + 1]);
                }
                else
                {
                    printf("$%02X,$%02X,", buffer[i], buffer[i + 1]);
                }
                break;
                
            case 2:
                if (i == count - 2 && carry == 0)
                {
                    printf("%%%s,%%%s", binary(buffer[i]), binary(buffer[i + 1]));
                }
                else
                {
                    printf("%%%s,%%%s,", binary(buffer[i]), binary(buffer[i + 1]));
                }
                break;
        }
    }

    if (carry == 1)
    {
        switch (format)
        {
            case 0:
                printf("%02x", buffer[i]);
                break;

            case 1:
                printf("$%02X", buffer[i]);
                break;

            case 2:
                printf("%%%s", binary(buffer[i]));
                break;
        }
        count++;
    }

    if (displayASCII == 1)
    {
        /* make spaces available if last line is not full */
        i = (dumpchunk - count);

        if (format == 1)
        {
            printf("   ");
        }

        if (i % 2 != 0)
        {
            switch (format)
            {
                case 1:
                    printf("     ");
                    break;
                    
                default:
                    printf("   ");
                    break;
            }
        }
    
        i /= 2;
    
        while (i--)
        {
            if (format == 1)
            {
                printf("        ");
            }
            else
            {
                printf("     ");
            }
        }

        /* print character dump on right side */
        for (i = 0; i < count; i++)
        {
            if (buffer[i] >= 32 && buffer[i] <= 127)
            {
                printf("%c", buffer[i]);
            }
            else if (buffer[i] >= 128+32 && buffer[i] <= 128+'z')
            {
                printf("%c", buffer[i]-128);
            }
            else
            {
                printf(".");
            }
        }
    }

    return;
}


static void dump_header(int format)
{
    if (format == 0 && displayHeader == 1)
    {
        printf("\n\n  Addr     0 1  2 3  4 5  6 7  8 9  A B  C D  E F");
        if (displayASCII == 1)
        {
            printf(" 0 2 4 6 8 A C E");
        }
        printf("\n");
        
        printf("--------  ---- ---- ---- ---- ---- ---- ---- ----");
        if (displayASCII == 1)
        {
            printf(" ----------------");
        }
    }

    return;
}


static char *binary(char s)
{
    static char buffer[9] = {'0', '0', '0', '0', '0', '0', '0', '0', '\0'};
    int i;
    
    
    for (i = 0; i < 8; i++)
    {
        int x = s & (1 << (7 - i));
        
        if (x != 0)
        {
            buffer[i] = '1';
        }
        else
        {
            buffer[i] = '0';
        }
    }
    

    return(buffer);
}
