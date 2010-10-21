all: checkmakefiles
	cd src && $(MAKE)
	cd tests && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean
	cd tests && $(MAKE) clean

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	cd tests && $(MAKE) MODE=release clean
	cd tests && $(MAKE) MODE=debug clean
	rm -f src/Makefile
	rm -f tests/Makefile

makefiles:
	cd src && opp_makemake -f --deep --make-so -o mixnet -O out $$NSC_VERSION_DEF
	cd tests && opp_makemake -f --deep --make-so -o mixnet -O out $$NSC_VERSION_DEF

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '======================================================================='; \
	echo 'src/Makefile does not exist. Please use "make makefiles" to generate it!'; \
	echo '======================================================================='; \
	echo; \
	exit 1; \
	fi

doxy:
	doxygen doxy.cfg
