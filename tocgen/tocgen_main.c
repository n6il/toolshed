/********************************************************************
 * tocgen.c - Sierra table of contents generator
 *
 * $Id$
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cocopath.h>
#include <util.h>


static int do_tocgen(char *infile, char *outfile, int quiet);

/* Help message */
static char *helpMessage[] =
{
    "Syntax: tocgen {[<opts>]} {<infile>} {[<outfile>]} {[<opts>]}\n",
    "Usage:  Table of contents generator for Sierra AGI games.\n",
    "Options:\n",
    "     -q        quiet mode (suppress output)\n",
    NULL
};


int main(int argc, char *argv[])
{
    char *infile = NULL, *outfile = NULL, *p;
    int i;
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
			if (infile == NULL)
			{
				infile = argv[i];
			}
			else if (outfile == NULL)
			{
				outfile = argv[i];
			}
			else
			{
				show_help(helpMessage);
				return(0);
			}


        }
    }

    if (infile == NULL)
    {
        show_help(helpMessage);
        return(0);
    }
	
	if (outfile == NULL)
	{
		outfile = "tOC";
	}

	return do_tocgen(infile, outfile, quiet);

}


struct volLine
{
	int disk;
	int side;
	int vcount;
	int vols[255];
};


static int get_num(char **line)
{
	char tchar;
	int num;
	char *numptr;
	
	numptr = *line;
	while (isdigit(**line))
	{
		(*line)++;
	}
	tchar = **line;
	**line = '\0';
	num = atoi(numptr);
	**line = tchar;
	
	return num;
}


static char *skipspace(char *line)
{
	while (isspace(*line)) line++;
	
	return line;
}


static int getVol(char **line)
{
	if (tolower(**line) != 'v')
	{
		return -1;
	}
	(*line)++;

	return get_num(line);
}


static int parse_line(char *line, struct volLine *v)
{
	v->vcount = 0;
	
	if (tolower(*line) != 'd')
	{
		return -1;
	}
	line++;
	v->disk = get_num(&line);
	line = skipspace(line);
	
	if (tolower(*line) != 's')
	{
		return -1;
	}
	line++;
	v->side = get_num(&line);
	
	while (*line != '\n' && *line != '\r')
	{
		line = skipspace(line);
		v->vols[v->vcount++] = getVol(&line);
	}
	
	return 0;
}


static int createToc(char *outfile, struct volLine *volArray, int volCount, int quiet)
{	
	int i, j;
	coco_path_id fp;
	struct volLine *v = &volArray[0];
	int size = 0, ec;
	unsigned int writesize = 1;
	char c;
	coco_file_stat fstat;
	
	fstat.perms = FAP_PREAD | FAP_READ | FAP_WRITE;
	ec = _coco_create(&fp, outfile, FAM_WRITE, &fstat);
	if (ec != 0)
	{
		return -1;
	}

	// Write out header
	c = volCount;
	_coco_write(fp, &c, &writesize);
	
	size = (volCount * 2);
	
	for (i = 0; i < volCount; i++)
	{
		c = '\0';
		_coco_write(fp, &c, &writesize);
		c = size;
		_coco_write(fp, &c, &writesize);
		size += v->vcount + 2;
		v++;
	}
	
	for (i = 0; i < volCount; i++)
	{
		if (quiet == 0)
		{
			printf("Disk %d, Side %d, ", volArray->disk, volArray->side);
		}
		
		c = volArray->disk;
		_coco_write(fp, &c, &writesize);
		c = volArray->side;
		_coco_write(fp, &c, &writesize);
		
		for (j = 0; j < volArray->vcount; j++)
		{
			if (quiet == 0)
			{
				printf("[%d] ", volArray->vols[j]);
			}
			
			if (j == volArray->vcount - 1)
			{
				volArray->vols[j] += 128;
			}
			c = volArray->vols[j];
			_coco_write(fp, &c, &writesize);
		}
		if (quiet == 0)
		{
			printf("\n");
		}

		volArray++;
	}
	
	_coco_close(fp);

	return 0;
}


static int do_tocgen(char *infile, char *outfile, int quiet)
{
    coco_path_id fp;
	char line[256];
	struct volLine volArray[256];
	struct volLine *v = &volArray[0];
	int linecount = 0, ec;
	unsigned int size;
	
	ec = _coco_open(&fp, infile, FAM_READ);
	
	if (ec != 0)
	{
		fprintf(stderr, "error opening %s\n", infile);
		return 1;
	}


	size = 256;
	while (_coco_readln(fp, line, &size) == 0)
	{
		size = 256;
		linecount++;
		
		if (parse_line(line, v) == -1)
		{
			fprintf(stderr, "error encountered parsing line %d\n", linecount);
			return 1;
		}
		
		v++;
	}

	_coco_close(fp);

	
	/* create tOC file */
	if (createToc(outfile, volArray, linecount, quiet) == -1)
	{
		fprintf(stderr, "tOC not created due to error\n");
	}
	else if (quiet == 0)
	{
		printf("tOC created successfully!\n");
	}

	return 0;
}
