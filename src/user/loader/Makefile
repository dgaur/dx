#
# Makefile
#

include Makefile.inc
include ../Makefile.user


.PHONY: all
all: $(LOADER_BIN) $(LOADER_DISASM)


#
# The user-mode loader is vaguely magical --
#
# * The kernel will not parse the ramdisk image; it just assumes the loader is
#   always in the first entry.
#
# * Furthermore, the kernel will not parse the loader's executable image (i.e.,
#   it lacks any knowledge of ELF or other file formats); rather, it maps the
#	ramdisk at USER_KERNEL_BOUNDARY; and the initial user thread then blindly
#	jumps directly into the first .tar entry.  So the data mapped at
#	(USER_KERNEL_BOUNDARY + TAR_BLOCK_SIZE) must be the executable code of the
#	loader.
#
# Like this:
#	... lower addresses ...
#
#	USER_KERNEL_BOUNDARY:					Ramdisk starts here.
#
#	USER_KERNEL_BOUNDARY + TAR_BLOCK_SIZE:	Loader instructions start here;
#											the initial user thread jumps here
#											directly.
#
#	USER_KERNEL_BOUNDARY + N:				Rest of loader.  Rest of ramdisk.
#
#	... higher addresses ...
#


#
# Size of the TAR header, in bytes
#
TAR_BLOCK_SIZE := 512



#
# Base address + entry point for the loader code; must be formatted in hex
# per the ld documentation.  This is the important address: the kernel jumps
# here; and the loader instructions must start here.
#
#@@@the "\#" syntax is a bug in make; see http://savannah.gnu.org/bugs/?20513
LOADER_TEXT := $(shell printf "%\#x" \
	$$(( $(USER_KERNEL_BOUNDARY) + $(TAR_BLOCK_SIZE) )) )



#
# Override the default entry point and .text section placement, to ensure the
# loader is mapped in the correct location
#
LINK_TEXT_SECTION := -e $(LOADER_TEXT) -Ttext $(LOADER_TEXT)


#
# The loader.o instructions must be placed precisely at $(LOADER_TEXT); prevent
# gcc from re-arranging the memory layout
#
CC_FLAGS  += -fno-reorder-functions


#
# The user loader.  The custom entry point must be at the head of .text section
# here
#
$(LOADER_EXE): $(LOADER_OBJECTS)


#
# Strip off the extra ELF packaging, leaving behind just the loader's executable
# code.  Append an empty block of extra/unused space for use as the .bss
# section; this assumes that (the .bss section + any section padding) fits
# inside 8KB
#
$(LOADER_BIN): $(LOADER_EXE)
	@echo Generating $(LOADER_BIN) ...
	@$(OBJCOPY) -O binary $< $@
	@dd if=/dev/zero of=$@ oflag=append bs=4096 count=2 conv=notrunc > \
		/dev/null 2>&1



#
# Disassemble the raw loader image, mainly for debugging
#
$(LOADER_DISASM): $(LOADER_BIN)
	@$(OBJDUMP) $(OBJDUMP_FLAGS) -D -b binary -m i386 \
		--adjust-vma=$(LOADER_TEXT) $< > $@



#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(LOADER_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif



#
# Clean the local directory
#
clean:
	@rm -f $(LOADER_BIN)
	@rm -f $(LOADER_DISASM)
	@rm -f $(LOADER_EXE)
	@rm -f $(LOADER_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)

