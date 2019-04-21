
include ../rules.mak

vpath %.md ../../../doc

HTML = ToolShed.html
MARKDOWN = markdown

all: $(HTML)

%.html: %.md
	-$(MARKDOWN) $< > $@

clean:
	$(RM) $(HTML)
