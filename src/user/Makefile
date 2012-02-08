#
# Makefile
#


#
# Inherit the various user settings
#
include Makefile.user



#
# The user-mode ramdisk
#
RAMDISK_DIR			:= ramdisk
RAMDISK_FILE		:= ramdisk.tgz
RAMDISK_LISTING		:= ramdisk.lst
RAMDISK_SKELETON	:= skeleton


#
# By default, collect all of the necessary user scripts + binaries into a single
# ramdisk image
#
.PHONY: all
all: $(RAMDISK_FILE) $(RAMDISK_LISTING)


#
# Create the initial ramdisk tree
#
$(RAMDISK_DIR):
	@mkdir -p $@


#
# The various user-mode components (subtrees) that may be built here;
# automatically install them into the ramdisk tree
#
USER_DIRS	:= loader vga console shell keyboard

.PHONY: $(USER_DIRS)
$(USER_DIRS): $(RAMDISK_DIR)
	@$(MAKE) -C $@ install RAMDISK_DIR=$(realpath $(RAMDISK_DIR))


#
# Pack the various daemons, drivers and other user executables into a tarball;
# this becomes the boot-time ramdisk.  Ensure that the loader is installed as
# the first entry, so that the kernel can locate it at boot-time
#
$(RAMDISK_FILE): $(RAMDISK_DIR) $(RAMDISK_SKELETON) $(USER_DIRS)
	@echo Building $@ ...
	@cp -a $(RAMDISK_SKELETON)/* $(RAMDISK_DIR)
	@$(TAR) chzf $@ --transform="s,^$(RAMDISK_DIR),," \
		$(wildcard $(RAMDISK_DIR)/boot/S00*) \
		$(RAMDISK_DIR)


#
# Record the contents of the ramdisk
#
$(RAMDISK_LISTING): $(RAMDISK_FILE)
	@$(TAR) tvzf $< > $@



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning user tree ...
	@rm -rf $(RAMDISK_DIR)
	@rm -f $(RAMDISK_FILE)
	@rm -f $(RAMDISK_LISTING)
	@for dir in $(USER_DIRS); do \
		$(MAKE) -C $$dir clean; \
	done


