.PHONY: QtClean textClean QT/mixim_generator_QT textmode/mixim_generator

all: QT/mixim_generator_QT textmode/mixim_generator

clean: QtClean textClean

distclean: QtDistClean textDistClean

QT/mixim_generator_QT: QT/Makefile
	@(cd QT; ${MAKE});

textmode/mixim_generator: textmode/Makefile
	@(cd textmode; ${MAKE});

QT/Makefile:
	@(cd QT; ${MAKE} -f Makefile.cvs);

textmode/Makefile:
	@(cd textmode; ${MAKE} -f Makefile.cvs; ./configure);

QtClean:
	@(cd QT; ${MAKE} clean; rm -f mixim_generator_QT);

textClean:
	@(cd textmode; ${MAKE} clean);

QtDistClean: QtClean
	@(cd QT; ${MAKE} -f Makefile.cvs clean);

textDistClean: textClean
	@(cd textmode; ${MAKE} -f Makefile.cvs clean);
