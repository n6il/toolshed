#include "cp.h"


int strcmp2(str1, str2)
char *str1, *str2;
{
	int l1 = strlen(str1);
	int l2 = strlen(str2);
	
	if (strncmp(str1, str2, min(l1, l2)) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
