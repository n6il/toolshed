/*
** dump count bytes, starting at addr, after printing string
*/

#include <stdio.h>

_dump(string, addr, count, fp)
char  *string, *addr;
int   count;
FILE *fp;
   {
   register char  *p, c;
   int   j;
   unsigned u;

   fprintf(fp, "%s\n\n", string);
   fprintf(fp, "      ");

   for (j = 0, p = addr; j < 16; j++, p++)
      fprintf(fp, " %1x ", (u = p) & 0x000f);

   fprintf(fp, " ");

   for (j = 0, p = addr; j < 16; j++, p++)
      fprintf(fp, "%1x", (u = p) & 0x000f);

   putc('\n', fp);
   fprintf(fp,
      "      -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --  ----------------\n");

   for ( ; count > 0; count -= 16)
      {
      fprintf(fp, "%04x: ", addr);

      for (j = 0, p = addr; j < 16; j++)
      {
      	if( count-j > 0 )
         	fprintf(fp, "%02x ", (*p++ & 0xff));
        else
        	fprintf(fp, "   " );
      }

      fprintf(fp, " ");

      for (j = 0, p = addr; j < 16; j++)
      {
      	if( count-j > 0 )
      		fprintf(fp, "%c", ((c = *p++ & 0x7f) >= 32 ? c : '.'));
      	else
      		fprintf(fp, " " );
      }

      putc('\n', fp);
      addr = p;     /* update memory pointer */
      }
   putc('\n', fp);
   }
