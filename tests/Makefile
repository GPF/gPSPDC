CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c99

.PHONY: test clean

test: phase1_helpers_test sh4_emit_encoding_test
	./phase1_helpers_test
	./sh4_emit_encoding_test

phase1_helpers_test: phase1_helpers_test.c
	$(CC) $(CFLAGS) -o $@ $<

sh4_emit_encoding_test: sh4_emit_encoding_test.c
	$(CC) $(CFLAGS) -I.. -o $@ $<

clean:
	rm -f phase1_helpers_test sh4_emit_encoding_test
