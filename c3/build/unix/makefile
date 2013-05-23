dirs	= exec prep comp opt compx optx lorder lsplit

# Make all components
all:
	@echo "**************************************************"
	@echo "*                                                *"
	@echo "*               MW C Compiler                    *"
	@echo "*                                                *"
	@echo "**************************************************"
	$(foreach dir, $(dirs), (cd $(dir); make);)

# Clean all components
clean:
	$(foreach dir, $(dirs), (cd $(dir); make clean);)

# Install all compontents
install:
	$(foreach dir, $(dirs), (cd $(dir); make install);)
