
/* Part2 of c source for "rma" */

#include "rma.h"

/*extern direct char *d006a;
extern char *d0681;*/

/* FindCmd - parses one of the CODFORM lists for a match of a
   ccmd
   returns: address of cmd if found,
            NULL if no match
*/

#ifdef __STDC__

CODFORM 
*FindCmd (char *src_str, CODFORM *cod_strt, CODFORM *cod_stop)
#else

FindCmd(src_str,cod_strt,cod_stop)
 char *src_str,
      *cod_strt,
      *cod_stop;
#endif
{
 CODFORM *cod_ptr;
 register char *cmp_str=d0681;

   while ( *src_str != '\0' ) {
      if ( islower(*src_str) ) {
         *cmp_str = *src_str;
      }
      else {
         *cmp_str = ( _tolower(*src_str) );
      }
      ++cmp_str;
      ++src_str;
   }
   *cmp_str = '\0';
   cod_ptr = cod_strt;
   while ( cod_ptr < cod_stop ) {
      cmp_str = cod_ptr->ccmd;
      d006a = d0681;
      while ( *cmp_str ) {
         if ( *cmp_str != *d006a++ ) {
            break;
         }
         ++cmp_str;
      }
      if ( *cmp_str == '\0' ) {
         return cod_ptr;
      }
      ++cod_ptr;
   }
   return 0;
}
