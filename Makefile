CC=gcc
CFLAGS=-c -Wall -Werror -g

all: lab01 testcase

lab01: server.o lab01.o
	$(CC) server.o lab01.o -o lab01

testcase: testcase.o
	$(CC) server.o testcase.o -o testcase

server.o: server.c
	$(CC) $(CFLAGS) server.c

lab01.o: lab01.c
	$(CC) $(CFLAGS) lab01.c

testharness.o: testharness.c
	$(CC) $(CFLAGS) testharness.c

clean:
	/bin/rm -f lab01 testcase *.o *.log

run:
	./lab01 8080

test:
	./testharness.sh

update:
	./update.sh
