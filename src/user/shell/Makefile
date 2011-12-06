#
# Makefile
#

include Makefile.inc
include ../Makefile.user


.PHONY: all
all: $(LOCAL_EXE)


$(LOCAL_EXE): $(LOCAL_OBJECTS)


#
# Include Lua support
#
CC_INCLUDES += -I$(dir $(LINK_LIBLUA))


#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(LOCAL_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif


clean:
	@rm -f $(LOCAL_EXE)
	@rm -f $(LOCAL_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)

