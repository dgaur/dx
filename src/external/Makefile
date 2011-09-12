#
# Makefile
#


#
# Subtrees that may be built
#
LIBM := libm

EXTERNAL_SUBDIRS	:= $(LIBM)


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
# Clean the tree
#
.PHONY: clean
clean:
	@echo Cleaning external tree ...
	@for dir in $(EXTERNAL_SUBDIRS); do \
		rm -rf $$dir; \
	done





