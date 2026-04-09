#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define MYPORT 3490
#define LENGTH 40

int main(int argc, char **argv) {
    int sockfd, n;
    char line[LENGTH];
    char remoteFile[80];
    char localFile[80];
    FILE *fp;

    struct sockaddr_in serverAddr;

    if(argc != 4) {
        printf("Usage: %s ip_address remotefile localfile\n", argv[0]);
        exit(1);
    }

    // Create socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(MYPORT);
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

    // Connect to server
    if(connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("Connection failed");
        exit(1);
    }

    // Prepare remote file path
    strncpy(remoteFile, argv[2], sizeof(remoteFile)-1);
    remoteFile[sizeof(remoteFile)-1] = '\0';

    // Send remote file path to server
    if(send(sockfd, remoteFile, strlen(remoteFile), 0) == -1) {
        perror("Send failed");
        close(sockfd);
        exit(1);
    }

    // Open local file to write
    strncpy(localFile, argv[3], sizeof(localFile)-1);
    localFile[sizeof(localFile)-1] = '\0';

    fp = fopen(localFile, "w");
    if(fp == NULL) {
        perror("File error");
        close(sockfd);
        exit(1);
    }

    // Receive file size
    uint32_t fileSizeNet;
    n = recv(sockfd, &fileSizeNet, sizeof(fileSizeNet), 0);
    if(n <= 0) {
        perror("Failed to receive file size");
        fclose(fp);
        close(sockfd);
        exit(1);
    }
    uint32_t fileSize = ntohl(fileSizeNet);
    printf("File size: %u bytes\n", fileSize);

    // Receive file content
    size_t totalReceived = 0;
    while(totalReceived < fileSize) {
        n = recv(sockfd, line, LENGTH, 0);
        if(n <= 0) {
            break; // connection closed or error
        }
        fwrite(line, 1, n, fp);
        totalReceived += n;
    }

    fclose(fp);
    close(sockfd);

    if(totalReceived == fileSize) {
        printf("Download finished successfully.\n");
    } else {
        printf("Download incomplete. Received %zu of %u bytes.\n", totalReceived, fileSize);
    }

    return 0;
}
