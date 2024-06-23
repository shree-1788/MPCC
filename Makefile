CC = gcc
CFLAGS = -Wall -pthread
LDFLAGS = -pthread

SRCS = src/main.c src/server.c src/client.c src/user.c src/encryption.c src/logger.c
OBJS = $(SRCS:.c=.o)
TARGET = mpcc

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)