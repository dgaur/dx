#
# Makefile
#

include ../Makefile.user


LOCAL_DIR		:= console
LOCAL_EXE		:= console.exe
LOCAL_OBJECTS	:= console.o
LOCAL_PRIORITY	:= S02


.PHONY: all
all: $(LOCAL_EXE)


$(LOCAL_EXE): $(LOCAL_OBJECTS)


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

