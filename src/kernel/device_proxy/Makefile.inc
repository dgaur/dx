#
# Makefile.inc
#

LOCAL_DIR		:= device_proxy


# The objects generated in this directory.
LOCAL_OBJECTS	:= device_proxy.o


# Include these objects in the kernel build
KERNEL_OBJECTS	+= $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))


