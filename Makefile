.PHONY: all makefiles clean cleanall doxy

# if out/config.py exists, we can also create command line scripts for running simulations
ADDL_TARGETS =
ifeq ($(wildcard out/config.py),)
else
    ADDL_TARGETS += run
endif

# default target
all: src/Makefile $(ADDL_TARGETS)
ifdef MODE
	@cd src && $(MAKE)
else
	@cd src && $(MAKE) MODE=release
	@cd src && $(MAKE) MODE=debug
endif

# command line scripts
run: % : src/scripts/%.in.py out/config.py
	@echo "Creating script \"./$@\""
	@head -n1 "$<" > "$@"
	@cat out/config.py >> "$@"
	@tail -n+2 "$<" >> "$@"
	@chmod a+x "$@"

# legacy
makefiles:
	@echo
	@echo '====================================================================='
	@echo 'Warning: make makefiles has been deprecated in favor of ./configure'
	@echo '====================================================================='
	@echo
	./configure
	@echo
	@echo '====================================================================='
	@echo 'Warning: make makefiles has been deprecated in favor of ./configure'
	@echo '====================================================================='
	@echo

clean: src/Makefile
ifdef MODE
	@cd src && $(MAKE) clean
else
	@cd src && $(MAKE) MODE=release clean
	@cd src && $(MAKE) MODE=debug clean
endif

cleanall: clean
	rm -f src/Makefile
	rm -f out/config.py
	rm -f run

src/Makefile:
	@echo
	@echo '====================================================================='
	@echo '$@ does not exist.'
	@echo 'Please run "./configure" or use the OMNeT++ IDE to generate it.'
	@echo '====================================================================='
	@echo
	@exit 1

out/config.py:
	@echo
	@echo '====================================================================='
	@echo '$@ does not exist.'
	@echo 'Please run "./configure" to generate it.'
	@echo '====================================================================='
	@echo
	@exit 1

# autogenerated documentation
doxy:
	doxygen doxy.cfg

doxyshow: doxy
	xdg-open doc/doxy/html/index.html

