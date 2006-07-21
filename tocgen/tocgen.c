/*
 *  tocgen.c
 *  
 *
 *  Created by Boisy Pitre on 7/21/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cocopath.h>


struct volLine
{
	int disk;
	int side;
	int vcount;
	int vols[255];
};


int get_num(char **line)
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


char *skipspace(char *line)
{
	while (isspace(*line)) line++;
	
	return line;
}


int getVol(char **line)
{
	if (tolower(**line) != 'v')
	{
		return -1;
	}
	(*line)++;

	return get_num(line);
}


int parse_line(char *line, struct volLine *v)
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


int createToc(char *outfile, struct volLine *volArray, int volCount)
{	
	int i, j;
	coco_path_id fp;
	struct volLine *v = &volArray[0];
	int size = 0, ec;
	unsigned int writesize = 1;
	char c;
	
	ec = _coco_create(&fp, outfile, FAM_WRITE, FAP_PREAD | FAP_READ | FAP_WRITE);
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
		printf("Disk %d, Side %d, ", volArray->disk, volArray->side);
		c = volArray->disk;
		_coco_write(fp, &c, &writesize);
		c = volArray->side;
		_coco_write(fp, &c, &writesize);
		
		for (j = 0; j < volArray->vcount; j++)
		{
			printf("[%d] ", volArray->vols[j]);
			if (j == volArray->vcount - 1)
			{
				volArray->vols[j] += 128;
			}
			c = volArray->vols[j];
			_coco_write(fp, &c, &writesize);
		}
		printf("\n");
		
		volArray++;
	}
	
	_coco_close(fp);

	return 0;
}


int main(int argc, char **argv)
{
    coco_path_id fp;
	char line[256];
	struct volLine volArray[256];
	struct volLine *v = &volArray[0];
	int linecount = 0, ec;
	unsigned int size;
	
	if (argc < 3)
	{
		fprintf(stderr, "Usage: tocgen <infile> <outfile>\n");
		return 1;
	}
	
	ec = _coco_open(&fp, argv[1], FAM_READ);
	
	if (ec != 0)
	{
		fprintf(stderr, "error opening %s\n", argv[1]);
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
	if (createToc(argv[2], volArray, linecount) == -1)
	{
		fprintf(stderr, "tOC not created due to error\n");
	}
	else
	{
		printf("tOC created successfully!\n");
	}

	return 0;
}
