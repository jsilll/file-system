#include "fs/operations.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

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

void processInput()
{
	char line[MAX_INPUT_SIZE];
	int n = sizeof(line) / sizeof(char);
	/* break loop with ^Z or ^D */
	while (fgets(line, n, stdin))
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
		{ /* error */
			fprintf(stderr, "Error: command to apply\n");
			exit(EXIT_FAILURE);
		}
		}
	}
}

/* Usage: ./tecnicofs intputfile outputfile synchstrategy */
int main(int argc, char *argv[])
{
	struct timeval begin, end;
	gettimeofday(&begin, 0);

	validateInitArgs(argc, argv);
	/* Re-assign STD I/O  */
	/* Vou perguntar ao stor das teoricas se podemos fazer isto */
	if (!freopen(argv[1], "r", stdin))
	{
		fprintf(stderr, "Error opening input file.\n");
		exit(EXIT_FAILURE);
	}
	else if (!freopen(argv[2], "w", stdout))
	{
		fprintf(stderr, "Error opening output file.\n");
		exit(EXIT_FAILURE);
	}

	/* init filesystem */
	init_fs();
	/* process input and print tree */
	processInput();
	applyCommands();
	print_tecnicofs_tree(stdout);
	/* release allocated memory */
	destroy_fs();

	gettimeofday(&end, 0);
	long seconds = end.tv_sec - begin.tv_sec;
	long microseconds = end.tv_usec - begin.tv_usec;
	double elapsed = seconds + microseconds * 1e-6;
	printf("TecnicoFS completed in %.4f seconds.\n", elapsed);

	exit(EXIT_SUCCESS);
}
