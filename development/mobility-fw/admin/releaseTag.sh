#!/bin/bash
# script to tag the svn module
#
# author: Andras Varga
# adapted by Daniel Willkomm
# adapted by Andreas Koepke
#

# svntag and release names...
relname=`cat Version`
svntag=`echo $relname | sed 's/\./_/g'`



# update ChangeLog files. (remove earlier marker line if exists, plus blank line below it)
today=`date '+20%y-%m-%d'`
for i in `find . -name ChangeLog`; do
    line="$today  ------ $relname released ------"
    echo "$line" > $i.b
    echo "" >> $i.b
    sed "/$line/{N;d;}" $i >> $i.b || exit 1
    mv $i.b $i || exit 1
done

# commit ChangeLog 
svn commit -m "--$relname released-- line added to ChangeLog by _admin/releaseTag.sh"

# tag it in svn
svn copy https://svn.tkn.tu-berlin.de/svn/mobility-fw/trunk https://svn.tkn.tu-berlin.de/svn/mobility-fw/tags/$svntag -m "$relname released" 

exit 0
