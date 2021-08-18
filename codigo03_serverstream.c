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

/////////////// CONSTANTS //////////////////////////
char *INVALID_ACC_NUMBER = "The account number argument is not valid.";
char *INSUFFICIENT_ARGUMENTS = "There is insufficient arguments to this command.";
char *ERROR_ON_FILE = "Something went wrong when trying writing/reading the file.";
char *INSERT_OK = "INSERT'S EXECUTION WAS SUCCESSFULLY.";
char *FILE_DOES_NOT_EXISTS = "The file you are looking for does not exists on DB.";
char *UPDATE_OK = "UPDATE'S EXECUTION WAS SUCCESSFULLY.";
char *FILE_ALREADY_EXISTS = "The file you are trying to insert already exists on DB.";
char *DELETE_OK = "DELETE'S EXECUTION WAS SUCCESSFULLY.";

// Static dir where the files will be stored
char *dir = "out/";

////////////////////////////////////////////////////

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

int validate_account_number(char *account_number) {
  /*
  * Function to validate if the second argument of a command is a valid account_number
  * validating that it is just a number, does not matter the length
  * Arguments:
  *       account_number   - The number to validate
  * Return:
  *       1 if is a valid number, 0 otherwise
  */
  return strspn(account_number, "0123456789") == strlen(account_number) ? 1 : 0;
}

int write_file(char *filename, char *content) {
  /*
  * Function to write a string in a file
  * Arguments:
  *       filename   - The filename
  *       content     - What the file must contain
  * Return:
  *       1 if everything was ok, -1 if an error happends
  */
  char *filename_with_dir = (char *) calloc(MAXDATASIZE, sizeof(char));
  // Concat dir with filename
  strcat(strcpy(filename_with_dir, dir), filename);
  FILE *out_file = fopen(filename_with_dir, "w");

  if (out_file == NULL) {
    puts(ERROR_ON_FILE);
    return -1;
  }

  fprintf(out_file, "%s", content);
  fclose(out_file);
  return 1;
}

int delete_file(char *filename) {
  /*
  * Function to delete a file
  * Arguments:
  *       filename   - The filename
  * Return:
  *       0 if everything was ok, -1 if an error happends
  */
  char *filename_with_dir = (char *) calloc(MAXDATASIZE, sizeof(char));
  // Concat dir with filename
  strcat(strcpy(filename_with_dir, dir), filename);
  if (remove(filename_with_dir) == 0)
    return 0;
  else
    return -1;
}

int validate_file_exists(char *filename) {
  /*
  * Function to validate if a file exists
  * Arguments:
  *         filename        - Filename to check if exists
  * Return:
  *         0 if file exists, -1 if not
  */
  char *filename_with_dir = (char *) calloc(MAXDATASIZE, sizeof(char));
  // Concat dir with filename
  strcat(strcpy(filename_with_dir, dir), filename);
  return access( filename_with_dir, F_OK ) == 0 ? 0 : -1;
}

char *handle_insert(char * buffentrada, char *token) {
  /*
  * Function to handle insert command
  * validating there is all the arguments needed
  * Arguments:
  *       buffentrada   - The original string received by the server
  *       token         - the token pointer to get the next argument
  * Return:
  *       message       - A message that could be an error or a successful message
  */
  char *name;
  int starts_at = 0;

  // take the second argument, the account number
  token = strtok(NULL, " ");
  // When just 'insert' was sent to the server
  if (!token) {
    puts(INSUFFICIENT_ARGUMENTS);
    return INSUFFICIENT_ARGUMENTS;
  }
  // When sent an invalid accoind number
  if(!validate_account_number(token)) {
    puts(INVALID_ACC_NUMBER);
    return INVALID_ACC_NUMBER;
  }

  // Could not insert a file that already exists
  if(validate_file_exists(token) == 0) {
    puts(FILE_ALREADY_EXISTS);
    return FILE_ALREADY_EXISTS;
  }

  // Count the position where the name argument starts.
  // strlen("insert") + account number length + one space
  starts_at = 7 + strlen(token) + 1;
  if ( starts_at >= strlen(buffentrada) ) {
    puts(INSUFFICIENT_ARGUMENTS);
    return INSUFFICIENT_ARGUMENTS;
  }
  
  // Alloc memory to store name argument
  name = (char *) calloc(strlen(buffentrada) - starts_at, sizeof(char));
  // Copy the name argument to the name buffer
  memcpy( name, buffentrada + starts_at, (strlen(buffentrada) - starts_at) * sizeof(char) );
  
  if ( write_file(token, name) < 0 ) {
    return ERROR_ON_FILE;
  }

  return INSERT_OK;
}

