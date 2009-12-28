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
					strlen.o \
					strncat.o \
					strncpy.o \
					strrev.o \
					strspn.o \
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
# Clean the local directory
#
clean:
	@rm -f $(LIBC_FILE)
	@rm -f $(LIBC_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)
