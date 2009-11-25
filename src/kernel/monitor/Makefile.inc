#
# Makefile.inc
#

LOCAL_DIR		:= monitor


# The objects generated in this directory.
LOCAL_OBJECTS	:= monitor.o


# Include these objects in the kernel build
KERNEL_OBJECTS	+= $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))


