/* ctest.c - runtime test for the c3 lwasm/lwlink OS-9 backend's init-data
 * support (crt0 image copy + pointer relocation + bss zeroing).
 *
 * Built and run on NitrOS-9 (USim, Pico-Thing L2) it prints five lines, one
 * per init-data path; any missing/garbled line points at the broken path:
 *
 *     arr ok      initialised char array         -> copied from the image
 *     arr ok      char *parr = arr;              -> data-pointer reloc (+Y)
 *     lit ok      char *plit = "lit ok\n";       -> code-pointer reloc (+mbase)
 *     tab ok      int tab[3] = { ... };          -> initialised int array
 *     zero ok     int zero;  (uninitialised)     -> crt0 zeroes the data area
 *
 * Needs putstr.s (OS-9 I$Write helper).  See README.md / mktestdisk.sh.
 */
char  arr[] = "arr ok\n";       /* initialised char array  -> image copy   */
char *parr  = arr;              /* pointer into data area  -> ireloc  (+Y) */
char *plit  = "lit ok\n";       /* pointer into module body-> icreloc (+mb) */
int   tab[3] = { 65, 66, 67 };  /* initialised int array                    */
int   zero;                     /* uninitialised           -> crt0 zeroes   */

main()
{
    putstr(arr);                /* image copy worked?              */
    putstr(parr);               /* data-pointer relocation worked? */
    putstr(plit);               /* code-pointer relocation worked? */
    if (tab[1] == 66) putstr("tab ok\n");
    if (zero == 0)    putstr("zero ok\n");
}
