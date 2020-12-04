#include <ctype.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <unistd.h>
#include "fs/operations.h"

#define MAX_INPUT_SIZE 100

/*
 * Validates the initial arguments for the program.
 * Input:
 *  - argc: number of arguments in argv
 *  - argv: array passed arguments
 */
void validateInitArgs(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: [numthreads] [nomesocket].\n");
    exit(EXIT_FAILURE);
  }
  else if (atoi(argv[1]) <= 0)
  {
    fprintf(stderr, "Invalid numthreads.\n");
    exit(EXIT_FAILURE);
  }
}

/*
 * Responsible for parsing, a command with an error.
 */
void errorParse()
{
  fprintf(stderr, "Error: command invalid\n");
  exit(EXIT_FAILURE);
}

/*
 * Sends response to the client according to the result of the operation asked by the client.
 * Input:
 *  - sockfd: sock file descriptor for the client
 *  - response_code: response code that should be sent to the client
 *  - client_addr: client address
 *  - addrelen: client address length
 */
void sendResponse(int sockfd, int response_code, struct sockaddr_un *client_addr, socklen_t addrlen)
{
  if (sendto(sockfd, &response_code, sizeof(int), 0, (struct sockaddr *)client_addr, addrlen) < 0)
    perror("server: sendto error");
}

/*
 * Waits for a any command, that should be sent by a mounted client
 * Input:
 *  - arg: socket file descriptor for the server 
 */
void *consumerThread(void *arg)
{
  int sockfd = *(int *)arg;
  int numArgs;
  char token;
  char command[MAX_INPUT_SIZE];
  char arg1[MAX_INPUT_SIZE];
  char arg2[MAX_INPUT_SIZE];

  struct sockaddr_un client_addr;
  socklen_t addrlen;
  int c;
  int searchResult;
  FILE *fp;
  addrlen = sizeof(struct sockaddr_un);

  while (1)
  {
    c = recvfrom(sockfd, command, sizeof(command) - 1, 0, (struct sockaddr *)&client_addr, &addrlen);
    if (c <= 0)
      continue;
    command[c] = '\0';
    numArgs = sscanf(command, "%c %s %s", &token, arg1, arg2);
    if (numArgs < 2)
    {
      sendResponse(sockfd, TECNICOFS_ERROR_OTHER, &client_addr, addrlen);
      continue;
    }
    switch (token)
    {
    case 'c':
      switch (arg2[0])
      {
      case 'f':
        printf("Create file: %s\n", arg1);
        sendResponse(sockfd, create(arg1, T_FILE), &client_addr, addrlen);
        break;
      case 'd':
        printf("Create directory: %s\n", arg1);
        sendResponse(sockfd, create(arg1, T_DIRECTORY), &client_addr, addrlen);
        break;
      default:
        fprintf(stderr, "Error: invalid node type\n");
        sendResponse(sockfd, TECNICOFS_ERROR_INVALID_NODE_TYPE, &client_addr, addrlen);
      }
      break;
    case 'm':
      printf("Move file: %s to %s\n", arg1, arg2);
      sendResponse(sockfd, move(arg1, arg2), &client_addr, addrlen);
      break;
    case 'l':
      searchResult = lookup(arg1);
      if (searchResult >= 0)
      {
        printf("Search: %s found\n", arg1);
        sendResponse(sockfd, searchResult, &client_addr, addrlen);
      }
      else
      {
        printf("Search: %s not found\n", arg1);
        sendResponse(sockfd, searchResult, &client_addr, addrlen);
      }
      break;
    case 'd':
      printf("Delete: %s\n", arg1);
      sendResponse(sockfd, delete (arg1), &client_addr, addrlen);
      break;
    case 'p':
      printf("Print: %s\n", arg1);
      fp = fopen(arg1, "w");
      if (fp == NULL)
      {
        sendResponse(sockfd, TECNICOFS_ERROR_FILE_NOT_OPEN, &client_addr, addrlen);
        break;
      }
      print_tecnicofs_tree(fp);
      fclose(fp); /* If function fails, server must continue */
      sendResponse(sockfd, SUCCESS, &client_addr, addrlen);
      break;
    default:
    {
      fprintf(stderr, "Error: command to apply\n");
      sendResponse(sockfd, TECNICOFS_ERROR_OTHER, &client_addr, addrlen);
    }
    }
  }
}

/*
 * Creates all the threads.
 * Input:
 *  - threads_count_char: number of threads to be created
 *  - sockfd: socket file descriptor for the server
 */
void executeThreads(char *threads_count_char, int sockfd)
{
  int i, *result;
  int threads_count = atoi(threads_count_char);
  pthread_t tid[threads_count];
  for (i = 0; i < threads_count; i++)
  {
    if (pthread_create(&tid[i], NULL, consumerThread, (void *)&sockfd) != 0)
    {
      fprintf(stderr, "Failed to create a thread %d.\n", i);
      exit(EXIT_FAILURE);
    }
  }

  for (i = 0; i < threads_count; i++)
  {
    if (pthread_join(tid[i], (void **)&result) != 0)
      printf("Error while joining thread.\n");
  }
}

/*
 * Initializes the socket sockaddr_un struct
 * Input:
 *  - path: path for the file associated with the socket
 *  - addr: socket struct for the server
 */
int setSockAddrUn(char *path, struct sockaddr_un *addr)
{
  if (addr == NULL)
    return 0;
  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);
  return SUN_LEN(addr);
}

/*
 * Mounts the socket for the server
 * Input:
 *  - path: path for the file associated with the socket
 *  - addr: socket struct for the server
 * Returns: socket file descriptor
 */
int socketMount(char *socket_name)
{
  int sockfd;
  struct sockaddr_un server_addr;

  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
  {
    perror("server: can't open socket");
    exit(EXIT_FAILURE);
  }
  unlink(socket_name);
  if (bind(sockfd, (struct sockaddr *)&server_addr, setSockAddrUn(socket_name, &server_addr)) < 0)
  {
    perror("server: bind error");
    exit(EXIT_FAILURE);
  }
  return sockfd;
}

int main(int argc, char *argv[])
{
  int sockfd;
  validateInitArgs(argc, argv);
  init_fs();
  sockfd = socketMount(argv[2]);
  executeThreads(argv[1], sockfd);
  close(sockfd);
  unlink(argv[2]);
  exit(EXIT_SUCCESS);
  destroy_fs();
  exit(EXIT_SUCCESS);
}