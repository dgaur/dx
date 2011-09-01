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
					ctype.o \
					errno.o \
					exit.o \
					_Exit.o \
					fprintf.o \
					getchar.o \
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
					putchar.o \
					puts.o \
					printf.o \
					rand.o \
					sbrk.o \
					snprintf.o \
					sprintf.o \
					strcat.o \
					strchr.o \
					strcoll.o \
					strcmp.o \
					strcpy.o \
					strerror.o \
					strlen.o \
					strncat.o \
					strncpy.o \
					strrev.o \
					strspn.o \
					strtod.o \
					strtol.o \
					strtoul.o \
					uitoa.o \
					vsnprintf.o

LIBC_FILE := $(notdir $(LINK_LIBC))



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
UNIT_TEST_C_FILES := unittest.c ctype.c errno.c memset.c \
					 strtod.c strtoul.c strspn.c
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

