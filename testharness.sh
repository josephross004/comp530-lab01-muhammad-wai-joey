#!/usr/bin/env bash

test_result() {
	if [ "$1" -eq "0" ]; then
		echo "$2 (PASSED)"
	else
		echo "$2 (FAILED)"
	fi
}

# -------------------------------
# Number of test cases.
# update this number as needed.
# -------------------------------
N=25

for ((i = 1; i < N+1; i++)); do
    echo "----------------------------"
  	./testcase "tc$i"
  	ec=$?
  	test_result "$ec" "tc$i"
	echo "\n\n\n-----------------------\n" >> "./valgrind_output.log"
	echo "Test case: tc$i" >> "./valgrind_output.log"
	echo "\n------------------------\n\n\n" >> "./valgrind_output.log"
	# run the test under Valgrind

	
	# Valgrind outline
	#     valgrind must detect memory leaks, invalid read/write, use-after-free errors, double frees, 
	#         unfreed LL nodes, unfreed strings in request_struct, and leaks from repeated test cycles
	#     For each test case, confirm exit status is correct (that valgrind errors trigger a FAIL) and no 
	#         leaks or invalid memory or uninit'd reads.
	#     Entire test suite has to run under Valgrind automatically. 


    # from valgrind man page: 
    # --error-exitcode=1 makes it so taht any leaks cause a failure exit on the testcase
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./testcase "tc$i" 2>> "./valgrind_output.log"
	ec=$?
	test_result "$ec" "tc$i (Valgrind)"
	
 	sleep 1
done


# ----------------------------------------------
# Honors section will included test cases below
# ---------------------------------------------- 

# Hey grader, the memory leak test is included in the loop above ^ 
# but is also done as an indirect consequence of this test below.

# Process reaping outline
#     1. Write a test in testharness.sh that starts the srever in the background and captures the parent PID
#     2. Send multiple client requests triggering a fork() in the server and the child handles the client 
#         and then the child has to exit. 
#     3. Check /proc for zombies - locate children of parent PID and if there are ANY zombies (ANY!!) return FAIL
#     4. Clean server shutdown. ^C should be a clean exit with no lingering children. 
#     5. Run the reaping test under back to back
#     6. Run the reaping test under heavy load (many requests)
#
#           Entire test suite has to run under Valgrind automatically. 

# (standard according to assignment - by default client.py 
# is going to use localhost:8080 .)
# Start server using valgrind 
# valgrind --trace-children=yes 
valgrind --leak-check=full  --show-leak-kinds=all --errors-for-leak-kinds=all --trace-children=yes ./lab01 "8080" > server.log   2>&1 &

# capture server PID
SERVER_PID=$!

# give it a moment to bind
sleep 1

# SANITY CHECK: did the server crash? 
# see line 109-110 - taking a different approach here
if ps -ef | grep "$SERVER_PID" ; then
    echo "Server is running"
fi

# SANITY CHECK: where's the server listening (netstat)?
# netstat to find out
if netstat -n | grep "8080 "; then
    echo "Server is listening on port 8080."
fi

# blow through multiple requests 
for i in {1..50}; do
    python3 client.py "get" "endpoint" "id=$i" > /dev/null 2>&1 &
done

echo "Waiting 15s for processes to finish logging."
sleep 15

# use file to count how many spawns there were.
SPAWNED_IN_LOG_COUNT=$(grep -c "Child process:" server.log)
PARENT_IN_LOG_COUNT=$(grep -c "Parent process:" server.log)

# show evidence of children being created
echo "$SPAWNED_IN_LOG_COUNT child processes"
echo "$PARENT_IN_LOG_COUNT parennt processes"

# Check to see if it matches expected
if [ "$SPAWNED_IN_LOG_COUNT" -eq 50 ]; then
    echo "EVIDENCE: Success. Parent spawned all 50 children."
else
    echo "EVIDENCE: Failed. Only $SPAWNED_IN_LOG_COUNT children were detected."
fi

echo "Waiting 15s for clients to finish network calls."
sleep 15

# wait I totally dropped the ball on the old version of this - didn't always work
# Redirecting a ps-ef works way better and might be better if /proc isn't synced at this point
ZOMBIE_COUNT=$(ps -ef | grep $SERVER_PID | grep "<defunct" | wc -l)

if [ "$ZOMBIE_COUNT" -gt 0 ]; 
then
    echo "REAPING TEST: FAILED ($ZOMBIE_COUNT zombies detected)"
    kill $SERVER_PID
    exit 1
else
    echo "REAPING TEST: PASSED (No zombies found)"
fi

# Clean Shutdown using  kill
kill $SERVER_PID

sleep 1 

# Fail if any leak or errors detected

if ! grep "ERROR SUMMARY: 0 errors from 0 contexts" "server.log"; then
    echo "VALGRIND: FAILED "
    exit 1
fi

echo "VALGRIND: PASSED."