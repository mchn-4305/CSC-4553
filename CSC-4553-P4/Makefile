CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c11
TARGET = memsimulator
OBJS = memsimulator.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

memsimulator.o: memsimulator.c config.h

clean:
	rm -f $(TARGET) $(OBJS)
