#
# Makefile
#

include $(DX_ROOT_DIR)/Makefile.dx


#
# Child subdirectories, one for each flavor of physical media
#
MEDIA_SUBDIRS := floppy	iso



#
# Subdirectory for archiving/preserving the source tree
#
ARCHIVE_SUBDIR := dx.archive



#
# By default, build the entire tree
#
.PHONY: all
all: $(MEDIA_SUBDIRS)



#
# Generate an archive of the current project source; this is typically only
# useful when packaging a release
#
$(ARCHIVE_SUBDIR):
	@mkdir $(ARCHIVE_SUBDIR)
	@if [ -n "$(INCLUDE_ARCHIVE)" ]; then \
		echo "Generating archive directory ..."; \
	fi



#
# Build the child subdirectories individually
#
.PHONY: $(MEDIA_SUBDIRS)
$(MEDIA_SUBDIRS): $(ARCHIVE_SUBDIR)
	@$(MAKE) -C $@ all



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning media tree ...
	@for dir in $(MEDIA_SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	@rm -rf $(ARCHIVE_SUBDIR)
