
CC = gcc
AR = ar

CFLAGS = -Wall -Wextra -g -std=gnu11 -fno-omit-frame-pointer

LIBRARY = liblwp.a
OBJECTS = lwp.o rr.o magic64.o

.PHONY: all clean test

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

lwp.o: lwp.c lwp.h fp.h rr.h
	$(CC) $(CFLAGS) -c lwp.c -o lwp.o

rr.o: rr.c rr.h lwp.h fp.h
	$(CC) $(CFLAGS) -c rr.c -o rr.o

magic64.o: magic64.S
	$(CC) -o magic64.o -c magic64.S

test_rr: tests/test_rr.c rr.o
	$(CC) $(CFLAGS) -I. tests/test_rr.c rr.o -o test_rr

test: test_rr
	./test_rr

clean:
	rm -f $(OBJECTS) $(LIBRARY) test_rr