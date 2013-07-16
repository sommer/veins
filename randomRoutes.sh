#!/bin/bash

NETFILE="";
ROUTEFILE="";
NROUTES=5000;

SUMOTOOLS=/usr/share/sumo/tools;
DUAROUTER=duarouter;
USEDUAITERATE="0";
KEEPROUTE="0";
DUAROUTEROPS="--ignore-errors";
DUAITERATEOPTS="-L -C";

TRIPSFILE="trips.trips.xml";

HELP="-h: shows this help\n-n: netfile\n-r: name of the output route file\n-N: Number of routes to generate\n-s: path to the directory tools of sumo\n-i: If present employs duaIterate instead of duaroute";

if [[ $# -eq 0 ]]
then
  echo -e "$HELP" ; exit;
fi

set --$(getopt :hn:r:N:t:s:ik "$@")

while [ $# -gt 0 ]
do
    case "$1" in
    (-h) echo -e "$HELP" ; exit;;
    (-n) NETFILE="$2"; shift;;
    (-r) ROUTEFILE="$2"; shift;;
    (-N) NROUTES="$2"; shift;;
    (-t) TRIPGENERATOR="$2"; shift;;
    (-s) SUMOTOOLS="$2"; shift;;
    (-i) USEDUAITERATE="1";; 
    (-k) KEEPROUTE="1";;
    (--) shift; break;;
    (-*) echo "$0: error - unrecognized option $1" 1>&2; exit 1;;
    (*)  echo -e "$HELP" ; exit;;
    esac
    shift
done

TRIPGENERATOR=$SUMOTOOLS/trip/randomTrips.py;
DUAITERATE=$SUMOTOOLS/assign/duaIterate.py;

if [ ! -f $TRIPGENERATOR ]
then
	echo "Trip Generator $TRIPGENERATOR not found";
	exit -1;
fi

XMLSTARLET=$(which xmlstarlet);
if [ "$XMLSTARLET" == "" ]
then
	echo "xmlstarlet command not installed or not in the PATH";
	exit -1;
fi

ROUTES=0;
TEMPROUTES=$ROUTEFILE.temp

rm -f $TEMPROUTES $TEMPROUTES.alt $ROUTEFILE $ROUTEFILE.alt trips.trips.xml;

AWKCOMMAND='BEGIN{FS="e e";} ($1=="  <rout"){count++; print $1 "e id=\"route"count"\" e" $2;} ($1 != "  <rout"){print $0;}';

  $TRIPGENERATOR -n $NETFILE -t $TRIPSFILE -b 0 -e $NROUTES;
  
  if [ "$USEDUAITERATE" == "0" ]
  then
	echo "Using duarouter"
  	$DUAROUTER -n $NETFILE -t $TRIPSFILE -o $TEMPROUTES $DUAROUTEROPS;
  	if [ "$KEEPROUTE" == "1" ]
  	then
		cp $TEMPROUTES duaRoute.rou.xml;
  	fi
else
	echo "Using duaIterate"
	if [ ! -f $TRIPGENERATOR ]
	then
		echo "duaIterate command $DUATITERATE not found";
		exit -1;
	fi
	#$DUAITERATE -n $NETFILE -r $TEMPROUTES $DUAITERATEOPTS;
	$DUAITERATE -n $NETFILE -t $TRIPSFILE $DUAITERATEOPTS;
	mv *_049.rou.xml $TEMPROUTES;
	if [ "$KEEPROUTE" == "1" ]
	then
		cp $TEMPROUTES duaIterate.rou.xml;
	fi
	rm *_0*;
	rm dua-log.txt 
  fi
  
  xmlstarlet ed -m "//routes/vehicle/route" "//routes" $TEMPROUTES | 
  xmlstarlet ed -d "//routes/vehicle" | 
  awk "$AWKCOMMAND"  >> $ROUTEFILE;  
  ROUTES=$(xmlstarlet sel -t -v "count(/routes/route)" $ROUTEFILE);
  echo "Creadas $ROUTES rutas de $NROUTES";

rm $TEMPROUTES trips.trips.xml;
