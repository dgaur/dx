#
# Makefile
#

include ../Makefile.user
include Makefile.inc


all: $(LOCAL_LIB)


# Link the startup files into a single object
$(LOCAL_LIB): $(LOCAL_OBJECTS)
	@echo Linking $@
	@$(LINK) -r $(LINK_FLAGS) $^


clean:
	@rm -f *.o
