#
# Makefile.inc for libtar
#

LOCAL_DIR		:= libtar
LOCAL_LIB		:= libtar.a
LOCAL_OBJECTS	:= libtar.o

USER_LIBS		+= $(LOCAL_DIR)/$(LOCAL_LIB)
