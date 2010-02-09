#
# Makefile
#

# Inherit the local build settings
include Makefile.common


#
# Subtrees that may be built
#
COMMON_SUBDIRS	:= libc libm


#
# By default, build the entire tree
#
.PHONY: all
all: $(COMMON_SUBDIRS)



#
# Build the child subdirectories individually
#
.PHONY: $(COMMON_SUBDIRS)
$(COMMON_SUBDIRS):
	@$(MAKE) -C $@ all



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning common tree ...
	@for dir in $(COMMON_SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done



