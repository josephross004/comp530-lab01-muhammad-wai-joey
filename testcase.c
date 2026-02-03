#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"

// ----------------------------------
// You may add helper functions here
// -----------------------------------







// End helper function definitions


typedef int (*function_ptr)();

typedef struct {
    char *name;
    function_ptr fn;
} fn_table_entry_t;


// ----------------------------------
// Define test cases here.
// Three examples have been provided.
// ----------------------------------


int tc1() {
    int res = PASS;
    char* req = (char*)calloc( CHUNK, sizeof( char ) );
    strncpy( req, "GET /endpoint?user=brent&mode=debug&test=rizz HTTP/1.1\r\n", CHUNK-1 );
    res = create_request( req );
    if ( res == FAIL ) printf( "Invalid HTTP request\n" );
    unallocate_request();
    free( req );
    return res;
}

int tc2() {
    int res = PASS;
    char* req = (char*)calloc( CHUNK, sizeof( char ) );
    strncpy( req, "GET /endpoint?user=brent&mode=debug&test=rizz HTTP/1.1\r\n", CHUNK-1 );
    create_request( req );
    if ( strcmp( rs->path, "/endpoint")) {
        printf( "Invalid HTTP request path!\n" );
        res = FAIL;
    }
    unallocate_request();
    free( req );
    return res;
}

int tc3() {
    int res = PASS;
    char* req = (char*)calloc( CHUNK, sizeof( char ) );
    strncpy( req, "JOST /endpoint HTTP/1.1\r\nContent-Length: 26\r\n\r\nuser=brent&mode=debug&test=rizz", CHUNK-1 );
    res = create_request( req );
    if ( res == FAIL ) printf( "Invalid HTTP request\n" );
    unallocate_request();
    free( req );
    return res;
}


// End test case function definitions

// ----------------------------------
// Every new test case function needs 
// to be defined the function table.
// Update as needed.
// ----------------------------------

fn_table_entry_t fn_table[] = {
    {"tc1", tc1},
    {"tc2", tc2},
    {"tc3", tc3},
    {NULL, NULL} // mark the end
};

// End function table definitions

function_ptr lookup_function(const char *fn_name) {
    if (!fn_name) {
        return NULL;
    }
    for (int i = 0; fn_table[i].name != NULL; ++i) {
        if (!strcmp(fn_name, fn_table[i].name)) {
            return fn_table[i].fn;
        }
    }
    return NULL; // testcase not found
}

int main( int argc, char** argv ) {

    int rv = FAIL;
    
    if ( argc != 2 ) {
        printf("------------------------\n");
        printf("Test case program\n");
        printf("------------------------\n");
        printf("Usage: ./testcase <testcase name>\n\n");
        printf("Example: ./testcase tc1\n\n");
        return 0;
    }

    function_ptr func = lookup_function( argv[1] );
    if (func != NULL) {
        rv = func();
    } else {
        printf("Testcase (%s) not defined!\n", argv[1] );
    }

    return rv;
}
