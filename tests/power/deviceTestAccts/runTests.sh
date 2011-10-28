#/bin/bash

lPATH='.'
LIBSREF=( )
lINETPath='../../../../inet/src'
for lP in '../../../src' \
          '../../../src/base' \
          '../../../src/modules' \
          '../../testUtils' \
          '../utils' \
          "$lINETPath"; do
    for pr in 'mixim' 'inet' 'powerTestUtils'; do
        if [ -d "$lP" ] && [ -f "${lP}/lib${pr}$(basename $lP).so" -o -f "${lP}/lib${pr}$(basename $lP).dll" ]; then
            lPATH="${lP}:$lPATH"
            LIBSREF=( '-l' "${lP}/${pr}$(basename $lP)" "${LIBSREF[@]}" )
        elif [ -d "$lP" ] && [ -f "${lP}/lib${pr}.so" -o -f "${lP}/lib${pr}.dll" ]; then
            lPATH="${lP}:$lPATH"
            LIBSREF=( '-l' "${lP}/${pr}" "${LIBSREF[@]}" )
        fi
    done
done
PATH="${PATH}:${lPATH}" #needed for windows
LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${lPATH}"
NEDPATH="../../../src:../.."
[ -d "$lINETPath" ] && NEDPATH="${NEDPATH}:$lINETPath"
export PATH
export NEDPATH
export LD_LIBRARY_PATH

lCombined='miximtests'
lSingle='deviceTestAccts'
if [ ! -e "${lSingle}" -a ! -e "${lSingle}.exe" ]; then
    if [ -e "../../${lCombined}.exe" ]; then
        ln -s "../../${lCombined}.exe" "${lSingle}.exe"
    elif [ -e "../../${lCombined}" ]; then
        ln -s "../../${lCombined}"     "${lSingle}"
    fi
fi
          
rm *.vec results/*.sca 2>/dev/null
for i in One Two 
do
 ./${lSingle} -c $i "${LIBSREF[@]}"
done

