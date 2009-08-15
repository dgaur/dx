#
# Makefile
#

include ../Makefile.user


KEYBOARD_DRIVER		:= keyboard.exe
KEYBOARD_OBJECTS	:= keyboard.o


.PHONY: all
all: $(KEYBOARD_DRIVER)


$(KEYBOARD_DRIVER): $(KEYBOARD_OBJECTS)


#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(KEYBOARD_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif


clean:
	@rm -f $(KEYBOARD_DRIVER)
	@rm -f $(KEYBOARD_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)

