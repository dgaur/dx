#
# Makefile.inc for shell
#

LOCAL_DIR		:= shell
LOCAL_EXE		:= shell.exe
LOCAL_OBJECTS	:= shell.o

LINK_EXTRA_LIBS	+= $(LINK_LIBLUA) $(LINK_LIBM)
USER_EXES		+= $(LOCAL_DIR)/$(LOCAL_EXE)

