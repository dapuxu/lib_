#!/bin/sh


DIR_DENUG="debug"

cd src/
make;make clean;

cp ${DIR_DENUG}/*.h ../head
cd ../
