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
	
	while (*line != '\n')
	{
		line = skipspace(line);
		v->vols[v->vcount++] = getVol(&line);
	}
	
	return 0;
}


int createToc(struct volLine *volArray, int volCount)
{	
	int i, j;
	FILE *fp;
	struct volLine *v = &volArray[0];
	int size = 0;
	
	fp = fopen("tOC", "w+");
	
	if (fp == NULL)
	{
		return -1;
	}

	// Write out header
	fputc(volCount, fp);
	
	size = (volCount * 2);
	
	for (i = 0; i < volCount; i++)
	{
		fputc(0, fp);
		fputc(size, fp);
		size += v->vcount + 2;
		v++;
	}
	
	for (i = 0; i < volCount; i++)
	{
		printf("Disk %d, Side %d, ", volArray->disk, volArray->side);
		fputc(volArray->disk, fp);
		fputc(volArray->side, fp);
		
		for (j = 0; j < volArray->vcount; j++)
		{
			printf("[%d] ", volArray->vols[j]);
			if (j == volArray->vcount - 1)
			{
				volArray->vols[j] += 128;
			}
			fputc(volArray->vols[j], fp);
		}
		printf("\n");
		
		volArray++;
	}
	
	fclose(fp);

	return 0;
}


int main(int argc, char **argv)
{
	FILE *fp;
	char line[256];
	struct volLine volArray[256];
	struct volLine *v = &volArray[0];
	int linecount = 0;
	
	if (argc < 2)
	{
		fprintf(stderr, "Usage: tocgen <filename>\n");
		return 1;
	}
	
	fp = fopen(argv[1], "r");
	
	if (fp == NULL)
	{
		fprintf(stderr, "error opening %s\n", argv[1]);
		return 1;
	}
	

	while (fgets(line, 255, fp) != 0)
	{
		linecount++;
		
		if (parse_line(line, v) == -1)
		{
			fprintf(stderr, "error encountered parsing line %d\n", linecount);
			return 1;
		}
		
		v++;
	}

	fclose(fp);
	
	/* create tOC file */
	if (createToc(volArray, linecount) == -1)
	{
		fprintf(stderr, "tOC not created due to error\n");
	}
	else
	{
		printf("tOC created successfully!\n");
	}

	return 0;
}
