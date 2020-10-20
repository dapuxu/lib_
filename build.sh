#!/bin/sh


DIR_DENUG="debug"
DIR_LIST="list"
DIR_NET="network"
DIR_STR="string"

cd src/
make clean;make all

cd ../demo
make clean;make all

cd ..
