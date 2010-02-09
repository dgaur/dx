#
# Makefile
#

include ../Makefile.common


#
# The individual object files that comprise the math library
#
LIBM_OBJECTS	:=	libm.o

LIBM_FILE := $(notdir $(LINK_LIBM))



#
# Build the actual math library
#
all: $(LIBM_FILE)

$(LIBM_FILE): $(LIBM_OBJECTS)



#
# Generate/include the dependencies for the local objects
#
LOCAL_DEPENDENCIES := $(LIBM_OBJECTS:%.o=%.dep)
ifeq ($(filter clean, $(MAKECMDGOALS)),)
-include $(LOCAL_DEPENDENCIES)
endif



#
# Simple unit-tests.  This is mildly tricky -- use the native toolchain to
# compile a subset of the dx libm; can only link/test the standalone routines
# that do not require OS or system interaction.
#
UNIT_TEST_C_FILES := unittest.c
UNIT_TEST_ASM_FILES	:=
unittest: $(UNIT_TEST_C_FILES) $(UNIT_TEST_ASM_FILES)
	@gcc -o $@ $(CC_INCLUDES) $(UNIT_TEST_C_FILES) \
		-x assembler-with-cpp $(UNIT_TEST_ASM_FILES)


#
# Clean the local directory
#
clean:
	@rm -f $(LIBM_FILE)
	@rm -f $(LIBM_OBJECTS)
	@rm -f $(LOCAL_DEPENDENCIES)
	@rm -f unittest unittest.exe

