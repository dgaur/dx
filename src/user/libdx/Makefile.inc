#
# Makefile.inc for libdx
#

LOCAL_DIR	:= libdx
LOCAL_LIB	:= libdx.a


#
# The individual object files that comprise the dx library
#
LOCAL_OBJECTS	:=	libdx.o \
					create_address_space.o \
					create_process.o \
					create_thread.o \
					defer_interrupt.o \
					delete_message.o \
					delete_thread.o \
					expand_address_space.o \
					interrupt_handler_loop.o \
					map_device.o \
					message.o \
					read_kernel_stats.o \
					receive_message.o \
					register_interrupt_handler.o \
					send_and_receive_message.o \
					send_message.o \
					start_thread.o \
					unmap_device.o \
					unregister_interrupt_handler.o


USER_LIBS	+= $(LOCAL_DIR)/$(LOCAL_LIB)

