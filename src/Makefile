#
# Makefile
#


#
# Inherit the global OS build settings
#
include $(DX_ROOT_DIR)/Makefile.dx


#
# Subtrees that may be built
#
SRC_SUBDIRS	:= common external kernel user


#
# By default, build the entire tree
#
.PHONY: all
all: $(SRC_SUBDIRS)



#
# The order is important here.  Must build the common library code before either
# the kernel or the user code; likewise, must build the external libraries
# before any user executables
#
kernel: common
user:   common external



#
# Build the child subdirectories individually
#
.PHONY: $(SRC_SUBDIRS)
$(SRC_SUBDIRS):
	@$(MAKE) -C $@ all



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning source tree ...
	@for dir in $(SRC_SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done



#
# Display a brief help message listing targets + options
#
.PHONY: help
help:
	@echo
	@echo "Building the dx source tree --"
	@echo "* \"make all\" builds the entire tree"
	@echo "* \"make clean\" cleans the entire tree"
	@echo "* \"make common\" builds the common kernel/user libraries"
	@echo "* \"make external\" builds the external (3rd party) libraries"
	@echo "* \"make kernel\" builds the kernel tree"
	@echo "* \"make help\" displays this message"
	@echo "* \"make user\" builds the user tree"
	@echo
	@echo "Add \"DEBUG=1\" to generate a debug builds, e.g., \"make all DEBUG=1\""
	@echo
