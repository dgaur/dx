#
# Makefile
#

include ../Makefile.user


SHELL_EXECUTABLE	:= shell.exe
SHELL_OBJECTS		:= shell.o


.PHONY: all
all: $(SHELL_EXECUTABLE)


$(SHELL_EXECUTABLE): $(SHELL_OBJECTS)


#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(SHELL_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif


clean:
	@rm -f $(SHELL_EXECUTABLE)
	@rm -f $(SHELL_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)

