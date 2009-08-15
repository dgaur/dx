#
# Makefile
#
# Makefile for generating code documentation (HTML) for various components
# of the system
#



#
# The different documentation packages
#
DOXYGEN_TARGETS		:= kernel user



#
# By default, build all of the documentation
#
.PHONY: all
all: $(DOXYGEN_TARGETS)



#
# Kernel documentation
#
kernel: kernel.dox
	@doxygen $<



#
# User documentation
#
user: user.dox
	@doxygen $<



#
# Clean all of the documentation
#
.PHONY: clean
clean:
	@echo Cleaning HTML tree
	@rm -rf $(DOXYGEN_TARGETS)

