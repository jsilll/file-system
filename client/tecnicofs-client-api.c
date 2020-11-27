#include "tecnicofs-client-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int sockfd;
socklen_t servlen, clilen;
struct sockaddr_un serv_addr, client_addr;
char client_socket_name[MAX_INPUT_SIZE];
char send_buffer[MAX_INPUT_SIZE];
int send_size;
int receive_buffer; /* We always get a code from the server (defined in the API) */

/* Initializes a socket addr  */
int setSockAddrUn(char *path, struct sockaddr_un *addr)
{
  if (addr == NULL)
    return 0;
  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family = AF_UNIX;
  strcpy(addr->sun_path, path);
  return SUN_LEN(addr);
}

int sendCommand()
{
  if (sendto(sockfd, send_buffer, send_size + 1, 0, (struct sockaddr *)&serv_addr, servlen) < 0)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return SUCCESS;
}

int receiveResponse()
{
  int response;
  if (recvfrom(sockfd, &response, sizeof(receive_buffer), 0, 0, 0) < 0)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return response;
}

int tfsCreate(char *filename, char nodeType)
{
  send_size = sprintf(send_buffer, "c %s %c", filename, nodeType);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

int tfsDelete(char *path)
{
  send_size = sprintf(send_buffer, "d %s", path);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

int tfsMove(char *from, char *to)
{
  send_size = sprintf(send_buffer, "m %s %s", from, to);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

int tfsLookup(char *path)
{
  send_size = sprintf(send_buffer, "l %s", path);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

int tfsPrint(char *filename)
{
  send_size = sprintf(send_buffer, "p %s", filename);
  if (sendCommand() != SUCCESS)
    return TECNICOFS_ERROR_CONNECTION_ERROR;
  return receiveResponse();
}

int tfsMount(char *sockPath)
{
  char pid[256];
  
  /* Setting a random socket name */
  strcpy(client_socket_name, "/tmp/");
  sprintf(pid, "%d", getpid());
  strcat(client_socket_name, pid);
  
  /* Creating socket for the client */
  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
    return TECNICOFS_ERROR_MOUNT_ERROR;
  
  /* Binding the new socket to a file in /tmp */
  unlink(client_socket_name);
  clilen = setSockAddrUn(client_socket_name, &client_addr);
  if (bind(sockfd, (struct sockaddr *)&client_addr, clilen) < 0)
    return TECNICOFS_ERROR_MOUNT_ERROR;

  /* Getting the server address */
  servlen = setSockAddrUn(sockPath, &serv_addr);
  return SUCCESS;
}

int tfsUnmount()
{
  close(sockfd);
  unlink(client_socket_name);
  return SUCCESS;
}
