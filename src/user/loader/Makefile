#
# Makefile
#

include ../Makefile.user


LOADER_BIN		:= loader.bin
LOADER_EXE		:= loader.exe
LOADER_DISASM	:= $(LOADER_BIN).disasm
LOADER_OBJECTS	:= loader.o loader_environment.o loader_main.o


.PHONY: all
all: $(LOADER_BIN) $(LOADER_DISASM)



#
# Size of the TAR header, in bytes
#
TAR_BLOCK_SIZE := 512



#
# Base address + entry point for the loader code; must be formatted in hex
# per the ld documentation
#
#@@@the "\#" syntax is a bug in make; see http://savannah.gnu.org/bugs/?20513
LOADER_TEXT := $(shell printf "%\#x" \
	$$(( $(USER_KERNEL_BOUNDARY) + $(TAR_BLOCK_SIZE) )) )



#
# Override the default entry point and .text section placement.  At boot-time,
# the kernel maps the ramdisk (tarball) at USER_KERNEL_BOUNDARY; and the
# initial user thread then jumps directly into the first TAR entry.  So the
# data mapped at (USER_KERNEL_BOUNDARY + TAR_BLOCK_SIZE) must be the
# executable code of the loader.
#
LINK_TEXT_SECTION := -e $(LOADER_TEXT) -Ttext $(LOADER_TEXT)



#
# The user loader.  The custom entry point must be at the head of .text section
# here
#
$(LOADER_EXE): $(LOADER_OBJECTS)



#
# The kernel does not recognize or parse file formats (ELF, a.out, etc), so
# strip off the extra packaging, leaving behind just the executable code.  This
# allows the initial user thread to jump directly (blindly) into this code.
# Append an empty block of extra/unused space for use as the .bss section; this
# assumes that (the .bss section + any section padding) fits inside 8KB
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

