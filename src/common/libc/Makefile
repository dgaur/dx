#
# Makefile
#

include ../Makefile.common


#
# The individual object files that comprise the C library
#
LIBC_OBJECTS	:=	libc.o \
					abort.o \
					atoi.o \
					clearerr.o \
					clock.o \
					ctype.o \
					difftime.o \
					errno.o \
					exit.o \
					_Exit.o \
					fclose.o \
					feof.o \
					ferror.o \
					fflush.o \
					fgetc.o \
					fopen.o \
					fprintf.o \
					fputs.o \
					fputc.o \
					fread.o \
					freopen.o \
					fwrite.o \
					getchar.o \
					getenv.o \
					gets.o \
					hal/longjmp.o \
					hal/setjmp.o \
					itoa.o \
					locale.o \
					malloc.o \
					memchr.o \
					memcmp.o \
					memcpy.o \
					memmove.o \
					memset.o \
					mktime.o \
					putchar.o \
					puts.o \
					printf.o \
					rand.o \
					remove.o \
					rename.o \
					sbrk.o \
					setvbuf.o \
					snprintf.o \
					sprintf.o \
					strcat.o \
					strchr.o \
					strcoll.o \
					strcmp.o \
					strcpy.o \
					stream.o \
					strerror.o \
					strftime.o \
					strlen.o \
					strncat.o \
					strncpy.o \
					strrchr.o \
					strrev.o \
					strspn.o \
					strstr.o \
					strtod.o \
					strtol.o \
					strtoul.o \
					system.o \
					time.o \
					tmpfile.o \
					tmpnam.o \
					uitoa.o \
					ungetc.o \
					vsnprintf.o \
					write.o

LIBC_FILE := $(notdir $(LINK_LIBC))


#
# Definitions for the Lea malloc() implementation
#
MALLOC_DEFINES	:=  -DHAVE_MMAP=0 -DHAVE_MREMAP=0 -DNO_MALLINFO \
					-DLACKS_FCNTL_H -DLACKS_SYS_PARAM_H \
					-DLACKS_SYS_MMAN_H -DLACKS_STRINGS_H

malloc.o: CC_DEFINES += $(MALLOC_DEFINES)


#
# Tool settings specific to libc.  The libc implementation here is
# incomplete; so routines here are just stubs.
#
CC_DEFINES		+= -Wno-unused-parameter
CC_FLAGS		+= -std=c99


#
# Build the actual C library
#
all: $(LIBC_FILE)

$(LIBC_FILE): $(LIBC_OBJECTS)



#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(LIBC_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif



#
# Simple unit-tests.  This is mildly tricky -- use the native toolchain to
# compile a subset of the dx libc; can only link/test the standalone routines
# that do not require OS or system interaction.
#
UNIT_TEST_C_FILES := unittest.c ctype.c errno.c memset.c mktime.c \
					 strftime.c strtod.c strtoul.c strspn.c strrchr.c strstr.c
UNIT_TEST_ASM_FILES	:= hal/setjmp.asm hal/longjmp.asm
unittest: $(UNIT_TEST_C_FILES) $(UNIT_TEST_ASM_FILES)
	@gcc -o $@ $(CC_INCLUDES) $(CC_DEFINES) $(UNIT_TEST_C_FILES) -lm


#
# Clean the local directory
#
clean:
	@rm -f $(LIBC_FILE)
	@rm -f $(LIBC_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)
	@rm -f unittest unittest.exe

