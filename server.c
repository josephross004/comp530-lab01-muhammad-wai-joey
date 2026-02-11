// I pledge the COMP530 honor code.

// -----------------------------------
//  COMP 530: Operating Systems
//
//  Spring 2026 - Lab 1
// -----------------------------------

#include "server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h> 
#include <wait.h>

// -------------------------------------------------
// Debugging flag (0=no, 1=yes)
// You man change this value for debugging purposes
#define DEBUG 0


// -------------------------------------------------
// Global variables
// Do add or modify global variables
request_struct* rs;
int server_socket_fd = 0;          // server socket file descriptor


// ----------------------------------------
// Functions that CANNOT be modified
// ----------------------------------------

// ------------------------------------
// Function that initializes the signal 
// handler function
//
//
int initialize_handler() {

  if ( signal( SIGCHLD, sig_child_handler ) == SIG_ERR ) {
      perror( "Unable to create SIGCHLD handler!");
      return FAIL;
  }

  return OK;

} // end initialize_handler() function

// ------------------------------------
// Function that creates a server socket 
// and then binds it to the specified port. 
//
// Two process can communicate with each 
// other using a socket (i.e., interprocess 
// communication).
//
int bind_port(unsigned int port_number) {

  int set_option = 1;
  struct sockaddr_in server_address;

  server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&set_option, sizeof(set_option));

  if (server_socket_fd < 0) return FAIL;

  bzero( (char *)&server_address, sizeof( server_address ) );

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port_number);

  if (bind(server_socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == 0) {
    return OK;
  } else return FAIL;

} // end bind_port() function

// ------------------------------------
// Function that reads the HTTP request 
// from the socket, and writes the HTTP 
// response back to the client.
//
//
void handle_client( int client_socket_fd ) {

  char* request = (char*)malloc( CHUNK*4 );
  char* json_str = (char*)malloc( CHUNK*4 );
  char* response = (char*)malloc( CHUNK*4 );
  char* error = (char*)malloc( CHUNK );

  close( server_socket_fd );
  bzero( request, CHUNK*4 );
  bzero( response, CHUNK*4 );

  read( client_socket_fd, request, (CHUNK*4) - 1 );

  int valid = create_request( request );

  if ( ( valid != FAIL ) ) { 
    create_json( json_str );
    create_response( response, json_str );
  } else {
    strcpy( error, "Shame on you, bad http request");
    sprintf( response, "HTTP/1.1 401 Bad Request\r\nContent-Length: %d\r\n\r\n%s", (int)strlen( error ), error );
  }
  
  write( client_socket_fd, response, strlen(response) );
  close( client_socket_fd );

  unallocate_request();

  free( request );
  free( response );
  free( json_str );
  free( error );

  exit( OK );

} // end handle_client() function

// ------------------------------------
// Function that creates a HTTP response 
// that sends the JSON back to the client
//
//
void create_response( char* http_response, char* json_str ) {

  sprintf( http_response, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n%s", 
              (int)strlen(json_str), json_str );

  if ( DEBUG ) printf( "%s\n", http_response );

} // end create_response() function


// ----------------------------------------
// Functions that CAN be modified
// ----------------------------------------

// ------------------------------------
// Function that frees all the memory
// allocated for the request_struct and kv_pair_t
// data structures (see server section in README)
//
// Arguments:   None
// Return:      None
//
void unallocate_request() {

  // TODO

  // free strings
  free(rs->url);
  free(rs->path);
  free(rs->query);
  free(rs->method);

  // free nodes
  kv_pair_t* dummy = NULL;
  while (rs->head_node != NULL){
    dummy = rs->head_node;
    rs->head_node = dummy->next_node;
    free(dummy);
  }

  // free main pointer
  free(rs);
} // end unallocate_request() function

// ------------------------------------
// Function that asynchonously reaps children 
// (i.e. recieves a SIGCHLD signal). 
// 
// Hint: look at the waitpid function.
//
//
// Arguments:  integer that represents
//             the signal type (for this 
//             lab it will be SIGCHLD).
//
// Return:      None
//
void sig_child_handler( int signal_type ) {
  

  if (signal_type == SIGCHLD) {
    // Reap all available children
    while (waitpid(-1, NULL, WNOHANG) > 0) { /* keep on reaping! */ }
    return;
  }

  // Graceful shutdown: close the listening socket so accept() unblocks and don't lose allocated bytes
  if ((signal_type == SIGTERM || signal_type == SIGINT) && server_socket_fd > 0) {
    close(server_socket_fd);
    server_socket_fd = -1;
  }


} // end sig_child_handler() function

