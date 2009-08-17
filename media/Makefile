#
# Makefile
#

include $(DX_ROOT_DIR)/Makefile.dx


#
# Child subdirectories, one for each flavor of physical media
#
MEDIA_SUBDIRS := floppy	iso



#
# By default, build the entire tree
#
.PHONY: all
all: $(MEDIA_SUBDIRS)



#
# Build the child subdirectories individually
#
.PHONY: $(MEDIA_SUBDIRS)
$(MEDIA_SUBDIRS):
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
