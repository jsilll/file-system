/* ======================================
Sistemas Operativos 17/18
Aula teórica 1: Exemplo simples de programação 
multi-tarefa com pthreads construído durante a
aula

Este ficheiro contém a solução multi-tarefa
final.

Para compilar:

gcc -c main-par.c
gcc -pthread -o main-par main-par.o


=============================== */

#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *runQueue()
{
    printf("Mequie maluquilsons\n");
}

int main(int argc, char const *argv[])
{
    int *result;
    pthread_t tid;
    pthread_create(&tid, NULL, runQueue, NULL);
    pthread_join(tid, (void **)&result);
    return 0;
}