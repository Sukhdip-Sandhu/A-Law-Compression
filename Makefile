CC = gcc
CFLAGS = -std=c11 -Wall -pedantic-errors
TARGET = a_law_compression
HEADER = modify_header

all: alaw header

alaw: $(TARGET)
$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

header: $(HEADER)
$(HEADER): $(HEADER).c
	$(CC) $(CFLAGS) -o $(HEADER) $(HEADER).c

clean:
	$(RM) $(TARGET)
