for f in *.vec 
do
 echo $f diff:
 diff -B -I '^version.*' -I '^run.*' -I '^attr.*' -I '^vector.*' $f valid/$f
done
cd results
for f in *.sca
do
 echo $f diff:
 diff -B -I '^version.*' -I '^run.*' -I '^attr.*' -I '^vector.*' $f ../valid/$f
done
