# REFERENCE FINDER-APP MAKEFILE

P=aesdsocket
SOURCES= aesdsocket.c
OBJECTS= $(SOURCES:.c=.o)

all: $(P)

debug: CCFLAGS += -DDEBUG -g
debug: all

$(P).o: $(P).c
	$(CC) $(CCFLAGS) -Wall -std=c11 -D_POSIX_C_SOURCE=200809L -c $(SOURCES)

$(P): $(OBJECTS)
	$(CC) $(LDFLAGS) -lpthread -pthread $(OBJECTS) -o $(P)

clean:
	rm -f $(OBJECTS) $(P)

# TARGET = aesdsocket
# SOURCES = aesdsocket.c
# OBJECTS = $(SOURCES:.c=.o)
# CFLAGS = -g -Wall -O3
# CC=$(CROSS_COMPILE)gcc

# # Default target which builds the application
# all: ./$(TARGET)

# 	$(CC) $(CFLAGS) -c $(SOURCES) -o $(OBJECTS)

# # Clean target with removed the application
# clean:
# 	rm -f $(TARGET) $(OBJECTS)

# .PHONY: all clean