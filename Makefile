MAKEFILES = base/Makefile

help:
	@echo
	@echo "*******************************************************************"
	@echo "	Targets are:"
	@echo "	base			-- create base"
	@echo "	modules			-- create modules"
	@echo "	lib        	 	-- create mixim library (base and modules)"
	@echo "	examples		-- create example networks "
	@echo "	configurator 	-- create the configuration tool "
	@echo "	tests			-- create tests "
	@echo "	all				-- create all of the above "
	@echo "	docs			-- create documentation "
	@echo "	clean			-- clean  everything up "
	@echo "	distclean		-- get back to pristine fresh state "
	@echo "*******************************************************************"

# make all
.PHONY: all base modules lib examples tests configurator

all: base modules lib examples tests #configurator

lib: modules

${MAKEFILES}:
	sh mkmk

base: ${MAKEFILES}
	@(cd base; ${MAKE})
	@(cd include; ${MAKE})
	
modules: base
	@(cd modules; ${MAKE})
	
tests: base
	@(cd tests; ${MAKE})
	
examples: modules
	@(cd examples; ${MAKE})
	
clean:
	@(cd base; ${MAKE} clean)
	@(cd include; ${MAKE} clean)
	@(cd modules; ${MAKE} clean)
	@(cd tests; ${MAKE} clean)
	@(cd examples; ${MAKE} clean)

configurator:
	@(cd configurator; ${MAKE})

distclean: clean
	@cd configurator &&  $(MAKE) distclean
	@rm -rf doc/api
	@rm -rf doc/neddoc
	@rm -f doc/miximtags.xml

# add dependencies:
modules: base
include: modules
lib: include
bin: lib
examples: lib
tests: lib

# documentation
DOC_DIR=doc

# generate all docs (API and neddoc reference)
docs: doxy neddoc

# generate doxygen API reference
doxy: $(DOC_DIR)/doxy.cfg
	doxygen $(DOC_DIR)/doxy.cfg

# generate neddoc documentation linked to doxygen API reference
neddoc:
	opp_neddoc -o $(DOC_DIR)/neddoc -t $(DOC_DIR)/miximtags.xml -d ../api \
	base/ examples/ modules/ doc/

neddoc-without-doxy:
	opp_neddoc -o $(DOC_DIR)/neddoc -d ../api base/ examples/ modules/ doc/


