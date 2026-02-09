#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"

// ----------------------------------
// You may add helper functions here
// -----------------------------------
int validate_request(char* e_method, char* e_url, char* e_path, char* e_query, 
                     char** e_keys, char** e_vals, int num_kv) {
    
    // 1. Validate core string fields
    // Per policy: missing fields must be empty strings, not NULL
    if (strcmp(rs->method, e_method) != 0) {
        printf("Fail: Method mismatch. Expected %s, got %s\n", e_method, rs->method);
        return FAIL;
    }
    if (strcmp(rs->url, e_url) != 0) {
        printf("Fail: URL mismatch. Expected %s, got %s\n", e_url, rs->url);
        return FAIL;
    }
    if (strcmp(rs->path, e_path) != 0) {
        printf("Fail: Path mismatch. Expected %s, got %s\n", e_path, rs->path);
        return FAIL;
    }
    if (strcmp(rs->query, e_query) != 0) {
        printf("Fail: Query mismatch. Expected %s, got %s\n", e_query, rs->query);
        return FAIL;
    }

    // 2. Validate Linked List (KV Pairs)
    kv_pair_t* curr = rs->head_node;
    for (int i = 0; i < num_kv; i++) {
        // If list ends prematurely
        if (curr == NULL) {
            printf("Fail: KV list shorter than expected (idx %d)\n", i);
            return FAIL;
        }
        // Check key and value
        if (strcmp(curr->key, e_keys[i]) != 0 || strcmp(curr->value, e_vals[i]) != 0) {
            printf("Fail: KV mismatch at idx %d. Expected %s=%s, got %s=%s\n", 
                    i, e_keys[i], e_vals[i], curr->key, curr->value);
            return FAIL;
        }
        curr = curr->next_node;
    }

    // 3. Final Integrity Checks
    // Per policy: If query is missing, head_node must be NULL
    if (num_kv == 0 && rs->head_node != NULL) {
        printf("Fail: head_node should be NULL for empty query\n");
        return FAIL;
    }
    // Check for trailing nodes
    if (curr != NULL) {
        printf("Fail: KV list longer than expected\n");
        return FAIL;
    }

    return PASS;
}
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

/*
I. Basic GET Requests.
    1. GET with valid path and a single key-value pair.
    2. GET with multiple key-value pairs.
    3. GET with no query (for example GET /path HTTP/1.1)
    4. GET with no query AND verify that set query="" and head_node=NULL
    5. GET with missing path (GET HTTP/1.1) - i.e., produce empty URL path and query fields. 
    
*/

int get1() {
    int res = PASS;
    char* req = (char*)calloc(CHUNK, sizeof(char));
    strncpy(req, "GET /endpoint?user=brent&mode=debug HTTP/1.1\r\n", CHUNK-1);

    if (create_request(req) == FAIL) {
        free(req);
        return FAIL;
    }

    // Define expected KV pairs
    char* expected_keys[] = {"user", "mode"};
    char* expected_vals[] = {"brent", "debug"};

    // Single line validation of the entire internal state
    res = validate_request("GET", "/endpoint?user=brent&mode=debug", "/endpoint", 
                           "user=brent&mode=debug", expected_keys, expected_vals, 2);

    unallocate_request();
    free(req);
    return res;
}

