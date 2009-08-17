rm *.vec results/*.sca
for i in One Two 
do
 ./deviceTestAccts -c $i
done

