/*###########################################################################
#                                                                           #
#                          OS-9/6809 CROSS ASSEMBLER                        #
#                                                                           #
#                                                                           #
#############################################################################
#                                                                           #
# $Id$
#                                                                           #
#############################################################################
#                                                                           #
# File: output.c                                                            #
# Purpose: symbol table and cross reference output routines                 #
############################################################################*/

#include "mamou.h"

/*
 *  stable --- prints the symbol table in alphabetical order
 */
void stable(struct nlist *ptr)
{
  static int counter = 0;
	if (ptr != NULL)
	{
		stable(ptr->Lnext);
		printf("%-10s %04x", ptr->name, (int)ptr->def);
		counter++;
		if (counter >=4)
		{
			printf("\n");
			counter = 0;
		}
		else
		{
			printf("     ");
		}
		stable(ptr->Rnext);
	}
	return;
}


/*
 *  cross  --  prints the cross reference table 
 */
void cross(struct nlist *point)
{
	struct link *tp;
	int i = 1;

	if (point != NULL)
	{
		cross(point->Lnext);
		printf("%-10s %04x *", point->name, (int)point->def);
		tp = point->L_list;
		while (tp != NULL)
		{
			if (i++ > 10)
			{
				i = 1;
				printf("\n                      ");
			}
			printf("%04d ", (int)tp->L_num);
			tp = tp->next;
		}
		printf("\n");
		cross(point->Rnext);
	}

	return;
}
