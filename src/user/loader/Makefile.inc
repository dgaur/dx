#
# Makefile.inc for usermode loader
#

LOADER_DIR		:= loader
LOADER_BIN		:= loader.bin
LOADER_EXE		:= loader.exe
LOADER_OBJECTS	:= loader.o loader_environment.o loader_main.o

LOADER_DISASM	:= $(LOADER_BIN).disasm

LINK_EXTRA_LIBS += $(LINK_LIBTAR)
USER_EXES		+= $(LOADER_DIR)/$(LOADER_BIN)

