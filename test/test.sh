#!/bin/bash

curl -L https://raw.githubusercontent.com/mxw/grmr/master/src/finaltests/bible.txt > bible.txt 2>/dev/null

if [ -f bible.txt ]
then
    echo "Download test file: OK"
else
    echo "Download test file: NG"
    exit 1
fi

res=0
cmake -B build .. && cmake --build build

if [ -f build/aes ]
then
    echo "Build binary: OK"
else
    echo "Build binary: NG"
    exit 1
fi

PASSWD="MonSuperMotDePasse"
printf "Monothread cypher "
./build/aes --file_in bible.txt --file_out bible.cyphered --passwd ${PASSWD}
printf "monothread decypher: "
./build/aes --file_in bible.cyphered --file_out bible.decyphered --passwd ${PASSWD} --decypher

diff_result=$(diff bible.txt bible.decyphered)
if [[ -z "${diff_result}" ]]
then
    echo "OK"
else
    echo NG
    res=$(($res +1))
fi

rm bible.decyphered

printf "Monothread cypher multithread decypher: "

./build/aes --file_in bible.cyphered --file_out bible.decyphered --passwd ${PASSWD} --decypher --thread 4

diff_result=$(diff bible.txt bible.decyphered)
if [[ -z "${diff_result}" ]]
then
    echo "OK"
else
    echo NG
    res=$(($res +1))
fi

rm bible.decyphered bible.cyphered


printf "Multithread cypher "
./build/aes --file_in bible.txt --file_out bible.cyphered --passwd ${PASSWD} --thread 4
printf "multithread decypher: "
./build/aes --file_in bible.cyphered --file_out bible.decyphered --passwd ${PASSWD} --decypher --thread 3

diff_result=$(diff bible.txt bible.decyphered)
if [[ -z "${diff_result}" ]]
then
    echo "OK"
else
    echo NG
    res=$(($res +1))
fi

rm bible.decyphered

printf "Multithread cypher monothread decypher: "

./build/aes --file_in bible.cyphered --file_out bible.decyphered --passwd ${PASSWD} --decypher 
diff_result=$(diff bible.txt bible.decyphered)
if [[ -z "${diff_result}" ]]
then
    echo "OK"
else
    echo NG
    res=$(($res +1))
fi

rm -fr bible.txt build bible.decyphered bible.cyphered

exit $res

