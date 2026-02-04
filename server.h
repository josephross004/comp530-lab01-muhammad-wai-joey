// -----------------------------------
// 	COMP 530: Operating Systems
//
//	Spring 2026 - Lab 1
// -----------------------------------

// -------------------------------------------------
// Global variables
// Do add or modify.
#define OK 0
#define PASS 0
#define FAIL -1
#define NONE -1
#define CHUNK 256
#define MINCHUNK 8

// -------------------------------------------------
// Data structures (see server section in README)
// Do not modify.
typedef struct kv_pair {
	char key[ CHUNK ];
	char value[ CHUNK ];
	struct kv_pair* next_node;
} kv_pair_t;

typedef struct {
	char* url;
	char* path;
	char* query;
	char* method;
	kv_pair_t* head_node;
} request_struct;

extern request_struct* rs;

// ----------------------------------------
// Functions that CANNOT be modified
// ----------------------------------------

// ------------------------------------
// Function that initializes the signal 
// handler function
//
//
int initialize_handler();

// ------------------------------------
// Function that creates a server socket 
// and then binds it to the specified port. 
//
// Two process can communicate with each 
// other using a socket (i.e., interprocess 
// communication).
//
int bind_port(unsigned int port_number);

// ------------------------------------
// Function that reads the HTTP request 
// from the socket, and writes the HTTP 
// response back to the client.
//
//
void handle_client( int client_socket_fd );

// ------------------------------------
// Function that creates a HTTP response 
// that sends the JSON back to the client
//
//
void create_response( char* http_response, char* json_str );

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
void unallocate_request();

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
void sig_child_handler( int signal_type );

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
int run_server( unsigned int port_number );


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
int create_request( char* http_request );



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
int create_json( char* json_str );
