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
# As a nicety, build a clean copy (archive) of the entire project tree; this
# is typically useful for building release packages or posting tarballs, etc
#
archive:
	@echo "Generating archive directory ..."
	@hg archive $(DX_ARCHIVE_DIR)
	@$(MAKE) -C $(DX_ARCHIVE_DIR)/doc/html


	
#
# Build the child subdirectories individually
#
.PHONY: $(ROOT_SUBDIRS)
$(ROOT_SUBDIRS):
	@$(MAKE) -C $@ all



#
# Clean the source tree.  Leave "configure" output + releases as is.
#
.PHONY: clean
clean:
	@echo Cleaning top-level tree ...
	@for dir in $(ROOT_SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	@rm -rf $(DX_ARCHIVE_DIR)



#
# Full clean.  Clean the source tree.  Delete "configure" output + any archived
# release images
#
.PHONY: distclean
distclean: clean
	@./configure --distclean
	@rm -rf releases



#
# Display a brief help message listing targets + options
#
.PHONY: help
help:
	@echo
	@echo "Building the dx operating system --"
	@echo "* \"make all\" builds the entire tree"
	@echo "* \"make archive\" builds a clean project archive"
	@echo "* \"make clean\" cleans the source tree"
	@echo "* \"make distclean\" cleans the source tree, releases, config files"
	@echo "* \"make doc\" builds the doxygen (HTML) documentation"
	@echo "* \"make help\" displays this message"
	@echo "* \"make src\" builds the source tree"
	@echo
	@echo "Add \"DEBUG=1\" to generate a debug build, e.g., \"make all DEBUG=1\""
	@echo


