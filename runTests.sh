#!/bin/zsh

# Getting the input params
INPUTDIR=$1
OUTPUTDIR=$2
MAXTHREADS=$3

# Validacao de argumentos

# Verificacao do numero de parametros passados
if [ $# -ne 3 ]
then
    echo "Input esperado: ./runTests.sh [input_dir] [output_dir] [max_threads]";
    exit 1;
fi

# Verificar que a pasta de inputs existe
if [ ! -d ${INPUTDIR%/} ]
then
    echo "Diretoria de input inexistente.";
    exit 1;
fi

# Verificar que a pasta de outputs existe
if [ ! -d ${OUTPUTDIR%/} ]
then
    echo "Diretoria de output inexistente."
    exit 1;
fi

# Verificar que o numero de threads é maior que 0
if [ ! $MAXTHREADS -gt 0 ]
then
    echo "Mínimo número de threads é 1.";
    exit 1;
fi


# Para cada teste na diretoria e para cada numero de threads ate ao maximo
for x in ${INPUTDIR%/}/*.txt; 
do  
    basename="$(basename -- $x)";
    for t in $(seq 1 $MAXTHREADS);
    do
    echo InputFile=$basename NumThreads=$t;
    exec ./tecnicofs $x ${OUTPUTDIR%/}/${basename%.*}-$t.txt $t | grep "completed"; # Filtrar so a linha final
    done
done

exit 1;
