#
# Makefile
#


#
# Inherit the various user settings
#
include Makefile.user



#
# The various user-mode components (subtrees) that may be built here
#
# @@The order is important here: the kernel expects find the user
# loader in the first entry; and the order also determines assignment of
# thread ids@@
#
USER_DIRS	:= loader vga shell keyboard


#
# Generate the list of files that should be included in the ramdisk
#
USER_EXES		:=
CHILD_MAKEFILES	:= $(foreach DIR, $(USER_DIRS), $(DIR)/Makefile.inc)
include $(CHILD_MAKEFILES)


#
# The user-mode ramdisk
#
RAMDISK_BOOT_TREE	:= boot
RAMDISK_FILE		:= ramdisk.tgz
RAMDISK_LISTING		:= ramdisk.lst


#
# The ramdisk should collect all of the necessary user binaries
#
.PHONY: all
all: $(RAMDISK_FILE) $(RAMDISK_LISTING)



#
# Build the executables individually.  Always try to rebuild these when
# collecting the ramdisk, since the dependencies are unknown here
#
.PHONY: $(USER_EXES)
$(USER_EXES):
	@$(MAKE) -C $(dir $@) all


#
# Build the child subdirectories individually
#
.PHONY: $(USER_DIRS)
$(USER_DIRS):
	@$(MAKE) -C $@ all



#
# Pack the various boot-time daemons into a tarball; this becomes the
# boot-time ramdisk.
#	- locate the various daemons in their respective subdirectories
#	- create a symbolic link under $(RAMDISK_BOOT_TREE) to each daemon
#	- collect $(RAMDISK_BOOT_TREE) into a simple tarball
#
$(RAMDISK_FILE): $(USER_EXES)
	@echo Building $@ ...
	@mkdir -p $(RAMDISK_BOOT_TREE)
	@ln -s -f -t $(RAMDISK_BOOT_TREE) $(addprefix ../, $^)
	@$(TAR) chzf $@ \
		$(foreach file, $(notdir $^), $(RAMDISK_BOOT_TREE)/$(file)) \
		--transform="s,^,/,"


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
	@rm -rf $(RAMDISK_BOOT_TREE)
	@rm -f $(RAMDISK_FILE)
	@rm -f $(RAMDISK_LISTING)
	@for dir in $(USER_DIRS); do \
		$(MAKE) -C $$dir clean; \
	done


