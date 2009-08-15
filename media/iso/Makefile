#
# Makefile
#
# Makefile for building .iso images
#

include $(DX_ROOT_DIR)/Makefile.dx


DX_ISO_IMAGE		:= dx.iso
GRUB_MENU_FILE		:= menu.lst
GRUB_STAGE2_FILE	:= stage2_eltorito
ISO_TREE			:= root
MKISOFS				:= mkisofs  # Normally in /usr/bin



all: $(DX_ISO_IMAGE)


# Build a directory tree that mimics the layout of the .iso; install
# the necessary files in the appropriate places in this tree.  Include the
# project archive, if any, along with the actual binaries
.PHONY: build-tree
build-tree: $(GRUB_MENU_FILE) $(GRUB_STAGE2_FILE)
	@mkdir -p $(ISO_TREE)/boot/grub/
	@cp $(GRUB_MENU_FILE)					$(ISO_TREE)/boot/grub
	@cp $(GRUB_STAGE2_FILE)					$(ISO_TREE)/boot/grub
	@cp $(DX_SRC_DIR)/kernel/dx				$(ISO_TREE)/boot
	@cp $(DX_SRC_DIR)/user/ramdisk.tgz		$(ISO_TREE)/boot
	@ln -s $(DX_MEDIA_DIR)/dx.archive		$(ISO_TREE)/dx.archive



# Build the .iso image
$(DX_ISO_IMAGE): build-tree Makefile
	$(MKISOFS) -b boot/grub/$(GRUB_STAGE2_FILE) --boot-info-table \
		--boot-load-size 4 -f --no-emul -o $@ -r $(ISO_TREE)



# Clean the local tree
clean:
	@rm -f $(DX_ISO_IMAGE)
	@rm -rf $(ISO_TREE)


