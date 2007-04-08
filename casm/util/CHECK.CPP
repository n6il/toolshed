/*
	Check binary record
*/
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	unsigned char 		type;
	unsigned short		size;
	unsigned short		addr;
} BinRec;

void GetRec(FILE *file, BinRec *rec)
{
	rec->type = fgetc(file);
	rec->size = (fgetc(file)*256)+fgetc(file);
	rec->addr = (fgetc(file)*256)+fgetc(file);
}


int main(int argc, char *argv[])
{
#pragma argsused
	FILE *file;
	BinRec rec;
	int flag = 0;
	int loc = 0;

	if(argv[1] == NULL)
		{
		printf("Usage: %s filename\n", argv[0]);
		return -1;
		}
	if((file = fopen(argv[1], "rb")) == NULL)
		{
		printf("Cannot open file %s\n", argv[1]);
		return -1;
		}

	while(flag == 0)
		{
      fseek(file, loc, SEEK_SET);
		GetRec(file, &rec);
		flag = rec.type;
		printf("Loc: %i  Type: %02X Address: %04X  Size: %04X\n",
					loc, rec.type, rec.addr, rec.size);
		loc+=(rec.size)+5;

		}
	fclose(file);

	return (0);
}
