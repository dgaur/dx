#
# Makefile
#
# This is the top-level dx Makefile.  All full product builds start here.
#
# See Makefile.dx for the list of customizations + required environment
#



# Include the default definitions + configuration
include Makefile.dx



#
# Top-level directories that may be built
#
ROOT_SUBDIRS	:= doc media src



#
# Cannot build any media images until the source tree is built
#
media: src



#
# By default, build the entire tree: source, documentation, everything
#
.PHONY: all
all: $(ROOT_SUBDIRS)



#
# Build the child subdirectories individually
#
PARALLEL_JOBS := $(shell grep -c processor /proc/cpuinfo)
.PHONY: $(ROOT_SUBDIRS)
$(ROOT_SUBDIRS):
	@$(MAKE) -C $@ -j $(PARALLEL_JOBS) -l $(PARALLEL_JOBS) all



#
# Clean the source tree.  Leave "configure" output and external sources.
#
.PHONY: clean
clean:
	@echo Cleaning top-level tree ...
	@for dir in $(ROOT_SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done



#
# Full clean.  Clean the source tree.  Delete "configure" output.  Delete
# external/third-party sources.
#
.PHONY: distclean
distclean: clean
	@$(MAKE) -C src/external $@
	@rm -f $(DX_ARCHIVE_FILE)
	@./configure --distclean



#
# Display a brief help message listing targets + options
#
.PHONY: help
help:
	@echo
	@echo "Building the dx operating system --"
	@echo "* \"make all\" builds the entire tree"
	@echo "* \"make clean\" cleans the source tree"
	@echo "* \"make distclean\" cleans the source tree, config files"
	@echo "* \"make doc\" builds the doxygen (HTML) documentation"
	@echo "* \"make help\" displays this message"
	@echo "* \"make src\" builds the source tree"
	@echo
	@echo "Add \"DEBUG=1\" to generate a debug build, e.g., \"make all DEBUG=1\""
	@echo


