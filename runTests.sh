#!/bin/bash

INPUTDIR=$1
OUTPUTDIR=$2
MAXTHREADS=$3
CURRENTTHREADS=1

make clean
make

for x in $INPUTDIR/*.txt; 
do  
    for t in $MAXTHREADS;
    do
    exec ./tecnicofs $x $OUTPUTDIR/$x.out $t | grep "completed"
    done
done

echo "Done."