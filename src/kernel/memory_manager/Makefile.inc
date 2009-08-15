#
# Makefile.inc
#

LOCAL_DIR		:= memory_manager


# The objects generated in this directory
LOCAL_OBJECTS	:= address_space.o \
				   address_space_manager.o \
				   delete.o \
				   kernel_heap.o \
				   memory_manager.o \
				   memory_pool.o \
				   new.o \
				   page_frame_manager.o \
				   page_frame_region.o \
				   shared_frame.o


# Include these objects in the kernel build
KERNEL_OBJECTS  += $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))

