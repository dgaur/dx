#
# Makefile
#

include ../Makefile.src



#
# Libraries + subtrees that may be built
#
LIBGDTOA		:= $(subst $(DX_SRC_EXTERNAL_DIR)/,,$(LINK_LIBGDTOA))
LIBGDTOA_DIR	:= $(subst /,,$(dir $(LIBGDTOA)))

LIBLUA			:= $(subst $(DX_SRC_EXTERNAL_DIR)/,,$(LINK_LIBLUA))
LIBLUA_DIR		:= $(subst /src/,,$(dir $(LIBLUA)))

LIBM			:= $(subst $(DX_SRC_EXTERNAL_DIR)/,,$(LINK_LIBM))
LIBM_DIR		:= $(subst /,,$(dir $(LIBM)))

EXTERNAL_DIRS	:= $(LIBGDTOA_DIR) $(LIBM_DIR) $(LIBLUA_DIR)
EXTERNAL_LIBS	:= $(LIBGDTOA) $(LIBM) $(LIBLUA)



#
# By default, build the entire tree
#
.PHONY: all
all: $(EXTERNAL_LIBS)



#
# Float/double conversion library (strtod, etc)
#
$(LIBGDTOA_DIR): gdtoa.tgz gdtoa.patch
	@echo Unpacking libgdtoa ...
	@$(TAR) xzf $< --transform="s/gdtoa/$@/"	# Only the directory name
	@cd $@ && $(PATCH) -p1 < ../gdtoa.patch

$(LIBGDTOA): $(LIBGDTOA_DIR)
	@$(MAKE) -C $<


#
# Math library
#
$(LIBM_DIR): fdlibm53.tar.gz fdlibm53.patch
	@echo Unpacking libm ...
	@$(TAR) xzf $< --transform="s/fdlibm53/$@/g"
	@cd $@ && $(PATCH) -p1 < ../fdlibm53.patch

$(LIBM): $(LIBM_DIR)
	@$(MAKE) -C $<


#
# LuaJIT library
#
$(LIBLUA_DIR): LuaJIT-1.1.7.tar.gz LuaJIT-1.1.7.patch
	@echo Unpacking lua ...
	@$(TAR) xzf $< --transform="s/LuaJIT-1.1.7/$@/g"
	@cd $@ && $(PATCH) -p1 < ../LuaJIT-1.1.7.patch

$(LIBLUA): $(LIBLUA_DIR)
	@$(MAKE) -C $</src dx CC="$(CC)" \
		MYCFLAGS="-nostdinc $(CC_DEFINES) $(CC_INCLUDES)" \
		MAKEFLAGS=



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning external tree ...
	@for dir in $(EXTERNAL_DIRS); do \
		if [ -e $$dir ]; then \
			$(MAKE) -C $$dir clean; \
		fi; \
	done

.PHONY: distclean
distclean:
	@echo Cleaning external tree ...
	@rm -rf $(EXTERNAL_DIRS)


