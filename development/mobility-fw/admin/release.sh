#!/bin/bash
#
# script to create a release for the MF
#
# (c) Daniel Willkomm
# (c) Andreas Koepke (adapted to svn)


if [ ! -f mobility-fw/trunk/Version ]; then
    echo 'Must be called in parent dir of mobility-fw dir!'
    exit 1
fi

# cvstag and release names...
relname=`cat mobility-fw/trunk/Version`
svntag=`echo $relname | sed 's/\./_/g'`

# check tag format
if echo $relname | grep -q '^mobility-fw[0-9]\+\.[0-9]\+[abp]\?[0-9]*$'; then
    echo "About to tag with the following data:"
    echo "  Release: $relname"
    echo "  svn tag: $svntag"
    echo "Enter to continue..."
    read tmp
else
    echo "wrong format in Version file, should look like this: mobility-fw3.0b1"
    exit 1
fi

# first add a release line to the ChangeLog files
# and tag the module
(cd mobility-fw/trunk && admin/releaseTag.sh)

# next export the module
svn export https://svn.tkn.tu-berlin.de/svn/mobility-fw/tags/$svntag $relname 

# remove the admin dir
rm -r $relname/admin

# make the documentation
(cd $relname && opp_makemake -n -Xadmin -Xbitmaps -Xdoc -Xtemplate && make docs && rm Makefile)

# tag the manual and add it to the module
(cd mf-manual/trunk && ../../mobility-fw/trunk/admin/makeManual.sh $relname $svntag)

# pack
tar cvzf $relname.tgz $relname/
zip -9 -r $relname.zip $relname/

exit 0
