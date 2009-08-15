#
# Makefile.inc
#

LOCAL_DIR		:= hal


# The objects generated in this directory.
LOCAL_OBJECTS	:= atomic_int32.o \
				   bits.o \
				   gdt.o \
				   idt.o \
				   interrupt.o \
				   interrupt_handler_stub.o \
				   io_mapped_register.o \
				   io_port_map.o \
				   kernel_panic.o \
				   page_directory.o \
				   processor_type.o \
				   spinlock.o \
				   thread.o \
				   tss.o \
				   x86_hal.o



# Include these objects in the kernel build
KERNEL_OBJECTS  += $(foreach OBJECT, $(LOCAL_OBJECTS), $(LOCAL_DIR)/$(OBJECT))
