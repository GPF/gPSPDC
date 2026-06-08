CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99

.PHONY: test clean

test: phase1_helpers_test
	./phase1_helpers_test

phase1_helpers_test: phase1_helpers_test.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f phase1_helpers_test
