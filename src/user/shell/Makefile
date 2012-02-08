#
# Makefile
#

include ../Makefile.user


LOCAL_DIR		:= shell
LOCAL_EXE		:= shell.exe
LOCAL_OBJECTS	:= shell.o
LOCAL_PRIORITY	:= S03


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
	@mkdir -p $(RAMDISK_DIR)/boot
	@cp -a $< $(RAMDISK_DIR)/boot/$(LOCAL_PRIORITY)$<


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

