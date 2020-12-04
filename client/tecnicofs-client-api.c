#include "tecnicofs-client-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SUCCESS 0

int sockfd;
socklen_t servlen, clilen;
struct sockaddr_un serv_addr, client_addr;
char client_socket_name[MAX_INPUT_SIZE];
char send_buffer[MAX_INPUT_SIZE];
int send_size;
int receive_buffer; /* We always get a code from the server (defined in the API) */

/*
 * Initializes the socked address struct
 * Input:
 *  - path: buffer location in file system
 *  - addr: socket address struct
 * Returns: size of socket address struct
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
 * Sends through the socket the message added to the buffer
 * Returns: SUCCESS or TECNICOFS_ERROR_CONNECTION_ERROR
 */
int sendCommand()
{
  if (sendto(sockfd, send_buffer, send_size + 1, 0, (struct sockaddr *)&serv_addr, servlen) < 0)
    return TECNICOFS_ERROR_CONNECTION_ERROR;

  return SUCCESS;
}

/*
 * Reads response from the server to client request
 * Returns: An integer server response or TECNICOFS_ERROR_CONNECTION_ERROR
 */
int receiveResponse()
{
  if (recvfrom(sockfd, &receive_buffer, sizeof(receive_buffer), 0, 0, 0) < 0)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receive_buffer;
}

/*
 * Sends to server a create command request
 * Input:
 *  - filename: filename of node to be created
 *  - nodeType: type of node to be added, either directory or file
 * Return: An integer server response or TECNICO_ERROR_CONNECTION_ERROR
 */
int tfsCreate(char *filename, char nodeType)
{
  send_size = sprintf(send_buffer, "c %s %c", filename, nodeType);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

/*
 * Sends to server a delete command request
 * Input:
 *  - path: path to file to be deleted
 * Return: An integer server response or TECNICO_ERROR_CONNECTION_ERROR;
 */
int tfsDelete(char *path)
{
  send_size = sprintf(send_buffer, "d %s", path);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

/*
 * Sends to server a move command request
 * Input:
 *  - from: path to file to be moved
 *  - to: new path for the file
 * Return: An integer server response or TECNICO_ERROR_CONNECTION_ERROR;
 */
int tfsMove(char *from, char *to)
{
  send_size = sprintf(send_buffer, "m %s %s", from, to);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

/*
 * Sends to server a lookup command request
 * Input:
 *  - path: path to file to be moved
 * Return: An integer server response or TECNICO_ERROR_CONNECTION_ERROR;
 */
int tfsLookup(char *path)
{
  send_size = sprintf(send_buffer, "l %s", path);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

/*
 * Sends to server a print command request
 * Input:
 *  - filename: filename of where to print the tecnicofs tree
 * Return: An integer server response or TECNICO_ERROR_CONNECTION_ERROR;
 */
int tfsPrint(char *filename)
{
  send_size = sprintf(send_buffer, "p %s", filename);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

/*
 * Creates and binds the socket for the client and sets up server's socket address for communication
 * Input:
 *  - sockPath: path to file to be deleted
 */
int tfsMount(char *sockPath)
{
  /* Creating socket for the client */
  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    return TECNICOFS_ERROR_MOUNT_ERROR;

  bzero((char *)&client_addr, sizeof(struct sockaddr_un));
  client_addr.sun_family = AF_UNIX;

  /* Binding the new socket to a file in /tmp with random name */
  strcpy(client_addr.sun_path, "/tmp/client-socket-XXXXXX");

  if (mkstemp(client_addr.sun_path) == -1)
    return TECNICOFS_ERROR_MOUNT_ERROR;

  if (unlink(client_addr.sun_path) != 0)
    return TECNICOFS_ERROR_MOUNT_ERROR;

  clilen = SUN_LEN(&client_addr);

  if (bind(sockfd, (struct sockaddr *)&client_addr, clilen) < 0)
    return TECNICOFS_ERROR_MOUNT_ERROR;

  /* Getting the server address */
  servlen = setSockAddrUn(sockPath, &serv_addr);
  return SUCCESS;
}

/*
 * Closes the client's socket file descriptor and unlinks the socket for the client.
 */
int tfsUnmount()
{
  close(sockfd);
  unlink(client_socket_name);
  return SUCCESS;
}
