#
# Makefile.inc
#

LOCAL_DIR		:= libk


# The objects generated in this directory.
LOCAL_OBJECTS	:= bitmap.o \
				   compiler_dependencies.o \
				   large_message.o \
				   klibc.o \
				   medium_message.o \
				   message.o


# Include any debug logic in the DEBUG build
ifdef DEBUG
LOCAL_OBJECTS += debug.o
endif


# Include these objects in the kernel build
KERNEL_OBJECTS  += $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))

