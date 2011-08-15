#
# Makefile
#

include ../Makefile.user


LIBTAR_OBJECTS	:=	libtar.o

LIBDX_FILE = $(notdir $(LINK_LIBTAR))



#
# Build the actual tar library
#
all: $(LIBTAR_FILE)

$(LIBTAR_FILE):	$(LIBTAR_OBJECTS)



#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(LIBTAR_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif



#
# Clean the local directory
#
clean:
	@rm -f $(LIBTAR_FILE)
	@rm -f $(LIBTAR_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)

