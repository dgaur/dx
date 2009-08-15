#
# Makefile.inc
#

LOCAL_DIR		:= io_manager


# The objects generated in this directory.
LOCAL_OBJECTS	:= io_manager.o


# Include these objects in the kernel build
KERNEL_OBJECTS	+= $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))


