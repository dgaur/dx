#
# Makefile.inc
#

LOCAL_DIR		:= boot


# The objects generated in this directory
LOCAL_OBJECTS	:= boot.o \
				   kernel_init.o \
				   multiboot_header.o


# Include these objects in the kernel build
KERNEL_OBJECTS	+= $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))

