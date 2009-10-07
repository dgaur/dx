#
# Makefile
#
# This is the top-level kernel Makefile.  All kernel builds
# should start here.
#


# Include the default kernel definitions + configuration
include Makefile.kernel



#
# Kernel subdirectories that may be built
#
KERNEL_SUBDIRS	:= boot \
				   device_proxy \
				   drivers \
				   drivers/kernel_test \
				   hal \
				   io_manager \
				   libk \
				   memory_manager \
				   thread_manager



#
# Generate the list of $(KERNEL_OBJECTS) that should be linked into the kernel
# image
#
KERNEL_OBJECTS :=
CHILD_MAKEFILES	= $(foreach DIR, $(KERNEL_SUBDIRS), $(DIR)/Makefile.inc)
include $(CHILD_MAKEFILES)



#
# Generate/include the dependency information for each of the child objects
#
ifeq ($(filter clean help, $(MAKECMDGOALS)),)
-include $(KERNEL_OBJECTS:.o=.dep)
endif



#
# Build the kernel itself
#
.PHONY: all
all: $(KERNEL_FILE)

$(KERNEL_FILE):	$(KERNEL_OBJECTS) $(KERNEL_LINK_SCRIPT) $(LINK_COMMON_LIBS) $(MAKEFILE_LIST)
	@echo Linking $@ ...
	@$(LINK) $(LINK_FLAGS) $(KERNEL_OBJECTS) $(LINK_COMMON_LIBS)
	@$(NM) $(NM_FLAGS) $@ > $(SYMBOL_FILE)
	@$(OBJDUMP) $(OBJDUMP_FLAGS) $@ > $(DISASM_FILE)
	@$(STRIP) $@



#
# Build the child subdirectories individually
#
.PHONY: $(KERNEL_SUBDIRS)
$(KERNEL_SUBDIRS):
	@$(MAKE) -C $@ all



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning kernel tree ...
	@rm -f $(KERNEL_FILE)
	@rm -f $(SYMBOL_FILE)
	@rm -f $(DISASM_FILE)
	@for dir in $(KERNEL_SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done



#
# Display a brief help message listing targets + options
#
.PHONY: help
help:
	@echo
	@echo Building the dx kernel --
	@echo \* \"make all\" builds the dx kernel
	@echo \* \"make clean\" cleans the kernel tree
	@echo \* \"make dx\" builds the dx kernel
	@echo \* \"make help\" displays this message
	@echo
	@echo Add \"DEBUG=1\" to generate a debug builds, e.g., \"make dx DEBUG=1\"
	@echo

