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
SRC_SUBDIRS	:= kernel lib user


#
# By default, build the entire tree
#
.PHONY: all
all: $(SRC_SUBDIRS)



#
# Both the kernel + user executables depend on the common libraries, so build
# those first
#
kernel user: lib



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
	@echo "* \"make help\" displays this message"
	@echo "* \"make kernel\" builds the kernel tree"
	@echo "* \"make lib\" builds the various libraries"
	@echo "* \"make user\" builds the user tree"
	@echo
	@echo "Add \"DEBUG=1\" to generate a debug builds, e.g., \"make all DEBUG=1\""
	@echo
