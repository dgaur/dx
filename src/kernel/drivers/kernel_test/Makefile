#
# Makefile
#
# Standard makefile for kernel component.  All local dependencies
# are listed in Makefile.inc.
#


#
# Import the common kernel configuration + the dependencies local
# to this directory
#
include ../../Makefile.kernel
include Makefile.inc



#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(LOCAL_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif



.PHONY: all
all:	$(LOCAL_OBJECTS)



.PHONY: clean
clean:
	@rm -f *.o
	@rm -f $(LOCAL_DEPENDENCIES)

