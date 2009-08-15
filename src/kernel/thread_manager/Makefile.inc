#
# Makefile.inc
#

LOCAL_DIR		:= thread_manager


# The objects generated in this directory.
LOCAL_OBJECTS	:= cleanup_thread.o \
				   idle_thread.o \
				   null_thread.o \
				   thread.o \
				   thread_manager.o \
				   user_thread.o


# Include these objects in the kernel build
KERNEL_OBJECTS	+= $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))


