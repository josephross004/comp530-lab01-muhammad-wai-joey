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

  // TODO






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

    printf("server_socket_fd = %d\n", server_socket_fd );

    while ( listen( server_socket_fd, 0 ) == 0 ) {

      printf("Listening and accepting connections on port %d\n", port_number );

      client_socket_fd = accept(server_socket_fd, (struct sockaddr *)&client_address, &client_length);

      // TODO - create a child to handle the client request.


      handle_client( client_socket_fd ); // This is just place holder


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
  
  // TODO
  
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

  

  return 0; // just a place holder


} // end create_json() function
