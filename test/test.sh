#!/bin/bash

curl -L https://raw.githubusercontent.com/mxw/grmr/master/src/finaltests/bible.txt > bible.txt

cmake -B build .. && cmake --build build

PASSWD="MonSuperMotDePasse"
echo "Monothreaded test"
echo "Cypher"
./build/aes --file_in bible.txt --file_out bible.cyphered --passwd ${PASSWD}
echo "Decypher"
./build/aes --file_in bible.cyphered --file_out bible.decyphered --passwd ${PASSWD} --decypher

diff bible.txt bible.decyphered

rm bible.decyphered

echo "Decypher multithreaded"

./build/aes --file_in bible.cyphered --file_out bible.decyphered --passwd ${PASSWD} --decypher --thread 4

diff bible.txt bible.decyphered

rm bible.decyphered bible.cyphered


echo "Multithreaded test"
echo "Cypher"
./build/aes --file_in bible.txt --file_out bible.cyphered --passwd ${PASSWD} --thread 4
echo "Decypher"
./build/aes --file_in bible.cyphered --file_out bible.decyphered --passwd ${PASSWD} --decypher --thread 4

diff bible.txt bible.decyphered

rm bible.decyphered

echo "Decypher monothreaded"

./build/aes --file_in bible.cyphered --file_out bible.decyphered --passwd ${PASSWD} --decypher 

diff bible.txt bible.decyphered

rm bible.decyphered bible.cyphered