char *handle_update(char *buffentrada, char *token) {
  /*
  * Function to handle update command
  * validating there is all the arguments needed
  * Arguments:
  *       buffentrada   - The original string received by the server
  *       token         - the token pointer to get the next argument
  * Return:
  *       message       - A message that could be an error or a successful message
  */
  char *name;
  int starts_at = 0;

  // take the second argument, the account number
  token = strtok(NULL, " ");
  // When just 'insert' was sent to the server
  if (!token) {
    puts(INSUFFICIENT_ARGUMENTS);
    return INSUFFICIENT_ARGUMENTS;
  }
  // When sent an invalid accoind number
  if(!validate_account_number(token)) {
    puts(INVALID_ACC_NUMBER);
    return INVALID_ACC_NUMBER;
  }
  // Could not update a file that not exists
  if(validate_file_exists(token) != 0) {
    puts(FILE_DOES_NOT_EXISTS);
    return FILE_DOES_NOT_EXISTS;
  }
  // Count the position where the name argument starts.
  // strlen("update") + account number length + one space
  starts_at = 6 + strlen(token) + 1;
  if ( starts_at >= strlen(buffentrada) ) {
    puts(INSUFFICIENT_ARGUMENTS);
    return INSUFFICIENT_ARGUMENTS;
  }
  
  // Alloc memory to store name argument
  name = (char *) calloc(strlen(buffentrada) - starts_at, sizeof(char));
  // Copy the name argument to the name buffer
  memcpy( name, buffentrada + starts_at, (strlen(buffentrada) - starts_at) * sizeof(char) );
  
  if ( write_file(token, name) < 0 ) {
    return ERROR_ON_FILE;
  }

  return UPDATE_OK;
}

char *handle_delete(char *token) {
  /*
  * Function to handle delete command
  * Arguments:
  *       token         - the token pointer to get the next argument
  * Return:
  *       message       - A message that could be an error or a successful message
  */

  // take the second argument, the account number
  token = strtok(NULL, " ");
  // When just 'insert' was sent to the server
  if (!token) {
    puts(INSUFFICIENT_ARGUMENTS);
    return INSUFFICIENT_ARGUMENTS;
  }
  // When sent an invalid accoind number
  if(!validate_account_number(token)) {
    puts(INVALID_ACC_NUMBER);
    return INVALID_ACC_NUMBER;
  }
  // Could not delete a file that not exists
  if(validate_file_exists(token) != 0) {
    puts(FILE_DOES_NOT_EXISTS);
    return FILE_DOES_NOT_EXISTS;
  }

  if ( delete_file(token) != 0) {
    puts(ERROR_ON_FILE);
    return ERROR_ON_FILE;
  }

  return DELETE_OK;
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
  char *buffsalida = (char *) calloc(MAXDATASIZE, sizeof(char));
  char *token_buff = (char *) calloc(MAXDATASIZE, sizeof(char));

  while(1){
    //if((numbytes = recv(new_fd, buffentrada, MAXDATASIZE-1, 0)) == -1){
    if(recv(new_fd, buffentrada, MAXDATASIZE-1, 0) == -1){
      perror("Error reading from client connection\n");
      exit(-1);
    }

    // Coppy buffentrada to another buffer in order to keep
    // the original received string
    snprintf(token_buff, MAXDATASIZE, "%s", buffentrada);

    token = strtok(token_buff, " ");
    // After reading the command received, it reads an empty buff
    // so just ommit this empty buff and wait for the other command sent by
    // client
    if (!token)
      continue;
    // Normalize to uppercase command
    // As is a refered argument, change is made on token itself
    str_to_upper(token);

    if(strcmp(token,"INSERT") == 0){
      puts("Handling insert command");
      snprintf(buffsalida, MAXDATASIZE - 1, "%s", handle_insert(buffentrada, token));
      puts(buffsalida);

      // Send the message response to client
      if (send(new_fd, buffsalida, strlen(buffsalida), 0) == -1) {
        printf("Error sending response to client.\n");  
      }
    }
    else if(strcmp(token,"UPDATE") == 0){
      puts("Handling update command");
      snprintf(buffsalida, MAXDATASIZE - 1, "%s", handle_update(buffentrada, token));
      puts(buffsalida);

      // Send the message response to client
      if (send(new_fd, buffsalida, strlen(buffsalida), 0) == -1) {
        printf("Error sending response to client.\n");  
      }
    }
    else if(strcmp(token,"SELECT") == 0){
      //Obtiene el siguiente token, son dos palabras en total 
      token = strtok(NULL, " ");
      printf("Comando select\n");
      puts(token);
      //Abrir archivo y enviarlo
    }
    else if(strcmp(token,"DELETE") == 0){
      puts("Handling delete command");
      snprintf(buffsalida, MAXDATASIZE - 1, "%s", handle_delete(token));
      puts(buffsalida);

      // Send the message response to client
      if (send(new_fd, buffsalida, strlen(buffsalida), 0) == -1) {
        printf("Error sending response to client.\n");  
      }
    }
    else if(strcmp(token,"EXIT") == 0){
      printf("Comando exit\n");
      break;
    }
    else{
      if (token) {
        printf("--%s--\n", token);
        strcpy(buffsalida, "Comando no reconocido");
        puts(buffsalida);
        // Send the message response to client
        if (send(new_fd, buffsalida, strlen(buffsalida), 0) == -1) {
          printf("Error sending response to client.\n");  
        }
      }
    }
    // Flushing buffer
    memset(buffentrada, '\0', strlen(buffentrada));
    memset(buffsalida, '\0', strlen(buffsalida));
  }
}

int main(int argc, char *argv[ ]){
  /* listen on sock_fd, new connection on new_fd */
  int sockfd, new_fd;

  /* connectors address information */
  struct sockaddr_in their_addr;
  int sin_size;

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
