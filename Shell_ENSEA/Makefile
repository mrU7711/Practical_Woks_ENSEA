CC = gcc
CFLAGS = -Wall -Wextra -g
OBJ = main.o shell.o

all: enseash

enseash: $(OBJ)
	$(CC) $(CFLAGS) -o enseash $(OBJ)

main.o: main.c shell.h
	$(CC) $(CFLAGS) -c main.c

shell.o: shell.c shell.h
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f *.o enseash
