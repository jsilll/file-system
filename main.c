#include "fs/operations.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

pthread_mutex_t commandsMutex = PTHREAD_MUTEX_INITIALIZER;

void validateInitArgs(int argc, char *argv[])
{
	/* Usage: ./tecnicofs inputfile outputfile numthreads synchstrategy */
	if (argc != 5)
	{
		fprintf(stderr, "Usage: [inputfile] [outputfile] [numthreads] [synchstrategy].\n");
		exit(EXIT_FAILURE);
	}
	else if (atoi(argv[3]) <= 0)
	{
		fprintf(stderr, "Invalid numthreads.\n");
		exit(EXIT_FAILURE);
	}
	/* Synchstrategy */
	else if (strcmp(argv[4], "mutex") && strcmp(argv[4], "rwlock") && strcmp(argv[4], "nosync"))
	{
		fprintf(stderr, "Invalid synchstrategy.\n");
		exit(EXIT_FAILURE);
	}
	else if (!strcmp(argv[4], "nosync") && atoi(argv[3]) != 1)
	{
		fprintf(stderr, "Synchstrategy 'nosync' only allowed with 1 thread.\n");
		exit(EXIT_FAILURE);
	}
}

FILE *fopenSafe(char *file_name, const char *mode)
{
	FILE *fp = fopen(file_name, mode);
	if (!fp)
	{
		switch (mode[0])
		{
		case 'r':
			fprintf(stderr, "Error opening input file: %s.\n", file_name);
			break;
		case 'w':
			fprintf(stderr, "Error opening output file: %s.\n", file_name);
			break;
		default:
			fprintf(stderr, "Error opening a file: %s\n", file_name);
			break;
		}
		exit(EXIT_FAILURE);
	}
	return fp;
}

int fcloseSafe(FILE *file)
{
	if (fclose(file) == EOF)
	{
		fprintf(stderr, "Error closing a file.\n");
	}
	return 0;
}

int insertCommand(char *data)
{
	if (numberCommands != MAX_COMMANDS)
	{
		strcpy(inputCommands[numberCommands++], data);
		return 1;
	}
	return 0;
}

char *removeCommand()
{
	if (numberCommands > 0)
	{
		numberCommands--;
		return inputCommands[headQueue++];
	}
	return NULL;
}

void errorParse()
{
	fprintf(stderr, "Error: command invalid\n");
	exit(EXIT_FAILURE);
}

void processInput(FILE *commands_file)
{
	char line[MAX_INPUT_SIZE];
	int n = sizeof(line) / sizeof(char);
	/* break loop with ^Z or ^D */

	while (fgets(line, n, commands_file))
	{
		char token, type;
		char name[MAX_INPUT_SIZE];
		int numTokens = sscanf(line, "%c %s %c", &token, name, &type);
		/* perform minimal validation */
		if (numTokens < 1)
		{
			continue;
		}
		switch (token)
		{
		case 'c':
			if (numTokens != 3)
				errorParse();
			if (insertCommand(line))
				break;
			return;
		case 'l':
			if (numTokens != 2)
				errorParse();
			if (insertCommand(line))
				break;
			return;
		case 'd':
			if (numTokens != 2)
				errorParse();
			if (insertCommand(line))
				break;
			return;
		case '#':
			break;
		default:
			errorParse();
		}
	}
}

void *queueWorker(void *syncstrat)
{
	while (numberCommands > 0)
	{
		pthread_mutex_lock(&commandsMutex);
		const char *command = removeCommand();
		pthread_mutex_unlock(&commandsMutex);
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
	return NULL;
}

void generateThreads(char *threads_count_char, char *syncstrat)
{
	int i, *result;
	int threads_count = atoi(threads_count_char);
	pthread_t tid[threads_count];
	for (i = 0; i < threads_count; i++)
	{
		if (pthread_create(&tid[i], NULL, queueWorker, syncstrat) != 0)
			fprintf(stderr, "Failed to create thread %d.\n", i);
	}
	for (i = 0; i < threads_count; i++)
	{
		if (pthread_join(tid[i], (void **)&result) != 0)
			printf("Error while joining thread.\n");
	}
}

double timeDiff(struct timeval *begin, struct timeval *end)
{
	long seconds = end->tv_sec - begin->tv_sec;
	long microseconds = end->tv_usec - begin->tv_usec;
	double elapsed = seconds + microseconds * 1e-6;
	return elapsed;
}

int main(int argc, char *argv[])
{
	struct timeval begin, end;
	FILE *file_buffer;
	gettimeofday(&begin, 0);
	validateInitArgs(argc, argv);
	init_fs();
	file_buffer = fopenSafe(argv[1], "r");
	processInput(file_buffer);
	fcloseSafe(file_buffer);
	file_buffer = fopenSafe(argv[2], "w");
	generateThreads(argv[3], argv[4]);
	print_tecnicofs_tree(file_buffer);
	destroy_fs();
	gettimeofday(&end, 0);
	fprintf(file_buffer, "TecnicoFS completed in %.4f seconds.\n", timeDiff(&begin, &end));
	fcloseSafe(file_buffer);
	exit(EXIT_SUCCESS);
}
