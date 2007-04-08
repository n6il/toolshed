SDIR	=	../../../casm/src
ODIR	=	.
GCC	=	@gcc -Wall -O3

SRC	=	as.c cmdline.c context.c cpu.c do9.c error.c eval.c export.c ffwd.c \
		input.c label.c macro.c parse.c \
		output.c output_bin.c output_listing.c output_mod.c \
		output_obj.c output_os9.c output_rof.c output_rom.c \
		output_s19.c \
		proc_branch.c proc_bitxfer.c proc_direct.c proc_extended.c \
		proc_general.c proc_immediate.c proc_indexed.c proc_inherent.c \
		proc_logicalmem.c proc_memxfer.c proc_pushpull.c proc_regtoreg.c \
		proc_util.c \
		pseudo.c struct.c symtab.c table9.c \
		util.c

DEFS	=	as.h config.h cpu.h error.h label.h macro.h \
		os9.h output.h proc_util.h proto.h pseudo.h \
		struct.h symtab.h table9.h util.h

DEFINES	:=	$(DEFS:%.h=$(SDIR)/.obj)
OBJ	:= 	$(SRC:%.c=$(ODIR)/%.o)
SRC	:=	$(SRC:%.c=$(SDIR)/%.c




$(ODIR)/casm: $(OBJ)
	@echo Linking...
	$(GCC) -o $(ODIR)/casm $(OBJ)


$(ODIR)/%.o: $(SDIR)/%.c $(HEADERS)
	@echo compiling $<
	$(GCC) -c $< -o $@

$(ODIR):
	@mkdir $(ODIR)

clean:
	@echo Cleaning...
	@rm -rf $(OBJ)
	@rm -rf casm

install: $(ODIR)/casm
	cp $(ODIR)/casm $(HOME)/bin
