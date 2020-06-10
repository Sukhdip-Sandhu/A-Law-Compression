CC = gcc
CFLAGS = -std=c11 -Wall -pedantic-errors
TARGET = main

all: $(TARGET)
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean:
	$(RM) $(TARGET)
