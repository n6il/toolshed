/********************************************************************
 * os9attr.c - File attribute utility for OS-9 filesystems
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cocotypes.h>
#include <os9path.h>


static int do_getattr(char **argv, char *p);
static int do_setattr(char **argv, char *p, int attrSetMask, int attrResetMask, int quiet);

/* Help message */
static char *helpMessage[] =
{
    "Syntax: attr {[<opts>]} {<file> [<...>]} {[<opts>]}\n",
    "Usage:  Display or modify file attributes.\n",
    "Options:\n",
    "     -q        quiet mode (suppress output)\n",
    "     -e        set execute permission\n",
    "     -w        set write permission\n",
    "     -r        set read permission\n",
    "     -s        set single user permission\n",
    "     -p[ewr]   set public permission\n",
    "     -n[ewrs]  unset owner permissions\n",
    "     -np[ewr]  unset public permissions\n",
    NULL
};


int os9attr(int argc, char *argv[])
{
    char *p = NULL;
    int i;
    int attrSetMask = 0;
    int attrResetMask = 0;
    int quiet = 0;

    /* walk command line for options */
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            for (p = &argv[i][1]; *p != '\0'; p++)
            {
                switch (*p)
                {
                    case 'q':
                        quiet = 1;
                        break;
                    case 'e':
                        attrSetMask |= FAP_EXEC;
                        break;
                    case 'w':
                        attrSetMask |= FAP_WRITE;
                        break;
                    case 'r':
                        attrSetMask |= FAP_READ;
                        break;
                    case 'n':
                        switch (*(p + 1))
                        {
                            case 'p':
                                switch (*(p + 2))
                                {
                                    case 'e':
                                        attrResetMask |= FAP_PEXEC;
                                        break;
                                    case 'w':
                                        attrResetMask |= FAP_PWRITE;
                                        break;
                                    case 'r':
                                        attrResetMask |= FAP_PREAD;
                                        break;
                                    }
                                    break;
                            case 'e':
                                attrResetMask |= FAP_EXEC;
                                break;
                            case 'w':
                                attrResetMask |= FAP_WRITE;
                                break;
                            case 'r':
                                attrResetMask |= FAP_READ;
                                break;
                            case 's':
                                attrResetMask |= FAP_SINGLE;
                                break;
                            case 'd':
                                attrResetMask |= FAP_DIR;
                                break;
                        }
                        break;
                    case 'p':
                        switch (*(p + 1))
                        {
                            case 'e':
                                attrSetMask |= FAP_PEXEC;
                                break;
                            case 'w':
                                attrSetMask |= FAP_PWRITE;
                                break;
                            case 'r':
                                attrSetMask |= FAP_PREAD;
                                break;
                        }
                        break;
                    case 's':
                        attrSetMask |= FAP_SINGLE;
                        break;
                    case 'h':
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
            
            if (attrSetMask == 0 && attrResetMask == 0)
            {
                /* No attribute options were specified */
                do_getattr(argv, p);
            }
            else
            {
                /* Attributes were specified */
                do_setattr(argv, p, attrSetMask, attrResetMask, quiet);
            }
            if (quiet == 0)
            {
                printf("\n");
            }
        }
    }

    if (p == NULL)
    {
        show_help(helpMessage);
        return(0);
    }

    return(0);
}
	

static int do_getattr(char **argv, char *p)
{
    error_code	ec = 0;
    os9_path_id path;
	
    /* open a path to the device */
    ec = _os9_open(&path, p, FAM_READ);
    if (ec != 0)
    {
        fprintf(stderr, "%s: error %d opening '%s'\n", argv[0], ec, p);
        return(ec);
    }
	
    {
        fd_stats fdbuf;
        int size = sizeof(fdbuf);

        _os9_gs_fd(path, size, &fdbuf);

        show_attrs(fdbuf.fd_att);
    }

    _os9_close(path);

    return(0);
}


static int do_setattr(char **argv, char *p, int attrSetMask, int attrResetMask, int quiet)
{
    error_code	ec = 0;
    os9_path_id path;
	
    /* open a path to the device */
    ec = _os9_open(&path, p, FAM_WRITE);
    if (ec != 0)
    {
        fprintf(stderr, "%s: error %d opening '%s'\n", argv[0], ec, p);
        return(ec);
    }
	
    {
        fd_stats fdbuf;
        int size = sizeof(fdbuf);

        ec = _os9_gs_fd(path, size, &fdbuf);
        
        if (attrSetMask != 0)
        {
            fdbuf.fd_att |= attrSetMask;
        }
        if (attrResetMask != 0)
        {
            fdbuf.fd_att &= ~attrResetMask;
        }
        
        ec = _os9_ss_fd(path, size, &fdbuf);

        if (quiet == 0)
        {
            show_attrs(fdbuf.fd_att);
        }
    }

    _os9_close(path);

    return(0);
}
