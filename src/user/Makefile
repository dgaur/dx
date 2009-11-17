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
USER_LIBS		:= libdx user_start
USER_DAEMONS	:= keyboard loader shell vga
USER_SUBDIRS	:= $(USER_LIBS) $(USER_DAEMONS)



#
# The user-mode ramdisk
#
RAMDISK_FILE	:= ramdisk.tgz
RAMDISK_LISTING	:= ramdisk.lst

# The contents of the ramdisk.  The order is important here: the kernel expects
# find the user loader in the first entry
#@@the order also determines assignment of thread ids
RAMDISK_CONTENTS :=	loader/loader.bin \
					vga/vga.exe \
					shell/shell.exe \
					keyboard/keyboard.exe



#
# By default, build the entire tree and the ramdisk
#
.PHONY: all
all: $(USER_SUBDIRS) $(RAMDISK_FILE)



#
# Most, if not all, of the usermode components depend on the usermode
# libraries, so build those first
#
$(USER_DAEMONS): $(USER_LIBS)



#
# Build the child subdirectories individually
#
.PHONY: $(USER_SUBDIRS)
$(USER_SUBDIRS):
	@$(MAKE) -C $@ all



#
# The ramdisk is just a simple tarball
#
$(RAMDISK_FILE): $(RAMDISK_CONTENTS)
	@echo Building $@ ...
	@tar czf $@ $(RAMDISK_CONTENTS)
	@tar tvzf $@ > $(RAMDISK_LISTING)



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning user tree ...
	@rm -f $(RAMDISK_FILE)
	@rm -f $(RAMDISK_LISTING)
	@for dir in $(USER_SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done


