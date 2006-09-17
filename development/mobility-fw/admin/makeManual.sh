#!/bin/bash -xe
#
# script to make the manual and insert it to the module documenation
#
# (c) Daniel Willkomm
# (c) Andreas Koepke
#
# $1 is release name
# $2 is svntag

# create a release tag
# svn copy https://svn.tkn.tu-berlin.de/svn/mf-manual/trunk https://svn.tkn.tu-berlin.de/svn/mf-manual/tags/$2 -m "$1 released" 

# create the folder for the manual
mkdir ../../$1/doc/manual

# create ps and pdf version of the manual
make ps
make pdf

# copy pdf and ps version into the manual folder
cp mf-manual.ps mf-manual.pdf ../../$1/doc/manual/

# create html version of the manual
latex2html -dir ../../$1/doc/manual -split +1 -link 2 -show_section_numbers -up_url ../index.html -up_title Home -title "MF Manual" -index index.html -local_icons mf-manual.tex

exit 0
