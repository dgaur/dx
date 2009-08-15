#
# Makefile
#

include ../Makefile.user


START_OBJECT = $(notdir $(LINK_START_FILE))


all: $(START_OBJECT)


# Link the startup files into a single object
$(START_OBJECT): setup_main.o start.o 
	@echo Linking $(START_OBJECT) ...
	@$(LINK) -r $(LINK_FLAGS) $^


clean:
	@rm -f *.o
