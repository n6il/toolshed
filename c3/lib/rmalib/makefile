# Makefile to create assembler library
include $(NITROS9DIR)/rules.mak


MODS = stimestr.r datestr.r getfmd.r mktemp.r linedit.r sho_regs.r \
	print_dec.r print_hex.r print_asc.r \
	mult16x8.r mult16x16.r div16x8.r div16x16.r div8x8.r \
	dec_bin.r bin_dec.r asc_bin.r bin_asc.r hex_bin.r \
	bin_hex.r bin2hex.r dectab.r bin_rom.r \
	strcat.r strcpy.r strncpy.r parsnstr.r strhcpy.r strhlen.r \
	strcmp.r strncmp.r strlen.r memmove.r ptsearch.r compare.r \
	b09strlen.r \
	to_upprs.r to_upper.r to_lowrs.r to_lower.r \
	is_punct.r is_print.r is_cntrl.r is_space.r is_xdigit.r \
	is_alnum.r is_alpha.r is_digit.r is_lower.r is_upper.r is_termin.r \
	prints.r puts.r fputs.r putcr.r fputcr.r putspace.r fputspace.r \
	putc.r fputc.r inkey.r getc.r fgetc.r gets.r fgets.r puty.r fputy.r \
	gety.r fgety.r ftrans.r \
	frewind.r ftoeof.r \
	jsr_cmd.r jsr_cmd2.r \
	to_non_sp.r memset.r rnd.r \
	os9defs.r windefs.r


all:	alib.l	test

alib.l: $(MODS)
	$(MERGE) $^ > $@

test: test.r
	$(LINKER) $^ -o=$@ -l=alib.l
	
clean:
	$(RM) *.r alib.l test