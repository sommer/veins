.PHONY: all makefiles clean cleanall doxy

all: checkmakefiles run debug memcheck
	@cd src && $(MAKE)

makefiles:
	./configure

clean: checkmakefiles
	cd src && $(MAKE) clean
	rm -f run debug memcheck

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	rm -f src/Makefile
	rm -f run debug memcheck

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	echo; \
	echo '====================================================================='; \
	echo 'src/Makefile does not exist. Please use "./configure" to generate it!'; \
	echo '====================================================================='; \
	echo; \
	exit 1; \
	fi

run debug memcheck: % : src/scripts/%.in.py out/config.py
	@echo "Creating script \"./$@\""
	@head -n1 "$<" > "$@"
	@cat out/config.py >> "$@"
	@tail -n+2 "$<" >> "$@"
	@chmod a+x "$@"

doxy:
	doxygen doxy.cfg
