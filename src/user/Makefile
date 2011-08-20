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
USER_LIBS		:= libdx libtar user_start
USER_DAEMONS	:= keyboard loader shell vga
USER_SUBDIRS	:= $(USER_LIBS) $(USER_DAEMONS)



#
# The user-mode ramdisk
#
RAMDISK_BOOT_TREE	:= boot
RAMDISK_FILE		:= ramdisk.tgz
RAMDISK_LISTING		:= ramdisk.lst


#
# The contents of the ramdisk.  The order is important here: the kernel expects
# find the user loader in the first entry
#@@the order also determines assignment of thread ids
#
RAMDISK_CONTENTS :=	loader.bin \
					vga.exe \
					shell.exe \
					keyboard.exe


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
# Pack the various boot-time daemons into a tarball; this becomes the
# boot-time ramdisk.
#	- locate the various daemons in their respective subdirectories
#	- create a symbolic link under $(RAMDISK_BOOT_TREE) to each daemon
#	- collect $(RAMDISK_BOOT_TREE) into a simple tarball
#
vpath %.exe $(USER_DAEMONS)
vpath %.bin $(USER_DAEMONS)
$(RAMDISK_FILE): $(RAMDISK_CONTENTS)
	@echo Building $@ ...
	@mkdir -p $(RAMDISK_BOOT_TREE)
	@ln -s -f -t $(RAMDISK_BOOT_TREE) $(abspath $^)
	@tar chzf $@ \
		$(foreach file, $(RAMDISK_CONTENTS), $(RAMDISK_BOOT_TREE)/$(file)) \
		--transform="s,^,/,"
	@tar tvzf $@ > $(RAMDISK_LISTING)



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning user tree ...
	@rm -rf $(RAMDISK_BOOT_TREE)
	@rm -f $(RAMDISK_FILE)
	@rm -f $(RAMDISK_LISTING)
	@for dir in $(USER_SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done