int get2() {
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

int get3() {
    int res = PASS;
    char* req = (char*)calloc( CHUNK, sizeof( char ) );
    strncpy( req, "GET /endpoint HTTP/1.1\r\n", CHUNK-1 );
    res = create_request( req );
    if ( res == FAIL ) printf( "Invalid HTTP request\n" );
    unallocate_request();
    free( req );
    return res;
}

int get4(){
    int res = PASS;
    char* req = (char*)calloc( CHUNK, sizeof( char ) );
    strncpy( req, "GET /endpoint? HTTP/1.1\r\n", CHUNK-1 );
    res = create_request( req );
    // check that query is empty string
    if ( strcmp(rs->query, "")) {
        printf( "invalid http request: query is not empty string\n" );
        res = FAIL;
    }
    // check that head-node is null
    if ( rs->head_node != NULL ) {
        printf( "invalid http request: head node not null\n" );
        res = FAIL;
    }
    if ( res == FAIL ) printf( "Invalid HTTP request\n" );
    unallocate_request();
    free( req );
    return res;
}

int get5() {
    int res = PASS;
    char* req = (char*)calloc( CHUNK, sizeof( char ) );
    strncpy( req, "GET HTTP/1.1\r\n", CHUNK-1 );
    res = create_request( req );
    // check that url is empty string
    if ( strcmp(rs->url, "")) {
        printf( "invalid http request: url is not empty string\n" );
        res = FAIL;
    }
    // check that query is empty string
    if ( strcmp(rs->query, "")) {
        printf( "invalid http request: query is not empty string\n" );
        res = FAIL;
    }
    if ( res == FAIL ) printf( "Invalid HTTP request\n" );
    unallocate_request();
    free( req );
    return res;
}

/*II. Method Field
    1. Ensure method is uppercase in output ("get" -> GET, gEt -> GET, pOsT -> POST)
    2. Invald method names (in tc3 given "JOST") should FAIL. 
*/

int method1() {
    int res = PASS;
    char* req = (char*)calloc( CHUNK, sizeof( char ) );
    strncpy( req, "JOST /endpoint HTTP/1.1\r\nContent-Length: 26\r\n\r\nuser=brent&mode=debug&test=rizz", CHUNK-1 );
    res = create_request( req );
    if ( res == FAIL ) printf( "Invalid HTTP request\n" );
    unallocate_request();
    free( req );
    return res;
}

int method2() {
    int res = PASS;
    char* req = (char*)calloc( CHUNK, sizeof( char ) );
    strncpy( req, "JOST /endpoint HTTP/1.1\r\nContent-Length: 26\r\n\r\nuser=brent&mode=debug&test=rizz", CHUNK-1 );
    res = create_request( req );
    if ( res == FAIL ) printf( "Invalid HTTP request\n" );
    unallocate_request();
    free( req );
    return res;
}

/*III. Basic POST requests.
    1. POST with entity body containing one KV pair. 
    2. POST with multiple kv pairs.
    3. POST where Content-Length matches body length.
    4. POST where body has no parameters, which leads to empty quesry and NULL head_node. 
    5. POST iwth missing body despite content-length
    6. POSt with no Content-Length at all (is this a FAILing test?)*/

/*IV. URL correctness
    Correct reconstruction of url for GET (/path?query)
    Correct URL for POST (includes query even though body contains it)
    Missing URL becomes empty string fields
    Extracting path correctly (/endpoint)
    Handle paths with leading or trailing slashes
    Missing path results in empty path string field
    GET: Query appears after ? and before the next space
    POST: Query is only in entity body, not URL
    Missing query reuslts in empty query string and NULL head_node*/

/*V. KV pairs. Valid kv pairs. Malformed pairs must not create nodes. 
    1. Create test for something like `user=brent&mode=debug` and check order. 
    2. key=  - no node.
    3. =value - no node. 
    4. &mode=debug - missing pair before '&' should skip
    5. user=brent && mode=debug - empty middle  is skipped
    6. key with no equals like `user` should be skipped
    7. user=brent&incomplete= should skip since the last pair is missing a value. 
    8. query starts with &
    9. query ends with &
    10. empty string entirely 
    11. Node order is preserved in the linked list
    12. Each node has correct KV values
    13. Next pointer is correct 
    14. Tail node's NEXT is NULL*/

/*VI. Memory management (could be duplicated under valgrind? )
    1. After create_request all hte fields heap-allocated are NON-NULL
        - LL nodes are allocated
        - No uninitialized pointers
    2. After unallocate_request all 
        - allocated string are null
        - nodes in LL are freed
        -head node is null
       - pointers in global struct are reset (are they supposed to be?)
    3. Repeated execution testing
        Run something like test1->unallocate->test2->unallocate->test3->unallocate
        Verify that there's no leftover state, no reused pointers, no segfaults, and no memory growth. */

/**/

// End test case function definitions

// ----------------------------------
// Every new test case function needs 
// to be defined the function table.
// Update as needed.
// ----------------------------------

fn_table_entry_t fn_table[] = {
    {"tc1", get1},
    {"tc2", get2},
    {"tc3", get3},
    {"tc4", get4},
    {"tc5", get5},
    {"tc6", method1},
    {"tc7", method2},
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
