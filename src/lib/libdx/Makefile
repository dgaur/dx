#
# Makefile
#

include ../Makefile.lib


#
# The individual object files that comprise the dx library
#
LIBDX_OBJECTS	:=	libdx.o \
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


LIBDX_FILE	:= $(notdir $(LINK_LIBDX))


#
# Custom tool settings for this library
#
CC_FLAGS += -std=c99


#
# Build the actual libdx library
#
all: $(LIBDX_FILE)

$(LIBDX_FILE):	$(LIBDX_OBJECTS)




#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(LIBDX_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif



#
# Clean the local directory
#
clean:
	@rm -f $(LIBDX_FILE)
	@rm -f $(LIBDX_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)

