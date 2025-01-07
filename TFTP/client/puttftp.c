// puttftp.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

#define MAX_BUFFER_SIZE 516
#define TIMEOUT_SEC 5
#define RESOLVING_SERVER "Resolving server address...\n"
#define SOCKET_CREATION_SUCCESS "UDP socket created.\n"
#define FILE_TRANSFER_COMPLETE "File transfer complete.\n"
#define ERROR_GETADDRINFO "Error resolving server address.\n"
#define ERROR_SOCKET "Error creating socket.\n"
#define ERROR_WRQ "Error sending WRQ.\n"
#define ERROR_ACK "Error receiving ACK.\n"
#define ERROR_FILE_OPEN "Error opening file for reading.\n"
#define ERROR_DATA_SEND "Error sending data block.\n"
#define WAITING_FOR_ACK "Waiting for ACK...\n"
#define INVALID_ACK "Invalid ACK received.\n"
#define BLOCK_SENT "Block #%d sent (%ld bytes).\n"
#define ACK_RECEIVED "ACK received: opcode=%d, block=%d\n"

void puttftp(const char *server, const char *file) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    printf(RESOLVING_SERVER);
    if (getaddrinfo(server, "1069", &hints, &res) != 0) {
        perror(ERROR_GETADDRINFO);
        return;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror(ERROR_SOCKET);
        freeaddrinfo(res);
        return;
    }
    printf(SOCKET_CREATION_SUCCESS);

    char request[MAX_BUFFER_SIZE];
    memset(request, 0, MAX_BUFFER_SIZE);
    request[0] = 0x00;
    request[1] = 0x02; // WRQ opcode
    strcpy(&request[2], file);
    strcpy(&request[strlen(file) + 3], "octet");

    ssize_t sent_bytes = sendto(sockfd, request, strlen(file) + strlen("octet") + 4, 0, res->ai_addr, res->ai_addrlen);
    if (sent_bytes == -1) {
        perror(ERROR_WRQ);
        freeaddrinfo(res);
        close(sockfd);
        return;
    }

    char ack[4];
    struct sockaddr_storage server_addr;
    socklen_t addr_len = sizeof(server_addr);

    ssize_t received_bytes = recvfrom(sockfd, ack, 4, 0, (struct sockaddr *)&server_addr, &addr_len);
    if (received_bytes == -1) {
        perror(ERROR_ACK);
        freeaddrinfo(res);
        close(sockfd);
        return;
    }

    uint16_t opcode = ntohs(*(uint16_t *)ack);
    uint16_t received_block = ntohs(*(uint16_t *)(ack + 2));
    printf(ACK_RECEIVED, opcode, received_block);

    if (opcode != 4 || received_block != 0) {
        fprintf(stderr, INVALID_ACK);
        freeaddrinfo(res);
        close(sockfd);
        return;
    }

    FILE *input_file = fopen(file, "rb");
    if (!input_file) {
        perror(ERROR_FILE_OPEN);
        freeaddrinfo(res);
        close(sockfd);
        return;
    }

    char buffer[MAX_BUFFER_SIZE];
    int block_num = 0;

    while (1) {
        size_t bytes_read = fread(buffer + 4, 1, 512, input_file);
        if (bytes_read < 0) {
            perror(ERROR_FILE_OPEN);
            fclose(input_file);
            freeaddrinfo(res);
            close(sockfd);
            return;
        }

        block_num++;
        buffer[0] = 0x00;
        buffer[1] = 0x03; // DATA opcode
        buffer[2] = (block_num >> 8) & 0xFF; // Block number high byte
        buffer[3] = block_num & 0xFF;        // Block number low byte

        ssize_t sent_data = sendto(sockfd, buffer, bytes_read + 4, 0, (struct sockaddr *)&server_addr, addr_len);
        if (sent_data == -1) {
            perror(ERROR_DATA_SEND);
            fclose(input_file);
            freeaddrinfo(res);
            close(sockfd);
            return;
        }
        printf(BLOCK_SENT, block_num, bytes_read);

        printf(WAITING_FOR_ACK);
        received_bytes = recvfrom(sockfd, ack, 4, 0, (struct sockaddr *)&server_addr, &addr_len);
        if (received_bytes == -1) {
            perror(ERROR_ACK);
            fclose(input_file);
            freeaddrinfo(res);
            close(sockfd);
            return;
        }

        opcode = ntohs(*(uint16_t *)ack);
        received_block = ntohs(*(uint16_t *)(ack + 2));
        printf(ACK_RECEIVED, opcode, received_block);

        if (opcode != 4 || received_block != block_num) {
            fprintf(stderr, INVALID_ACK);
            continue; // Resend the same block
        }

        if (bytes_read < 512) {
            printf("End of file reached. All data sent.\n");
            break;
        }
    }

    fclose(input_file);
    freeaddrinfo(res);
    close(sockfd);
    printf(FILE_TRANSFER_COMPLETE);
}
