#
# Makefile.inc for VGA driver
#

LOCAL_DIR		:= vga
LOCAL_EXE		:= vga.exe
LOCAL_OBJECTS	:= vga.o

RAMDISK_FILES	+= $(LOCAL_DIR)/$(LOCAL_EXE)
USER_EXES		+= $(LOCAL_DIR)/$(LOCAL_EXE)
