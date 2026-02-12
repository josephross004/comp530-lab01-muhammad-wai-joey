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
N=24

for ((i = 1; i < N+1; i++)); do
    echo "---------------------------"
  	./testcase "tc$i"
  	ec=$?
  	test_result "$ec" "tc$i"
	echo "\n\n\n-----------------------\n" >> "./valgrind_output.log"
	echo "Test case: tc$i" >> "./valgrind_output.log"
	echo "\n------------------------\n\n\n" >> "./valgrind_output.log"
	# run the test under Valgrind

	
	# Valgrind
	#     valgrind must detect memory leaks, invalid read/write, use-after-free errors, double frees, 
	#         unfreed LL nodes, unfreed strings in request_struct, and leaks from repeated test cycles
	#     For each test case, confirm exit status is correct (that valgrind errors trigger a FAIL) and no 
	#         leaks or invalid memory or uninit'd reads.
	#     Entire test suite has to run under Valgrind automatically. 

	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./testcase "tc$i" 2>> "./valgrind_output.log"
	ec=$?
	test_result "$ec" "tc$i (Valgrind)"

 	sleep 1
done


# ----------------------------------------------
# Honors section will included test cases below
# ---------------------------------------------- 


# Process reaping
#     1. Write a test in testharness.sh that starts the srever in the background and captures the parent PID
#     2. Send multiple client requests (5, 10, 20?) triggering a fork() in the server and the child handles the client 
#         and then the child has to exit. 
#     3. Check /proc for zombies - locate children of parent PID and if there are ANY zombies (ANY!!) return FAIL
#     4. Clean server shutdown. ^C should be a clean exit with no lingering children. 
#     5. Run the reaping test under back to back
#     6. Run the reaping test under heavy load (many requests)
#     7. Run the reaping under slower or faster request arrival times (is this possible or necessary?)
#
#     NB      Entire test suite has to run under Valgrind automatically. 

# (standard according to assignment - by default client.py 
# is going to use localhost:8080 .)
# Start server using valgrind 
valgrind --leak-check=full  --show-leak-kinds=all --errors-for-leak-kinds=all --trace-children=yes ./lab01 "8080" > server.log   2>&1 &

# capture server PID
SERVER_PID=$!

# give it a moment to bind
sleep 1

# SANITY CHECK: did the server crash? 
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "SERVER CRASHED: Check server.log for errors"
    exit 1
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

# Now count the zombies using /proc
ZOMBIE_COUNT=0
for pid in /proc/[0-9]*; do
    if [ -f "$pid/status" ]; then
    # proc output looks like this 
    # State:  S (sleeping)
    # ...
    # PPid:   1 
    # so awk '{print $2}' is just to get that second column out.
        PARENT=$(grep "PPid:" "$pid/status" | awk '{print $2}')
        STATE=$(grep "State:" "$pid/status" | awk '{print $2}')
        
        if [ "$PARENT" == "$SERVER_PID" ] && [ "$STATE" == "Z" ]; then
            ((ZOMBIE_COUNT++))
			# announce which process is a zombie - makes for easier debugging
            echo "ZOMBIE! $pid"
        fi
    fi
done

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