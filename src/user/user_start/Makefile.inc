#
# Makefile.inc for user_start library
#

LOCAL_DIR		:= user_start
LOCAL_LIB		:= user_start.o
LOCAL_OBJECTS	:= setup_main.o start.o 

USER_LIBS		+= $(LOCAL_DIR)/$(LOCAL_LIB)
