#
# Makefile.inc for keyboard driver
#

LOCAL_DIR		:= keyboard
LOCAL_EXE		:= keyboard.exe
LOCAL_OBJECTS	:= keyboard.o

RAMDISK_FILES	+= $(LOCAL_DIR)/$(LOCAL_EXE)
USER_EXES		+= $(LOCAL_DIR)/$(LOCAL_EXE)

