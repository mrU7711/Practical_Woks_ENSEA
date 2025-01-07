// gettftp.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define BUFFER_SIZE 516
#define TIMEOUT_SEC 5
#define RESOLVING_SERVER "Resolving server address...\n"
#define SOCKET_CREATION_SUCCESS "UDP socket created.\n"
#define WAITING_DATA "Waiting for data...\n"
#define FILE_DOWNLOAD_SUCCESS "File %s downloaded successfully.\n"
#define ERROR_GETADDRINFO "Error resolving server address.\n"
#define ERROR_SOCKET "Error creating socket.\n"
#define ERROR_SEND_RRQ "Error sending RRQ.\n"
#define ERROR_RECV "Error receiving data.\n"
#define ERROR_FILE_OPEN "Error opening file for writing.\n"
#define BUFFER_SIZE 516 // 512 bytes of data + 4-byte header

void gettftp(const char *server, const char *file) {
    int sockfd;
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Resolve server address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // Force IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server, "1069", &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }

    // Create socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    // Build RRQ packet
    int offset = 0;
    buffer[offset++] = 0; // Opcode (1 for RRQ)
    buffer[offset++] = 1;
    strcpy(buffer + offset, file);
    offset += strlen(file) + 1;
    strcpy(buffer + offset, "octet"); // Transfer mode
    offset += strlen("octet") + 1;

    // Send RRQ
    if (sendto(sockfd, buffer, offset, 0, res->ai_addr, res->ai_addrlen) < 0) {
        perror("sendto");
        exit(1);
    }

    // Set a timeout for recvfrom
    struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Open file for writing
    FILE *fp = fopen(file, "wb");
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    struct sockaddr_storage server_addr;
    socklen_t addr_len = sizeof(server_addr);
    int block_num = 0;

    do {
        printf("Waiting for data...\n");
        bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                                  (struct sockaddr *)&server_addr, &addr_len);
        if (bytes_received < 0) {
            perror("recvfrom");
            fclose(fp);
            close(sockfd);
            freeaddrinfo(res);
            exit(1);
        }

        //block number from the received packet
        uint16_t received_block = (buffer[2] << 8) | buffer[3];
        printf("ACK received: opcode=%d, block=%d\n", buffer[1], received_block);

        if (buffer[1] != 3 || received_block != block_num + 1) {
            fprintf(stderr, "Invalid DATA packet: opcode=%d, block=%d. Exiting...\n",
                    buffer[1], received_block);
            fclose(fp);
            close(sockfd);
            freeaddrinfo(res);
            exit(1);
        }

        // Write data to file (skip 4-byte header)
        fwrite(buffer + 4, 1, bytes_received - 4, fp);
        block_num++;

        printf("Block #%d received (%d bytes).\n", block_num, bytes_received - 4);

        // Send ACK
        buffer[0] = 0;    // ACK opcode
        buffer[1] = 4;    // ACK opcode
        buffer[2] = (block_num >> 8) & 0xFF; // Block number high byte
        buffer[3] = block_num & 0xFF;        // Block number low byte
        sendto(sockfd, buffer, 4, 0, (struct sockaddr *)&server_addr, addr_len);

    } while (bytes_received == BUFFER_SIZE);

    printf("End of file reached. All data received.\n");

    fclose(fp);
    close(sockfd);
    freeaddrinfo(res);
    printf("File transfer complete.\n");
}

void gettftp(const char *server, const char *file) {
    int sockfd;
    struct addrinfo hints, *res;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    // Resolve server address
    printf(RESOLVING_SERVER);
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(server, "1069", &hints, &res) != 0) {
        perror(ERROR_GETADDRINFO);
        exit(1);
    }

    // Create socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror(ERROR_SOCKET);
        freeaddrinfo(res);
        exit(1);
    }
    printf(SOCKET_CREATION_SUCCESS);

    // Build RRQ packet
    int offset = 0;
    buffer[offset++] = 0; // Opcode (1 for RRQ)
    buffer[offset++] = 1;
    strcpy(buffer + offset, file);
    offset += strlen(file) + 1;
    strcpy(buffer + offset, "octet");
    offset += strlen("octet") + 1;

    // Send RRQ
    if (sendto(sockfd, buffer, offset, 0, res->ai_addr, res->ai_addrlen) < 0) {
        perror(ERROR_SEND_RRQ);
        freeaddrinfo(res);
        close(sockfd);
        exit(1);
    }

    // Set timeout for receiving data
    struct timeval tv = { .tv_sec = TIMEOUT_SEC, .tv_usec = 0 };
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Open file for writing
    FILE *fp = fopen(file, "wb");
    if (!fp) {
        perror(ERROR_FILE_OPEN);
        freeaddrinfo(res);
        close(sockfd);
        exit(1);
    }

    struct sockaddr_storage server_addr;
    socklen_t addr_len = sizeof(server_addr);
    int block_num = 0;

    do {
        printf(WAITING_DATA);
        bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &addr_len);
        if (bytes_received < 0) {
            perror(ERROR_RECV);
            fclose(fp);
            freeaddrinfo(res);
            close(sockfd);
            exit(1);
        }

        // block number from the received packet
        uint16_t received_block = (buffer[2] << 8) | buffer[3];
        printf("ACK received: opcode=%d, block=%d\n", buffer[1], received_block);

        if (buffer[1] != 3 || received_block != block_num + 1) {
            fprintf(stderr, "Invalid DATA packet: opcode=%d, block=%d. Exiting...\n",
                    buffer[1], received_block);
            fclose(fp);
            freeaddrinfo(res);
            close(sockfd);
            exit(1);
        }

        // Write data to file (skip 4-byte header)
        fwrite(buffer + 4, 1, bytes_received - 4, fp);
        block_num++;

        printf("Block #%d received (%d bytes).\n", block_num, bytes_received - 4);

        // Send ACK
        buffer[0] = 0;
        buffer[1] = 4; // ACK opcode
        buffer[2] = (block_num >> 8) & 0xFF; // Block number high byte
        buffer[3] = block_num & 0xFF;        // Block number low byte
        sendto(sockfd, buffer, 4, 0, (struct sockaddr *)&server_addr, addr_len);

    } while (bytes_received == BUFFER_SIZE);

    printf("End of file reached. All data received.\n");

    fclose(fp);
    close(sockfd);
    freeaddrinfo(res);
    printf(FILE_DOWNLOAD_SUCCESS, file);
}
