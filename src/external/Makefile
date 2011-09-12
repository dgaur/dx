#
# Makefile
#

include ../Makefile.src


#
# Subtrees that may be built
#
LIBM := libm
LIBLUA := lua

EXTERNAL_SUBDIRS	:= $(LIBM) $(LIBLUA)


#
# By default, build the entire tree
#
.PHONY: all
all: $(EXTERNAL_SUBDIRS)


#
# Math library
#
$(LIBM): fdlibm53.tar.gz fdlibm53.patch
	@echo Unpacking libm ...
	@tar xzf $< --transform="s/fdlibm53/$(LIBM)/g"
	@cd $@ && patch -p1 < ../fdlibm53.patch
	@$(MAKE) -C $@


#
# LuaJIT library
#
$(LIBLUA): LuaJIT-1.1.7.tar.gz LuaJIT-1.1.7.patch
	@echo Unpacking lua ...
	@tar xzf $< --transform="s/LuaJIT-1.1.7/$(LIBLUA)/g"
	@cd $@ && patch -p1 < ../LuaJIT-1.1.7.patch
	@$(MAKE) -C lua/src dx CC="$(CC)" \
		MYCFLAGS="-nostdinc $(CC_DEFINES) $(CC_INCLUDES)" \
		MAKEFLAGS=



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning external tree ...
	@for dir in $(EXTERNAL_SUBDIRS); do \
		rm -rf $$dir; \
	done





