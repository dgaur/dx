#
# Makefile.inc for usermode loader
#

LOADER_DIR		:= loader
LOADER_BIN		:= loader.bin
LOADER_EXE		:= loader.exe
LOADER_OBJECTS	:= loader.o loader_environment.o loader_main.o

LOADER_DISASM	:= $(LOADER_BIN).disasm

RAMDISK_FILES	+= $(LOADER_DIR)/$(LOADER_BIN)
USER_EXES		+= $(LOADER_DIR)/$(LOADER_BIN)

