for f in *.vec 
do
 cp $f valid/$f
done
cd results
for f in *.sca
do
 cp $f ../valid/$f
done
