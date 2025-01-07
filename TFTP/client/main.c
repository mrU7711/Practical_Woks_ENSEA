// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prototypes for the client functions
void gettftp(const char *server, const char *file);
void puttftp(const char *server, const char *file);

#define USAGE_MSG "Usage: %s <operation> <server> <file>\n"
#define INVALID_OPERATION_MSG "Invalid operation: %s\n"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, USAGE_MSG, argv[0]);
        fprintf(stderr, "Operation: get or put\n");
        return 1;
    }

    char *operation = argv[1];
    char *server = argv[2];
    char *file = argv[3];

    if (strcmp(operation, "get") == 0) {
        gettftp(server, file);
    } else if (strcmp(operation, "put") == 0) {
        puttftp(server, file);
    } else {
        fprintf(stderr, INVALID_OPERATION_MSG, operation);
        return 1;
    }

    return 0;
}
