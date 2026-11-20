
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11 -g

TARGET = memsimulator
OBJS = memsimulator.o backing_store.o page_table.o phys_mem.o tlb.o

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

memsimulator.o: memsimulator.c config.h backing_store.h page_table.h phys_mem.h tlb.h
	$(CC) $(CFLAGS) -c memsimulator.c -o $@

backing_store.o: backing_store.c backing_store.h config.h
	$(CC) $(CFLAGS) -c backing_store.c -o $@

page_table.o: page_table.c page_table.h config.h
	$(CC) $(CFLAGS) -c page_table.c -o $@

phys_mem.o: phys_mem.c phys_mem.h config.h
	$(CC) $(CFLAGS) -c phys_mem.c -o $@

tlb.o: tlb.c tlb.h config.h
	$(CC) $(CFLAGS) -c tlb.c -o $@

test: $(TARGET)
	./$(TARGET) addresses.txt 256 fifo

clean:
	rm -f $(TARGET) $(OBJS)
