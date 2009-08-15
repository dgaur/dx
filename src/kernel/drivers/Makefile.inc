#
# Makefile.inc
#

LOCAL_DIR		:= drivers


# The objects generated in this directory.
LOCAL_OBJECTS	:= display.o \
				   i8254pit.o \
				   i8259pic.o


# In the debug build (only), include the serial port driver for debugging
ifdef DEBUG
LOCAL_OBJECTS	+= serial_console.o
endif


# Include these objects in the kernel build
KERNEL_OBJECTS	+= $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))


