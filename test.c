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

#define THREADS_COUNT 4

int computacaoDemorada(int x)
{
    sleep(10);
    return x * 100 + 1;
}

void *fnThread(void *arg)
{
    int *x, *r;
    x = (int *)arg;
    r = (int *)malloc(sizeof(int));

    *r = computacaoDemorada(*x);
    return r;
}

//Variavel global
int args[THREADS_COUNT];

int main()
{

    int i;
    int *result;
    pthread_t tid[THREADS_COUNT];
    for (i = 0; i < THREADS_COUNT; i++)
    {
        args[i] = i;
        if (pthread_create(&tid[i], NULL, fnThread, &args[i]) != 0)
        {
            printf("Erro ao criar tarefa.\n");
            return 1;
        }
        printf("Lancou uma tarefa\n");
    }

    for (i = 0; i < THREADS_COUNT; i++)
    {
        if (pthread_join(tid[i], (void **)&result) != 0)
        {
            printf("Erro ao esperar por tarefa.\n");
            return 2;
        }
        printf("Tarefa retornou com resultado = %d\n", *result);
    }

    return 0;
}

/* ===================================================================== */

void applyCommands()
{
    while (numberCommands > 0)
    {
        const char *command = removeCommand();
        if (command == NULL)
        {
            continue;
        }
        char token, type;
        char name[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
        if (numTokens < 2)
        {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }
        int searchResult;
        switch (token)
        {
        case 'c':
            switch (type)
            {
            case 'f':
                printf("Create file: %s\n", name);
                create(name, T_FILE);
                break;
            case 'd':
                printf("Create directory: %s\n", name);
                create(name, T_DIRECTORY);
                break;
            default:
                fprintf(stderr, "Error: invalid node type\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'l':
            searchResult = lookup(name);
            if (searchResult >= 0)
                printf("Search: %s found\n", name);
            else
                printf("Search: %s not found\n", name);
            break;
        case 'd':
            printf("Delete: %s\n", name);
            delete (name);
            break;
        default:
        {
            fprintf(stderr, "Error: command to apply\n");
            exit(EXIT_FAILURE);
        }
        }
    }
}

for (i = 0; i < threads_count; i++)
{
    printf("nah valdir");
    if (pthread_create(&tid[i], NULL, runQueue, NULL) != 0)
        fprintf(stderr, "Failed to create thread number %d.\n", i); /* WIP Synchcoiso */
    printf("Thread number %d has been successfully created\n", i);
}

for (i = 0; i < threads_count; i++)
{
    if (pthread_join(tid[i], (void **)&result) != 0)
        printf("Erro ao esperar por tarefa.\n");
    printf("Tarefa retornou com resultado = %d\n", *result);
}