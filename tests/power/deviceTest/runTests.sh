rm *.vec *.sca
for i in One Two Three Four Five Six Seven Eight Nine Ten Eleven Twelve Thirteen Fourteen
do
 ./deviceTest -c $i
done

