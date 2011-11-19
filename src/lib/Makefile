#
# Makefile
#

include ../Makefile.src



#
# Libraries + subtrees that may be built
#
LIBC			:= $(subst $(DX_SRC_LIB_DIR)/,,$(LINK_LIBC))
LIBC_DIR		:= $(subst /,,$(dir $(LIBC)))

LIBDX			:= $(subst $(DX_SRC_LIB_DIR)/,,$(LINK_LIBDX))
LIBDX_DIR		:= $(subst /,,$(dir $(LIBDX)))

LIBGDTOA		:= $(subst $(DX_SRC_LIB_DIR)/,,$(LINK_LIBGDTOA))
LIBGDTOA_DIR	:= $(subst /,,$(dir $(LIBGDTOA)))

LIBLUA			:= $(subst $(DX_SRC_LIB_DIR)/,,$(LINK_LIBLUA))
LIBLUA_DIR		:= $(subst /src/,,$(dir $(LIBLUA)))

LIBM			:= $(subst $(DX_SRC_LIB_DIR)/,,$(LINK_LIBM))
LIBM_DIR		:= $(subst /,,$(dir $(LIBM)))

LIBTAR			:= $(subst $(DX_SRC_LIB_DIR)/,,$(LINK_LIBTAR))
LIBTAR_DIR		:= $(subst /,,$(dir $(LIBTAR)))


ALL_LIBS		:= $(LIBC) $(LIBDX) $(LIBGDTOA)  $(LIBLUA) $(LIBM) $(LIBTAR)
EXTERNAL_LIBS	:= $(LIBGDTOA) $(LIBLUA) $(LIBM)	# Third-party sources

LIB_DIRS		:=	$(LIBC_DIR) \
					$(LIBDX_DIR) \
					$(LIBGDTOA_DIR) \
					$(LIBLUA_DIR) \
					$(LIBM_DIR) \
					$(LIBTAR_DIR)



#
# By default, build the entire tree
#
.PHONY: all
all: $(ALL_LIBS)



#
# Basic C library
#
$(LIBC): $(LIBC_DIR)
	@$(MAKE) -C $<


#
# dx-specific system library.  Contains all system calls.
#
$(LIBDX): $(LIBDX_DIR)
	@$(MAKE) -C $<



#
# Float/double conversion library (strtod, etc).  This is third-party code;
# see README.
#
$(LIBGDTOA_DIR): gdtoa.tgz gdtoa.patch
	@echo Unpacking libgdtoa ...
	@$(TAR) xzf $< --transform="s/gdtoa/$@/"	# Only the directory name
	@cd $@ && $(PATCH) -p1 < ../gdtoa.patch

$(LIBGDTOA): $(LIBGDTOA_DIR)
	@$(MAKE) -C $<


#
# LuaJIT library.  This is third-party code; see README.
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
# Math library.  This is third-party code; see README.
#
$(LIBM_DIR): fdlibm53.tar.gz fdlibm53.patch
	@echo Unpacking libm ...
	@$(TAR) xzf $< --transform="s/fdlibm53/$@/g"
	@cd $@ && $(PATCH) -p1 < ../fdlibm53.patch

$(LIBM): $(LIBM_DIR)
	@$(MAKE) -C $<


#
# Simple library for parsing .tar files
#
$(LIBTAR): $(LIBTAR_DIR)
	@$(MAKE) -C $<



#
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning lib tree ...
	@for dir in $(LIB_DIRS); do \
		if [ -e $$dir ]; then \
			$(MAKE) -C $$dir clean; \
		fi; \
	done

.PHONY: distclean
distclean:
	@echo Cleaning lib tree ...
	@rm -rf $(EXTERNAL_DIRS)


