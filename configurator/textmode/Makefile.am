bin_PROGRAMS=mixim_generator
mixim_generator_SOURCES=mixim_generator.c ../miximConfiguratorCommon.c mixim_generator.1

mixim_generator_CFLAGS = -Wall -Werror

#  uncomment the following if mixim_generator requires the math library
#mixim_generator_LDADD=-lm

EXTRA_DIST=mixim_generator.lsm.in mixim_generator.spec.in mixim_generator.texinfo

#  if you write a self-test script named `chk', uncomment the
#  following and add `chk' to the EXTRA_DIST list
#TESTS=chk

#  build and install the .info pages
info_TEXINFOS = mixim_generator.texinfo
mixim_generator_TEXINFOS = gpl.texinfo

#  install the man pages
man_MANS=mixim_generator.1
