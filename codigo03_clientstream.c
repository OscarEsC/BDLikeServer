#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>

// the port client will be connecting to
#define PORT 3490
// max number of bytes we can read at once
#define MAXDATASIZE 300

void usage(char *argv0) {
  fprintf(stderr, "Invalid arguments");
  fprintf(stderr, "\n");
  printf("Usage: %s <server_hostname> <server_port>\n", argv0);
  printf("server_hostname (char *) is the IP of the server.\n");
  printf("server_port (int) where the server is listeting to new connections.\n");
}

int stablish_connection(char *hostname, int port) {
  int sockfd;
  struct hostent *he;
  // connectors address information
  struct sockaddr_in their_addr;

  // get the host info
  if((he=gethostbyname(hostname)) == NULL){
    perror("gethostbyname()");
    exit(1);
  }
  else
    printf("Client-The remote host is: %s\n", hostname);

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("socket()");
    exit(1);
  }
  else 
    printf("Client-The socket() sockfd is OK...\n");

  // host byte order
  their_addr.sin_family = AF_INET;
  // short, network byte order
  printf("Server-Using %s and port %d...\n", hostname, port);
  their_addr.sin_port = htons(port);
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);

  // zero the rest of the struct
  memset(&(their_addr.sin_zero), '\0', 8);
  if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1){
    perror("connect()");
    exit(1);
  }
  else
    printf("Client-The connect() is OK...\n");

  return sockfd;
}

void validate_arguments(int argc, char *argv[]) {
  // Validate there is at least one argument to the exec
  if(argc < 2 || argc > 3){
    fprintf(stderr, "Not a valid number of arguments.\n");
    usage(argv[0]);
    // just exit
    exit(1);
  }

  // validate the second argument (server port) is numeric string
  // strspn count initial segment matching the second string chars on the firts one.
  // if it is all the string, it means all chars are digit
  if (argc == 3 && !(strspn(argv[2], "0123456789") == strlen(argv[2]))) {
    fprintf(stderr, "Not a valid port number.\n");
    usage(argv[0]);
    // just exit
    exit(1);
  }

  // defining default port to connection
  if (argc == 2) {
    // allocate dynamic mem on argv[2]
    argv[2] = (char *) calloc(1, sizeof(int));
    // normalizing char at argv
    // parsing PORT to char * and store it on argv[2]
    sprintf(argv[2], "%d", PORT);
  }

}

int main(int argc, char *argv[]){
  int sockfd, numbytes;
  char bufentrada[MAXDATASIZE],bufsalida[MAXDATASIZE];

  validate_arguments(argc, argv);

  sockfd = stablish_connection(argv[1], atoi(argv[2]));
  while(1){
	  scanf("%s",bufsalida);
	  send(sockfd, bufsalida, sizeof(bufsalida), 0);
	  //Recibir resultado y mostrarlo
  }
  printf("Client-Closing sockfd\n");
  close(sockfd);
  return 0;
}
