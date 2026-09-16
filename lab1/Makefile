CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -std=c11

TARGET = lab1

SRC = main.c src/cipher.c

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)