#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"

// ----------------------------------
// You may add helper functions here
// -----------------------------------


int validate_request(char* e_method, char* e_url, char* e_path, char* e_query, 
                     char** e_keys, char** e_vals, int num_kv) {
    
    // missing fields must be empty strings, not NULL
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

    // validate Linked  List (KV Pairs)
    kv_pair_t* curr = rs->head_node;
    for (int i = 0; i < num_kv; i++) {
        // list ends prematurely
        if (curr == NULL) {
            printf("Fail: KV list shorter than expected (idx %d)\n", i);
            return FAIL;
        }
        // key and value
        if (strcmp(curr->key, e_keys[i]) != 0 || strcmp(curr->value, e_vals[i]) != 0) {
            printf("Fail: KV mismatch at idx %d. Expected %s=%s, got %s=%s\n", 
                    i, e_keys[i], e_vals[i], curr->key, curr->value);
            return FAIL;
        }
        curr = curr->next_node;
    }

    // final Integrity Checks
    // Per policy:  query is missing, head_node = NULL
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


// Helper function to run a single test case
int run_test(char* req_str, int exp_status, char* e_meth, char* e_url, 
             char* e_path, char* e_qry, char** e_keys, char** e_vals, int num_kv) {
    int res = PASS;
    char* req = (char*)calloc(CHUNK, sizeof(char));
    strncpy(req, req_str, CHUNK - 1);

    // execute
    int actual_status = create_request(req);

    // validate Status (Handles Negative Testing)
    if (actual_status != exp_status) {
        printf("Fail: Expected create_request to return %d, but got %d\n", exp_status, actual_status);
        res = FAIL;
    } 
    // deep Validation (Only if the request was successful)
    else if (actual_status == PASS) {
        res = validate_request(e_meth, e_url, e_path, e_qry, e_keys, e_vals, num_kv);
    }

    // cleanup
    unallocate_request();
    free(req);
    return res;
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
    char* keys[] = {"user", "mode"};
    char* vals[] = {"brent", "debug"};
    return run_test("GET /endpoint?user=brent HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?user=brent", "/endpoint", 
                    "user=brent", keys, vals, 1);
}

int get2() {
    char* keys[] = {"user","mode","test"};
    char* vals[] = {"brent","debug","rizz"};

    return run_test("GET /endpoint?user=brent&mode=debug&test=rizz HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?user=brent&mode=debug&test=rizz", "/endpoint", 
                    "user=brent&mode=debug&test=rizz", keys, vals, 3);
}

int get3() {
    return run_test("GET /endpoint HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint", "/endpoint", "", NULL, NULL, 0);
}

int get4() {
    return run_test("GET /endpoint? HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?", "/endpoint", "", NULL, NULL, 0);
}

int get5() {
    return run_test("GET  HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "", "", "", NULL, NULL, 0);
}

/*II. Method Field
    1. Ensure method is uppercase in output ("get" -> GET, gEt -> GET, pOsT -> POST)
    2. Invald method names (in tc3 given "JOST") should FAIL. 
*/

// int method1() {
//     char* keys[] = {"user", "mode", "test"};
//     char* vals[] = {"brent", "debug", "rizz"};

//     // Note the mixed case "gEt" in the input, but "GET" in the expected output
//     return run_test("gEt /endpoint HTTP/1.1\r\nContent-Length: 40\r\n\r\nuser=brent&mode=debug&test=rizz\r\n\r\n", 
//                     PASS, "POST", "/endpoint?user=brent&mode=debug&test=rizz", "/endpoint", 
//                     "user=brent&mode=debug&test=rizz", keys, vals, 3);
// }

// int method2() {
//     char* keys[] = {"user", "mode", "test"};
//     char* vals[] = {"brent", "debug", "rizz"};

//     // same concept at methos1 but with "PoSt" instead of "gEt"
//     return run_test("PoSt /endpoint HTTP/1.1\r\nContent-Length: 40\r\n\r\nuser=brent&mode=debug&test=rizz\r\n\r\n", 
//                     PASS, "POST", "/endpoint?user=brent&mode=debug&test=rizz", "/endpoint", 
//                     "user=brent&mode=debug&test=rizz", keys, vals, 3);
// }



int method3() {
    // expect this to return FAIL (0) because "JOST" is invalid.
    // If it returns 0 then  PASS
    return run_test("JOST /endpoin HTTP/1.1\r\n\r\n", FAIL, "", "", "", "", NULL, NULL, 0);
}



/*III. Basic POST requests.
    1. POST with entity body containing one KV pair. 
    2. POST with multiple kv pairs.
    3. POST where Content-Length matches body length.
    4. POST where body has no parameters, which leads to empty quesry and NULL head_node. 
    5. POST iwth missing body despite content-length
    6. POSt with no Content-Length at all (is this a FAILing test?)*/

int post1(){
    char* keys[] = {"user"};
    char* vals[] = {"brent"};

    return run_test("POST /endpoint HTTP/1.1\r\nContent-Length: 10\r\n\r\nuser=brent\r\n\r\n", 
                    PASS, "POST", "/endpoint", "/endpoint", 
                    "user=brent", keys, vals, 1);
}

int post2(){
    char* keys[] = {"user", "mode", "test"};
    char* vals[] = {"brent", "debug", "rizz"};

    return run_test("POST /endpoint HTTP/1.1\r\nContent-Length: 31\r\n\r\nuser=brent&mode=debug&test=rizz\r\n\r\n", 
                    PASS, "POST", "/endpoint", "/endpoint", 
                    "user=brent&mode=debug&test=rizz", keys, vals, 3);
}

int post3(){
    char* keys[] = {"user", "mode", "test"};
    char* vals[] = {"brent", "debug", "rizz"};

    return run_test("POST /endpoint HTTP/1.1\r\nContent-Length: 31\r\n\r\nuser=brent&mode=debug&test=rizz\r\n\r\n", 
                    PASS, "POST", "/endpoint", "/endpoint", 
                    "user=brent&mode=debug&test=rizz", keys, vals, 3);
}

int post4(){
    return run_test("POST /endpoint HTTP/1.1\r\nContent-Length: 0\r\n\r\n\r\n", 
                    PASS, "POST", "/endpoint", "/endpoint", "", NULL, NULL, 0);
}

// int post5(){
//     return run_test("POST /endpoint HTTP/1.1\r\nContent-Length: 11\r\n\r\n\r\n", 
//                     FAIL, "POST", "/endpoint", "/endpoint", "", NULL, NULL, 0);
// }

int post6(){
    // this test is hard to do since the grading policy / design policy doesn't say that missing content-length should fail. 
    // we will assume that it should fail since we dont know how much of the body to read.
    return run_test("POST /endpoint HTTP/1.1\r\n\r\nuser=brent&mode=debug&test=rizz\r\n\r\n", 
                    PASS, "", "", "", "", NULL, NULL, 0);
}
/*IV. URL correctness
    Missing URL becomes empty string fields
    Handle paths with leading or trailing slashes
    Missing path results in empty path string field
    GET: Query appears after ? and before the next space
    POST: Query is only in entity body, not URL
    Missing query reuslts in empty query string and NULL head_node*/

int url1(){
    return run_test("GET  HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "", "", "", NULL, NULL, 0);
}

int url2(){
    return run_test("GET /endpoint/ HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint/", "/endpoint/", "", NULL, NULL, 0);
}

int url3(){
    return run_test("GET /endpoint/// HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint///", "/endpoint///", "", NULL, NULL, 0);
}

int url4(){
    char* keys[] = {"user"};
    char* vals[] = {"brent"};

    return run_test("GET /endpoint?user=brent HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?user=brent", "/endpoint", 
                    "user=brent", keys, vals, 1);
}
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

int kv1(){
    char* keys[] = {"user", "mode", "test"};
    char* vals[] = {"brent", "debug", "rizz"};

    return run_test("GET /endpoint?user=brent&mode=debug&test=rizz HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?user=brent&mode=debug&test=rizz", "/endpoint", 
                    "user=brent&mode=debug&test=rizz", keys, vals, 3);
}

int kv2(){
    return run_test("GET /endpoint?key= HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?key=", "/endpoint", 
                    "key=", NULL, NULL, 0);
}

int kv3(){
    return run_test("GET /endpoint?=value HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?=value", "/endpoint", 
                    "=value", NULL, NULL, 0);
}

int kv4(){
    char* keys[] = {"mode"};
    char* vals[] = {"debug"};

    return run_test("GET /endpoint?&mode=debug HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?&mode=debug", "/endpoint", 
                    "&mode=debug", keys, vals, 1);
}

int kv5(){
    char* keys[] = {"user", "mode"};
    char* vals[] = {"brent", "debug"};

    return run_test("GET /endpoint?user=brent&&mode=debug HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?user=brent&&mode=debug", "/endpoint", 
                    "user=brent&&mode=debug", keys, vals, 2);
}

int kv6(){
    char* keys[] = {"mode"};
    char* vals[] = {"debug"};

    return run_test("GET /endpoint?user&mode=debug HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?user&mode=debug", "/endpoint", 
                    "user&mode=debug", keys, vals, 1);
}

int kv7(){
    char* keys[] = {"user"};
    char* vals[] = {"brent"};

    return run_test("GET /endpoint?user=brent&incomplete= HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?user=brent&incomplete=", "/endpoint", 
                    "user=brent&incomplete=", keys, vals, 1);
}

int kv8(){
    char* keys[] = {"mode"};
    char* vals[] = {"debug"};

    return run_test("GET /endpoint?&mode=debug HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?&mode=debug", "/endpoint", 
                    "&mode=debug", keys, vals, 1);
}

int kv9(){
    char* keys[] = {"user"};
    char* vals[] = {"brent"};

    return run_test("GET /endpoint?user=brent& HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?user=brent&", "/endpoint", 
                    "user=brent&", keys, vals, 1);
}

int kv10(){
    return run_test("GET /endpoint? HTTP/1.1\r\n\r\n", 
                    PASS, "GET", "/endpoint?", "/endpoint", 
                    "", NULL, NULL, 0);
}
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
    // {"tc6", method1},
    // {"tc7", method2},
    {"tc6", method3},
    {"tc7", post1},
    {"tc8", post2},
    {"tc9", post3},
    {"tc10", post4},
    // {"tc13", post5},
    {"tc11", url1},
    {"tc12", url2},
    {"tc13", url3},
    {"tc14", url4},
    {"tc15", kv1},
    {"tc16", kv2},
    {"tc17", kv3},
    {"tc18", kv4},
    {"tc19", kv5},
    {"tc20", kv6},
    {"tc21", kv7},
    {"tc22", kv8},
    {"tc23", kv9},
    {"tc24", kv10},
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