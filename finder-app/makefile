
TARGET = writer
SOURCES = writer.c
OBJECTS = $(SOURCES:.c=.o)
CFLAGS = -g -Wall -O3 -std=c99
CC=$(CROSS_COMPILE)gcc

# Default target which builds the application
all: $(TARGET)

	$(CC) $(CFLAGS) -c $(SOURCES) -o $(OBJECTS)

# Clean target with removed the application
clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean