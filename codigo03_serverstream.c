#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

/* the port users will be connecting to */
#define MYPORT 3490

/* how many pending connections queue will hold */
#define BACKLOG 10

// max number of bytes we can get at once
#define MAXDATASIZE 300

void sigchld_handler(int s) {
  while(wait(NULL) > 0);
}

int init_server() {
  /**
  * Function to start and the server
  * Arguments:
  *
  * Return:
  *       sockfd      - the socket descriptor    
  */

  int sockfd;
  int yes = 1;

  /* my address information */
  struct sockaddr_in my_addr;
  struct sigaction sa;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    perror("Server-socket() error lol!");
    exit(1);
  }
  else
    printf("Server-socket() sockfd is OK...\n");

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    perror("Server-setsockopt() error lol!");
    exit(1);
  }
  else
    printf("Server-setsockopt is OK...\n");

  /* host byte order */
  my_addr.sin_family = AF_INET;

  /* short, network byte order */
  my_addr.sin_port = htons(MYPORT);

  /* automatically fill with my IP */
  my_addr.sin_addr.s_addr = INADDR_ANY;
  printf("Server-Using %s and port %d...\n", inet_ntoa(my_addr.sin_addr), MYPORT);

  /* zero the rest of the struct */memset(&(my_addr.sin_zero), '\0', 8);
  if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1){
    perror("Server-bind() error");
    exit(1);
  }
  else
    printf("Server-bind() is OK...\n");

  if(listen(sockfd, BACKLOG) == -1){
    perror("Server-listen() error");
    exit(1);
  }

  printf("Server-listen() is OK...Listening...\n");
  /* clean all the dead processes */

  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if(sigaction(SIGCHLD, &sa, NULL) == -1){
    perror("Server-sigaction() error");
    exit(1);
  }
  else
    printf("Server-sigaction() is OK...\n");

  return sockfd;
}

char *str_to_upper(char *str) {
  /*
  * Function to convert a string to uppercase
  * Arguments:
  *       str   - The string that must convert to uppercase
  */

  char *s = str;
  if (s) {
    while (*s) {
      *s = toupper((unsigned char) *s);
      s++;
    }
  }
}

void handle_client_connection(int new_fd) {
  /*
  * Function to handle a new client connection. Processing the commands received from client
  * Arguments:
  *       new_fd   - The socket's file descriptor where is the connection with the client
  */
  int numbytes;
  char *token, *raw_token;
  char *buffentrada = (char *) calloc(MAXDATASIZE, sizeof(char));

  while(1){
    //if((numbytes = recv(new_fd, buffentrada, MAXDATASIZE-1, 0)) == -1){
    if(recv(new_fd, buffentrada, MAXDATASIZE-1, 0) == -1){
      perror("Error reading from client connection\n");
      exit(1);
    }

    token = strtok(buffentrada, " "); //divide la cadena
    // After reading the command received, it reads an empty buff
    // so just ommit this empty buff and wait for the other command sent by
    // client
    if (!token)
      continue;
    // Normalize to uppercase command
    // As is a refered argument, change is made on token itself
    str_to_upper(token);

    if(strcmp(token,"INSERT") == 0){
      //Obtiene el siguiente token, son dos palabras en total 
      token = strtok(NULL, " ");
      //Codigo insert
      printf("Comando insert\n");
      puts(token);
    }
    else if(strcmp(token,"SELECT") == 0){
      //Obtiene el siguiente token, son dos palabras en total 
      token = strtok(NULL, " ");
      printf("Comando select\n");
      puts(token);
      //Abrir archivo y enviarlo
    }
    else if(strcmp(token,"EXIT") == 0){
      printf("Comando exit\n");
      puts(token);
      break;
    }
    else{
      printf("comando no reconocido\n");
      //Comando no reconocido
    }
    // Flushing buffer
    memset(buffentrada, '\0', sizeof buffentrada);
  }
}

int main(int argc, char *argv[ ]){
  /* listen on sock_fd, new connection on new_fd */
  int sockfd, new_fd;

  /* connectors address information */
  struct sockaddr_in their_addr;
  int sin_size;

  // Declare buffer to store info received from client
  char buffsalida[MAXDATASIZE];

  sockfd = init_server();

  /* accept() loop */
  while(1){
    sin_size = sizeof(struct sockaddr_in);
    if((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1){
      perror("Server-accept() error");
      continue;
    }

    printf("Server-accept() is OK...\n");
    printf("Server-new socket, new_fd is OK...\n");
    printf("Server: Got connection from %s\n", inet_ntoa(their_addr.sin_addr));

    /* this is the child process */
    if(!fork()){
      /* child doesnt need the listener */
      close(sockfd);
      handle_client_connection(new_fd);
      printf("ending connection with client\n");
      close(new_fd);
      exit(0);
    }

    /* parent doesnt need this */
    close(new_fd);
    printf("Server-new socket, new_fd closed successfully...\n");
  }
  return 0;
}
