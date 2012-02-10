#
# Makefile
#

include ../Makefile.user


LOCAL_DIR		:= lua
LOCAL_EXE		:= lua.exe
LOCAL_OBJECTS	:= lua.o


LINK_EXTRA_LIBS	+= $(LINK_LIBLUA) $(LINK_LIBM)

.PHONY: all
all: $(LOCAL_EXE)


$(LOCAL_EXE): $(LOCAL_OBJECTS)


#
# Include Lua support
#
CC_INCLUDES += -I$(dir $(LINK_LIBLUA))


.PHONY: install
install: $(LOCAL_EXE)
	@mkdir -p $(RAMDISK_DIR)/bin
	@cp -a $< $(RAMDISK_DIR)/bin/$<


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

