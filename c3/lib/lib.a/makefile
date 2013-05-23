include $(NITROS9DIR)/rules.mak

CFLAGS  = -s -q
RFILES = swab.r pwcryp.r bsearch.r dirutil.r errmsg.r prgname.r \
	datmods.r utime.r devtyp.r skip.r sets.r pflinit.r pffinit.r \
	fopen.r puts.r gets.r mktemp.r fread.r fwrite.r printf.r scanf.r\
	setbuf.r fseek.r putc.r ftelldummy.r getc.r flshdummy.r setbase.r \
	findstr.r pfldummy.r pffdummy.r htoi.r htol.r ltoa.r xtoa.r \
	system.r reverse.r strings.r strcmp.r strlen.r strncpy.r strncat.r \
	strncmp.r strhcpy.r strtok.r strpbrk.r strspn.r index.r patmatch.r \
	stringsu.r strucmp.r strnucmp.r case.r strclr.r memccpy.r memchr.r \
	memcmp.r memcpy.r memset.r atol.r atoi.r iob_data.r chcodes.r \
	l3tol.r ltol3.r qsort.r \
	sleep.r setjmp.r strass.r realloc.r calloc.r memory.r rand.r \
	atof.r scale.r gs1.r gs2.r ss1.r ss2.r ss3.r

main.l: $(RFILES) makefile
	cat $(RFILES) > main.l
#	cat pffinit_c $(RFILES2)  scanf_c >temp2
#	cat l3tol_c ltol3_c qsort_c $(RFILES6)  >temp7

clean:
	-$(RM) $(RFILES) main.l
