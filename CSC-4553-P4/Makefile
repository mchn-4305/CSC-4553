CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = memsimulator
OBJS = memsimulator.o backing_store.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

memsimulator.o: memsimulator.c config.h backing_store.h

backing_store.o: backing_store.c backing_store.h config.h

clean:
	rm -f $(TARGET) $(OBJS)
