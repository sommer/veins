rm *.vec *.sca
for i in One Two Three Four Five Six Seven Eight Nine Ten
do
 ./Nic80211Test -u Cmdenv -c $i
done

