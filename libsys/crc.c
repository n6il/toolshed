/********************************************************************
 * crc.c - OS-9 CRC computation algorithm
 *
 * $Id$
 ********************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <cocopath.h>
#include <cocotypes.h>
#include <os9module.h>


error_code _os9_crc_compute(u_char *ptr, u_int sz)
{
    error_code	ec = 0;
    u_char  crc[3] = {0xff, 0xff, 0xff};
    u_char  a;
    u_int   i;


    for (i = 0; i < sz; i++)
    {
        a = *(ptr++);

        a ^= crc[0];
        crc[0] = crc[1];
        crc[1] = crc[2];
        crc[1] ^= (a >> 7);
        crc[2] = (a << 1);
        crc[1] ^= (a >> 2);
        crc[2] ^= (a << 6);
        a ^= (a << 1);
        a ^= (a << 2);
        a ^= (a << 4);

        if (a & 0x80)
        {
            crc[0] ^= 0x80;
            crc[2] ^= 0x21;
        }
    }
    

    if ((crc[0] == OS9_CRC0) &&
        (crc[1] == OS9_CRC1) &&
        (crc[2] == OS9_CRC2))
    {
        ec = 1;
    }


    return ec;
}



/* Calculate the OS-9/6809 module CRC, returning 0 == !OK, 1 == OK */

error_code _os9_crc(OS9_MODULE_t *mod)
{
    return(_os9_crc_compute((u_char *) mod, INT(mod->size)));
}



/* Calculate the OS-9/68K module CRC, returning 0 == !OK, 1 == OK */

error_code _osk_crc(OSK_MODULE_t *mod)
{
    return(_os9_crc_compute((u_char *) mod, int4(mod->size)));
}



/* Calculate the OS-9/6809 module header parity, returning 0 == OK, !0 == !OK */

u_char _os9_header(OS9_MODULE_t *mod)
{
    u_char tmp = 0x00;
    u_char *ptr = (u_char *)mod;
    int i;
  

    for (i = 0; i < OS9_HEADER_SIZE; i++)
    {
        tmp ^= *(ptr++);
    }

  
    return tmp;
}



/* Calculate the OS-9/68K module header parity, returning 0 == OK, !0 == !OK */

u_short _osk_header(OSK_MODULE_t *mod)
{
    u_short  tmp = 0x0000;
    u_char   *ptr = (u_char *)mod;
    int      i;


    for (i = 0; i < OSK_HEADER_SIZE/2; i++)
    {
        tmp ^= int2(ptr);
        ptr+=2;
    }


    return tmp;
}
