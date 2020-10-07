#include "fs/operations.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100

int numberThreads = 0;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];
int numberCommands = 0;
int headQueue = 0;

int insertCommand(char *data) {
	if (numberCommands != MAX_COMMANDS) {
		strcpy(inputCommands[numberCommands++], data);
		return 1;
	}
	return 0;
}

char *removeCommand() {
	if (numberCommands > 0) {
		numberCommands--;
		return inputCommands[headQueue++];
	}
	return NULL;
}

void errorParse() {
	fprintf(stderr, "Error: command invalid\n");
	exit(EXIT_FAILURE);
}

void processInput() {
	char line[MAX_INPUT_SIZE];
	int n = sizeof(line) / sizeof(char);
	/* break loop with ^Z or ^D */
	while (fgets(line, n, stdin)) {
		char token, type;
		char name[MAX_INPUT_SIZE];
		int numTokens = sscanf(line, "%c %s %c", &token, name, &type);
		/* perform minimal validation */
		if (numTokens < 1) {
			continue;
		}
		switch (token) {
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

void applyCommands() {
	while (numberCommands > 0) {
		const char *command = removeCommand();
		if (command == NULL) {
			continue;
		}
		char token, type;
		char name[MAX_INPUT_SIZE];
		int numTokens = sscanf(command, "%c %s %c", &token, name, &type);
		if (numTokens < 2) {
			fprintf(stderr, "Error: invalid command in Queue\n");
			exit(EXIT_FAILURE);
		}
		int searchResult;
		switch (token) {
			case 'c':
				switch (type) {
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
				delete(name);
				break;
			default: { /* error */
				fprintf(stderr, "Error: command to apply\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

/* Usage: ./tecnicofs intputfile outputfile synchstrategy */
int main(int argc, char *argv[]) {
	/* Basic Params Validation */
	if (argc != 4) {
		exit(EXIT_FAILURE);
	}
	freopen(argv[1], "r", stdin);
	freopen(argv[2], "w", stdout);
	printf("TecnicoFS initialized in %s mode\n", argv[3]);
	/* init filesystem */
	init_fs();
	/* process input and print tree */
	processInput();
	applyCommands();
	print_tecnicofs_tree(stdout);
	/* release allocated memory */
	destroy_fs();
	exit(EXIT_SUCCESS);
}
