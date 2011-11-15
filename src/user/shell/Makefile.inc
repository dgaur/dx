#
# Makefile.inc for shell
#

LOCAL_DIR		:= shell
LOCAL_EXE		:= shell.exe
LOCAL_OBJECTS	:= shell.o

RAMDISK_FILES	+= $(LOCAL_DIR)/$(LOCAL_EXE)
USER_EXES		+= $(LOCAL_DIR)/$(LOCAL_EXE)