// ------------------------------------
// Function runs a loop that continuely 
// listens for client connections. When 
// a new client connects the server will
// create a new child to handle the request.
// (see server section in README).
// 
// Note: if the fork is unsuccessful, then
// return FAIL immediately.
//
//
// Arguments:  unsigned integer that represents 
//             the port number.
//
// Return:      0 (OK, defined in server.h): No fork error.
//             -1 (FAIL, defined in server.h): Fork error.
//
int run_server( unsigned int port_number ) {

  int client_socket_fd = NONE;

  struct sockaddr_in client_address;
  socklen_t client_length = sizeof( client_address );

  if ( bind_port( port_number ) == FAIL ) {

    printf("Failed to bind socket to port %d\n", port_number );
    printf("Stopping http server!\n" );

    return FAIL;

  } else {
    signal(SIGTERM, sig_child_handler);
    signal(SIGINT, sig_child_handler);

    printf("server_socket_fd = %d\n", server_socket_fd );

    while ( listen( server_socket_fd, 0 ) == 0 ) {

      printf("Listening and accepting connections on port %d\n", port_number );

      client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_address, &client_length);

      // TODO - create a child to handle the client request.

      pid_t pid = fork();

      if (pid == -1) {
          return FAIL;
      } else if (pid == 0) {
          // Child process
          printf("Child process: PID: %d, parent's PID: %d\n", getpid(), getppid());
          handle_client( client_socket_fd );
          return OK;
      } else {
          printf("Parent process: PID: %d, child PID: %d\n", getpid(), pid);
          close(client_socket_fd);
      }
    }
  }

  return OK;

} // end run_server() function


// ------------------------------------
// Function that creates the HTTP request.
// This function will parse the request and 
// allocate memory for the request data 
// structure (see server section in README).
//
//
// Arguments: string that holds the http request 
//            sent by the client.
//
// Return:  0 (OK, defined in server.h): Valid method (e.g., GET or POST)
//         -1 (FAIL, defined in server.h): Invalid method (e.g., not GET or POST). 
//
int create_request( char* http_request ) {
  if (http_request == NULL){
    return FAIL;
  }

  int counter = 0;
  rs = (request_struct *) malloc(sizeof(*rs));

  // init rs to be null pointers and null objects
  rs->method = rs->url = rs->path = rs->query = NULL;
  rs->head_node = NULL;

  char* dummy = http_request;

  // method
  while (*dummy != '\0' && *dummy != ' '){
    counter++; dummy++;
  }

  // At this point in the code, counter now has the length of the request up to the first space 
  // (or to the end of the string.) This represents the METHOD. 

  // Make it so that *dummy assigns the end of http_request to 0
  *dummy = 0;

  // Then copy everything up until the NULL terminator into the method field. 
  rs->method = (char*)malloc((counter + 1) * sizeof(char));
  strcpy(rs->method, http_request);

  char get[] = "GET";
  char post[] = "POST";

  // Fail if neither GET nor POST is invoked.
  if (strcmp(rs->method, get) != 0 && strcmp(rs->method, post) != 0){
    return FAIL;
  }

  // Go to the next character after the "/0"
  http_request += counter;
  http_request++;
  counter = 0;
  dummy = http_request;

  // url and path
  bool query = false;

  while (*dummy!= '\0' && *dummy != ' '){
    // In the event that the dummy picks up on '?' then there is a query. 
    if (*dummy == '?') {
      query=true;  
    }
    dummy++; 
  }

  // Now calculate the total length and capture rs->url
  int url_len = dummy - http_request;
  rs->url = (char*)malloc(url_len + 1);
  strncpy(rs->url, http_request, url_len);
  rs->url[url_len] = '\0';

  // Split rs->url into PATH and QUERY
  if (query) {
    // search for '?' built into string.h
    char* qmark = strchr(rs->url, '?');
    int path_len = qmark - rs->url;

    rs->path = (char*)malloc(path_len + 1);
    strncpy(rs->path, rs->url, path_len);
    rs->path[path_len] = '\0';

    rs->query = strdup(qmark + 1);
  } else {
    // null query case - no ? found
    rs->path = strdup(rs->url);
    rs->query = strdup("");
  }
  
  // and update the pointer again for the next part: 
  http_request = dummy;

  // CHECK: did we get the URL and path? 
  printf("rs->url : %s\n", rs->url);
  printf("rs->path : %s\n", rs->path);

  // skipping version
  while (*http_request != '\r'){
    http_request++;
  }
  http_request += 2; //crlf
  dummy = http_request;

  // headers and body
  counter = 0;

  // This part only applies to POST requests. 
  if (strcmp(rs->method, "POST") == 0) {
    int contentLength = 0;
    
    // 1. Scan headers for Content-Length
    while (*dummy != '\r') {
        if (strncasecmp(dummy, "Content-Length:", 15) == 0) {
            char* val_start = strchr(dummy, ' ') + 1;
            contentLength = atoi(val_start);
        }
        // Move to next line
        while (*dummy != '\n') dummy++;
        dummy++;
    }
    
    // 2. Skip the blank line (\r\n) to get to the body
    dummy += 2; 
    
    // 3. NOW allocate and copy
    rs->query = (char*)malloc(contentLength + 1);
    memcpy(rs->query, dummy, contentLength);
    rs->query[contentLength] = '\0';
  }


  // need to not run the query-key-value loop if query is null
  // Only parse if query exists and is not an empty string
  if (rs->query != NULL && rs->query[0] != '\0'){
    char* left = rs->query;
    rs->head_node = NULL;
    kv_pair_t* prev = NULL;
    kv_pair_t* curr = NULL;

    while (*left != '\0'){
      // Find delimiters
      char* eq = strchr(left, '=');
      char* amp = strchr(left, '&');

      // Validation: Skip malformed entries like "key=" or "=value"
      if (eq == NULL || eq == left || *(eq + 1) == '\0' || *(eq + 1) == '&') {
          if (amp) { left = amp + 1; continue; }
          else break;
      }

      curr = (kv_pair_t *) malloc(sizeof(kv_pair_t));
      curr->next_node = NULL;

      // 1. Copy Key: snip at '=', copy, then restore '='
      *eq = '\0';
      strcpy(curr->key, left); 
      *eq = '=';

      // 2. Copy Value: snip at '&' if it exists
      left = eq + 1; 
      if (amp) {
        *amp = '\0';
        strcpy(curr->value, left);
        *amp = '&'; // Restore '&' for the rest of the string
        left = amp + 1; // Move 'left' to the start of the next pair
      } else {
        // This is the last value in the string
        strcpy(curr->value, left);
        left += strlen(left); // Terminates the while loop
      }

      // 3. Link the node
      if(rs->head_node == NULL) {
        rs->head_node = curr;
      } else {
        prev->next_node = curr;
      }
      prev = curr;
    }
  }
  // handle queries and linked list
  
  //Special edge case: when '?' at the end of rs->url and rs->query is strlen
  
  return OK; // just a place holder
} // end create_request() function



