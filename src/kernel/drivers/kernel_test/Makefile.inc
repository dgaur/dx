#
# Makefile.inc
#

LOCAL_DIR		:= drivers/kernel_test


# The objects generated in this directory
LOCAL_OBJECTS	:= kernel_test.o \
				   memory_tests.o \
				   message_tests.o \
				   misc_tests.o \
				   thread_tests.o \
				   type_tests.o



# Only include the unit-tests in the debug kernel build
ifdef DEBUG
KERNEL_OBJECTS	+= $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))
endif

