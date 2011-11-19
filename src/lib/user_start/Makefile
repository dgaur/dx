#
# Makefile
#

include ../Makefile.lib


USER_START_FILE		:= $(notdir $(LINK_USER_START))
USER_START_OBJECTS	:= setup_main.o start.o 


all: $(USER_START_FILE)


#
# Link the startup files into a single object
#
$(USER_START_FILE): $(USER_START_OBJECTS)
	@echo Linking $@
	@$(LINK) -r $(LINK_FLAGS) $^


#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(USER_START_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif


clean:
	@rm -f $(USER_START_FILE) $(USER_START_OBJECTS)