// ------------------------------------
// Function that converts the data in 
// the request_struct and kv_pair_t 
// structures to JSON (see server 
// section in README).
//
// Arguments:   pointer memory which will hold 
//              the json string.
//
// Return:      -1 (FAIL, defined in server.h) Provided to you (see below)
//              The length of json string (in bytes)
//
int create_json( char* json_str ) {


  if ( ( json_str == NULL ) || ( rs == NULL ) ) {
    return FAIL;
  }

  int counter = 0;

  char* method = "{\"method\":\"";
  for(int i = 0; i < strlen(method); ++i) json_str[counter++] = method[i];
  for(int i = 0; i < strlen(rs->method); ++i) json_str[counter++] = rs->method[i];
  
  char* url = "\",\"url\":\"";
  for(int i = 0; i < strlen(url); ++i) json_str[counter++] = url[i];
  for(int i = 0; i < strlen(rs->url); ++i) json_str[counter++] = rs->url[i];

  char* path = "\",\"path\":\"";
  char* pathEnd = "\"";
  for(int i = 0; i < strlen(path); ++i) json_str[counter++] = path[i];
  for(int i = 0; i < strlen(rs->path); ++i) json_str[counter++] = rs->path[i];
  for(int i = 0; i < strlen(pathEnd); ++i) json_str[counter++] = pathEnd[i];

  kv_pair_t* curr = rs->head_node;

  while (curr != NULL){
    json_str[counter++] = ',';
    json_str[counter++] = '\"';
    for(int i = 0; i < strlen(curr->key); ++i) json_str[counter++] = curr->key[i];
    char* inBetween = "\":\"";
    for(int i = 0; i < strlen(inBetween); ++i) json_str[counter++] = inBetween[i];
    for(int i = 0; i < strlen(curr->value); ++i) json_str[counter++] = curr->value[i];
    json_str[counter++] = '\"';
    curr = curr->next_node;
  }
  
  json_str[counter++] = '}';
  json_str[counter++] = 0;
  return strlen(json_str); // just a place holder


} // end create_json() function