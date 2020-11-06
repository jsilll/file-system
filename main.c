#include "fs/operations.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100

int numberThreads = 0;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];

int prodptr = 0, consptr = 0, count = 0;
pthread_mutex_t commandsMutex;
pthread_cond_t podeProduzir, podeConsumir;
int endoffile = 0;

void commandsLock()
{
	if (pthread_mutex_lock(&commandsMutex) != 0)
		exit(EXIT_FAILURE);
}

void commandsUnlock()
{
	if (pthread_mutex_unlock(&commandsMutex) != 0)
		exit(EXIT_FAILURE);
}

void validateInitArgs(int argc, char *argv[])
{
	/* Usage: ./tecnicofs inputfile outputfile numthreads*/
	if (argc != 4)
	{
		fprintf(stderr, "Usage: [inputfile] [outputfile] [numthreads].\n");
		exit(EXIT_FAILURE);
	}
	else if (atoi(argv[3]) <= 0)
	{
		fprintf(stderr, "Invalid numthreads.\n");
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
		fprintf(stderr, "Error closing a file.\n");
	return 0;
}

// int getNextSlot()
// {
// 	int index = bufferTail;
// 	while (inputCommands[(index++) % MAX_COMMANDS][0] != DELETED_COMMAND)
// 	{
// 	}
// 	return index;
// }

void insertCommand(char *data)
{
	while (count == MAX_COMMANDS)
		pthread_cond_wait(&podeProduzir, &commandsMutex);

	strcpy(inputCommands[prodptr], data);
	prodptr++;
	if (prodptr == MAX_COMMANDS)
		prodptr = 0;
	count++;

	pthread_cond_signal(&podeConsumir);
}

char *removeCommand()
{
	char *res = inputCommands[consptr++];
	if (consptr == MAX_COMMANDS)
		consptr = 0;
	count--;
	return res;
}

void errorParse()
{
	fprintf(stderr, "Error: command invalid\n");
	exit(EXIT_FAILURE);
}

void *commandsProvider(void *commands_file) //Produtor
{
	char line[MAX_INPUT_SIZE];
	int n = sizeof(line) / sizeof(char);
	/* break loop with ^Z or ^D */
	while (fgets(line, n, (FILE *)commands_file))
	{
		char token, type;
		char name[MAX_INPUT_SIZE];
		int numTokens = sscanf(line, "%c %s %c", &token, name, &type);

		/* perform minimal validation */
		if (numTokens < 1)
			continue;

		switch (token)
		{
		case 'c':
			if (numTokens != 3)
				errorParse();
			insertCommand(line);
			break;
		case 'l':
			if (numTokens != 2)
				errorParse();
			insertCommand(line);
			break;
		case 'd':
			if (numTokens != 2)
				errorParse();
			insertCommand(line);
			break;
		case '#':
			break;
		default:
			errorParse();
		}
	}
	pthread_cond_signal(&podeConsumir);
	return NULL;
}

void *queueWorker()
{
	while (1)
	{
		commandsLock();
		while (count == 0 && !endoffile)
			pthread_cond_wait(&podeConsumir, &commandsMutex);

		if (count == 0 && endoffile)
		{
			pthread_cond_signal(&podeConsumir);
			commandsUnlock();
			return;
		}

		const char *command = removeCommand();
		char token, type;
		char name[MAX_INPUT_SIZE];
		int numTokens = sscanf(command, "%c %s %c", &token, name, &type);

		pthread_cond_signal(&podeProduzir);
		commandsUnlock();

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

void executeThreads(char *threads_count_char, FILE *commands_file)
{
	int i, *result;
	int threads_count = atoi(threads_count_char);
	pthread_t command_thread, worker_tid[threads_count];

	/* Creating command thread */
	if (pthread_create(&command_thread, NULL, commandsProvider, (void *)commands_file) != 0)
		fprintf(stderr, "Failed to commands thread.\n");

	/* Creating the consumer threads */
	for (i = 0; i < threads_count; i++)
	{
		if (pthread_create(&worker_tid[i], NULL, queueWorker, NULL) != 0)
			fprintf(stderr, "Failed to create thread %d.\n", i);
	}

	if (pthread_join(command_thread, (void **)&result) != 0)
		printf("Error while joining thread.\n");

	for (i = 0; i < threads_count; i++)
	{
		if (pthread_join(worker_tid[i], (void **)&result) != 0)
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

	validateInitArgs(argc, argv);
	init_fs();

	gettimeofday(&begin, 0);

	file_buffer = fopenSafe(argv[1], "r");
	executeThreads(argv[3], file_buffer);
	fcloseSafe(file_buffer);

	file_buffer = fopenSafe(argv[2], "w");
	print_tecnicofs_tree(file_buffer);
	fcloseSafe(file_buffer);
	destroy_fs();

	gettimeofday(&end, 0);
	printf("TecnicoFS completed in %.4f seconds.\n", timeDiff(&begin, &end));
	exit(EXIT_SUCCESS);
}