#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <memory.h>



// Macros

#define USAGE_ERROR "Not enough arguments or too much arguments, 3 arguments are required.\n"
#define HOST_ERROR "Hostname not found.\n"
#define FILE_LABEL "File: "
#define OPERATION_LABEL "Operation: "
#define HOST_LABEL "Host: "
#define PORT_LABEL "Port: "
#define INFO_PREFIX "Communication with: "
#define ERROR_STATUS "Status error: "
#define LINE_BREAK "\n"

#define EXPECTED_ARGC 4
#define OPERATION_ARG 0
#define HOST_ARG 1
#define FILE_ARG 3
#define PORT_ARG 2

#define MAX_BUFFER_SIZE 128

char PORT_Name[MAX_BUFFER_SIZE];
char ipAddress[MAX_BUFFER_SIZE];
char fileName[MAX_BUFFER_SIZE];


int terminal = STDOUT_FILENO;
struct addrinfo hints;
struct addrinfo *result = NULL;
int status;


void checkFormat(int argc, char **argv) {
    if (argc != 4) {
        write(terminal, USAGE_ERROR, strlen(USAGE_ERROR));
        close(terminal);
        exit(EXIT_FAILURE);
    }
    
    
    
    write(terminal, OPERATION_LABEL, strlen(OPERATION_LABEL));
    write(terminal, argv[OPERATION_ARG], strlen(argv[OPERATION_ARG]));
    write(terminal, LINE_BREAK, strlen(LINE_BREAK));
    write(terminal, HOST_LABEL, strlen(HOST_LABEL));
    write(terminal, argv[HOST_ARG], strlen(argv[HOST_ARG]));
    write(terminal, LINE_BREAK, strlen(LINE_BREAK));
    write(terminal, FILE_LABEL, strlen(FILE_LABEL));
    write(terminal, argv[FILE_ARG], strlen(argv[FILE_ARG]));
    write(terminal, LINE_BREAK, strlen(LINE_BREAK));
}

void resolveHostname(char *hostname, char **argv) {
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       
    hints.ai_protocol = IPPROTO_UDP;
    
    status = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (status != 0) {
        write(terminal, HOST_ERROR, strlen(HOST_ERROR));
        write(terminal, ERROR_STATUS, strlen(ERROR_STATUS));
        write(terminal, gai_strerror(status), strlen(gai_strerror(status)));
        write(terminal, LINE_BREAK, strlen(LINE_BREAK));
        close(terminal);
        exit(EXIT_FAILURE);
    }
    status = getnameinfo(result->ai_addr, result->ai_addrlen, ipAddress, MAX_BUFFER_SIZE, PORT_Name, MAX_BUFFER_SIZE, NI_NUMERICHOST | NI_NUMERICSERV);
    if (status != 0) {
        write(terminal, HOST_ERROR, strlen(HOST_ERROR));
        close(terminal);
        freeaddrinfo(result);
        exit(EXIT_FAILURE);
    }
    write(terminal, INFO_PREFIX, strlen(INFO_PREFIX));
    write(terminal, ipAddress, strlen(ipAddress));
    write(terminal, LINE_BREAK, strlen(LINE_BREAK));
    write(terminal, PORT_Name, strlen(PORT_Name));
    write(terminal, LINE_BREAK, strlen(LINE_BREAK));

    int sd = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
    char rrq[128]={0};
    rrq[1]=1;
    sprintf(rrq+2,"%s",argv[3]);
    sprintf(rrq+3+strlen(argv[3]),"octet");

    sendto(sd,rrq,strlen(argv[3])+9,NULL,
        result->ai_addr,result->ai_addrlen);
    freeaddrinfo(result);
    
}


int main(int argc, char **argv) {
    
    checkFormat(argc, argv);
    resolveHostname(argv[HOST_ARG], argv);
    close(terminal);
    return 0;
}