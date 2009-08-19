#
# Makefile
#
# Makefile for building floppy images.  Mainly useful for
# simulation/virtualization, because of the simple file format
#

include $(DX_ROOT_DIR)/Makefile.dx


BLANK_FLOPPY_IMAGE	:= grub-ext2-floppy.vfd
DX_FLOPPY_IMAGE		:= dx.vfd
DX_KERNEL_FILE		:= $(DX_SRC_DIR)/kernel/dx
DX_RAMDISK_FILE		:= $(DX_SRC_DIR)/user/ramdisk.tgz
FLOPPY_TREE			:= root
GRUB_MENU_FILE		:= menu.lst
LOAD_IMAGE			:= load-image.sh



all: $(DX_FLOPPY_IMAGE)


# Build a directory tree that mimics the layout of the floppy disk; install
# the necessary files in the appropriate places in this tree.  Unlike the
# .iso image, do *not* include the project archive here: floppy images are
# mainly useful for testing in a VM environment; and the full project tree
# may not fit on a floppy anyway
$(FLOPPY_TREE): $(GRUB_MENU_FILE) $(DX_KERNEL_FILE) $(DX_RAMDISK_FILE)
	@echo Rebuilding floppy tree ...
	@mkdir -p $(FLOPPY_TREE)/boot/grub/
	@cp $(GRUB_MENU_FILE)		$(FLOPPY_TREE)/boot/grub
	@cp $(DX_KERNEL_FILE)		$(FLOPPY_TREE)/boot
	@cp $(DX_RAMDISK_FILE)		$(FLOPPY_TREE)/boot
	@touch $(FLOPPY_TREE)	# Always rebuild .vfd after updating the tree


# Build a floppy image (ext2)
$(DX_FLOPPY_IMAGE): $(BLANK_FLOPPY_IMAGE) $(FLOPPY_TREE) $(LOAD_IMAGE) Makefile
	@cp -f $(BLANK_FLOPPY_IMAGE) $(DX_FLOPPY_IMAGE)
	@sh $(LOAD_IMAGE) $(DX_FLOPPY_IMAGE) $(FLOPPY_TREE)


# Clean the local tree
.PHONY: clean
clean:
	@rm -f $(DX_FLOPPY_IMAGE)
	@rm -rf $(FLOPPY_TREE)


# Transfer the entire image to a physical 1.44M floppy disk.  This completely
# overwrites the old contents of the floppy disk
.PHONY: ext2-floppy
ext2-floppy: $(DX_FLOPPY_IMAGE)
	@echo "Copying image to physical floppy (ext2) ..."
	@dd if=$(DX_FLOPPY_IMAGE) of=/dev/fd0 bs=32768 count=45


#@this only works under cygwin; need mtools or equivalent for *nix
.PHONY: fat12-floppy
fat12-floppy: $(DX_FLOPPY_IMAGE)
	@echo "Copying image to physical floppy (fat12) ..."
	@cp -r $(FLOPPY_TREE)/* /cygdrive/a
