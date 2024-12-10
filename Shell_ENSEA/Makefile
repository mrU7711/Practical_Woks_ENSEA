CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = enseash
OBJ = main.o shell.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

main.o: main.c shell.h
	$(CC) $(CFLAGS) -c main.c

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f $(OBJ) $(TARGET)
