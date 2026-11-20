CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = memsimulator
OBJS = memsimulator.o backing_store.o page_table.o phys_mem.o tlb.o
.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

memsimulator.o: memsimulator.c config.h backing_store.h page_table.h phys_mem.h tlb.hbacking_store.o: backing_store.c backing_store.h config.h
page_table.o: page_table.c page_table.h backing_store.h config.h
phys_mem.o: phys_mem.c phys_mem.h config.h
tlb.o: tlb.c tlb.h config.h

test: $(TARGET)
	./$(TARGET) tests/repeated-addresses.txt
	./$(TARGET) addresses.txt

clean:
	rm -f $(TARGET) $(OBJS)
