#!/bin/bash

# Getting the input params
INPUTDIR=$1
OUTPUTDIR=$2
MAXTHREADS=$3
CURRENTTHREADS=1

# Performing basic validation
if [ ! -d ${INPUTDIR%/} ]
then
    echo "Diretoria de input inexistente.";
    exit 1;
fi

if [ ! $MAXTHREADS -gt 0 ]
then
    echo "Mínimo número de threads é 1.";
    exit 1;
fi

if [ ! -d ${OUTPUTDIR%/} ]
then
    echo "Diretoria de output inexistente."
    exit 1;
fi

# For each test and for each number of threads in range
for x in ${INPUTDIR%/}/*.txt; 
do  
    basename="$(basename -- $x)";
    for t in $(seq 1 $MAXTHREADS);
    do
    echo InputFile=$basename NumThreads=$t;
    exec ./tecnicofs $x ${OUTPUTDIR%/}/${basename%.*}-$t.txt $t | grep "completed"; # Getting only the last line of the program. 
    done
done

exit 1;
