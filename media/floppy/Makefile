#
# Makefile
#
# Makefile for building floppy images.  Mainly useful for
# simulation/virtualization, because of the simple file format
#

include $(DX_ROOT_DIR)/Makefile.dx


BLANK_FLOPPY_IMAGE	:= grub-ext2-floppy.vfd
DX_FLOPPY_IMAGE		:= dx.vfd
FLOPPY_TREE			:= root
GRUB_MENU_FILE		:= menu.lst
LOAD_IMAGE			:= load-image.sh



all: $(DX_FLOPPY_IMAGE)


# Build a directory tree that mimics the layout of the floppy disk; install
# the necessary files in the appropriate places in this tree.  Unlike the
# .iso image, do *not* include the project archive here: floppy images are
# mainly useful for testing in a VM environment; and the full project tree
# may not fit on a floppy anyway
.PHONY: build-tree
build-tree: $(GRUB_MENU_FILE)
	@mkdir -p $(FLOPPY_TREE)/boot/grub/
	@cp $(GRUB_MENU_FILE)					$(FLOPPY_TREE)/boot/grub
	@cp $(DX_SRC_DIR)/kernel/dx				$(FLOPPY_TREE)/boot
	@cp $(DX_SRC_DIR)/user/ramdisk.tgz		$(FLOPPY_TREE)/boot


# Build a floppy image (ext2)
$(DX_FLOPPY_IMAGE): $(BLANK_FLOPPY_IMAGE) build-tree $(LOAD_IMAGE) Makefile
	@cp -f $(BLANK_FLOPPY_IMAGE) $(DX_FLOPPY_IMAGE)
	@sh $(LOAD_IMAGE) $(DX_FLOPPY_IMAGE) $(FLOPPY_TREE)


# Clean the local tree
clean:
	@rm -f $(DX_FLOPPY_IMAGE)
	@rm -rf $(FLOPPY_TREE)


# Transfer the entire image to a physical 1.44M floppy disk.  This completely
# overwrites the old contents of the floppy disk
ext2-floppy: $(DX_FLOPPY_IMAGE)
	@echo "Copying image to physical floppy (ext2) ..."
	@dd if=$(DX_FLOPPY_IMAGE) of=/dev/fd0 bs=32768 count=45


fat12-floppy: $(DX_FLOPPY_IMAGE)
	@echo "Copying image to physical floppy (fat12) ..."
	@cp -r $(FLOPPY_TREE)/* /cygdrive/a
