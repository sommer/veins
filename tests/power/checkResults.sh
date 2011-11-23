for f in *.vec 
do
 if [ -f "$f" ]; then
  echo "diff $f valid/$f"
  diff -I '^version' -I '^run' -I '^attr' -I '^vector' -w "$f" "valid/$f"
  rm -f "$f"
 fi
done
if [ -d results ]; then
 cd results
 for f in *.sca
 do
  if [ -f "$f" ]; then
   echo "diff $f ../valid/$f"
   diff -I '^version' -I '^run' -I '^attr' -I '^vector' -w "$f" "../valid/$f"
  fi
 done
 cd ..
 rm -rf results
fi