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
N=27

for ((i = 1; i < N+1; i++)); do
    echo "----------------------------"
  	./testcase "tc$i"
  	ec=$?
  	test_result "$ec" "tc$i"
	# add header to log file
	echo "\n\n\n------------------------\n" >> "./valgrind_output.log"
	echo "Test case: tc$i" >> "./valgrind_output.log"
	echo "\n------------------------\n\n\n" >> "./valgrind_output.log"
	# run the test under Valgrind
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./testcase "tc$i" 2>> "./valgrind_output.log"
	ec=$?
	test_result "$ec" "tc$i (Valgrind)"

 	sleep 1
done


# ----------------------------------------------
# Honors section will included test cases below
# ---------------------------------------------- 

# Valgrind
#     valgrind must detect memory leaks, invalid read/write, use-after-free errors, double frees, 
#         unfreed LL nodes, unfreed strings in request_struct, and leaks from repeated test cycles
#     For each test case, confirm exit status is correct (that valgrind errors trigger a FAIL) and no 
#         leaks or invalid memory or uninit'd reads.
#     Entire test suite has to run under Valgrind automatically. 

# Process reaping
#     1. Write a test in testharness.sh that starts the srever in the background and captures the parent PID
#     2. Send multiple client requests (5, 10, 20?) triggering a fork() in the server and the child handles the client 
#         and then the child has to exit. 
#     3. Check /proc for zombies - locate children of parent PID and if there are ANY zombies (ANY!!) return FAIL
#     4. Clean server shutdown. ^C should be a clean exit with no lingering children. 
#     5. Run the reaping test under back to back
#     6. Run the reaping test under heavy load (many requests)
#     7. Run the reaping under slower or faster request arrival times (is this possible or necessary?)

