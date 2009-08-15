#
# Makefile
#

include ../Makefile.user


VGA_DRIVER	:= vga.exe
VGA_OBJECTS	:= vga.o


.PHONY: all
all: $(VGA_DRIVER)


$(VGA_DRIVER): $(VGA_OBJECTS)


#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(VGA_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif


clean:
	@rm -f $(VGA_DRIVER)
	@rm -f $(VGA_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)

