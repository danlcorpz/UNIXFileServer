#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>

#define LINELENGTH 40
#define BACKLOG 10
#define MYPORT 3490
#define LENGTH 80

int main(int argc, char **argv) {
    int listenfd, newsocketfd;
    FILE *fp;
    char line[LINELENGTH];
    struct sockaddr_in myAddr, clientAddr;
    socklen_t sin_size;
    char filePath[LENGTH];

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(1);
    }

    // Setup address
    myAddr.sin_family = AF_INET;
    myAddr.sin_port = htons(MYPORT);
    myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(myAddr.sin_zero, '\0', sizeof(myAddr.sin_zero));

    if (bind(listenfd, (struct sockaddr *)&myAddr, sizeof(myAddr)) < 0) {
        perror("bind failed");
        close(listenfd);
        exit(1);
    }

    if (listen(listenfd, BACKLOG) < 0) {
        perror("listen failed");
        close(listenfd);
        exit(1);
    }

    printf("Server listening on port %d\n", MYPORT);

    struct stat fileStat;

    while (1) {
        sin_size = sizeof(clientAddr);
        printf("\nWaiting for client...\n");

        if ((newsocketfd = accept(listenfd, (struct sockaddr *)&clientAddr, &sin_size)) < 0) {
            perror("accept failed");
            continue;
        }

        printf("Client %s connected.\n", inet_ntoa(clientAddr.sin_addr));

        if (fork() == 0) { // child process
            close(listenfd);

            // Receive file path from client
            int r = recv(newsocketfd, filePath, LENGTH - 1, 0);
            if (r <= 0) {
                close(newsocketfd);
                exit(0);
            }
            filePath[r] = '\0';

            printf("Requested file: %s\n", filePath);

            // Prevent directory traversal
            if (strstr(filePath, "..")) {
                printf("Rejected file path: %s\n", filePath);
                close(newsocketfd);
                exit(1);
            }

            // Open file
            fp = fopen(filePath, "r");
            if (!fp) {
                perror("File doesn't exist");
                uint32_t size = 0;
                send(newsocketfd, &size, sizeof(size), 0); // send 0 size
                close(newsocketfd);
                exit(1);
            }

            // Get file size
            if (stat(filePath, &fileStat) != 0) {
                perror("stat failed");
                fclose(fp);
                close(newsocketfd);
                exit(1);
            }

            printf("Sending %s to client\n", filePath);

            // Send file size (network byte order)
            uint32_t size = htonl(fileStat.st_size);
            send(newsocketfd, &size, sizeof(size), 0);
            printf("File size: %ld bytes\n", fileStat.st_size);

            // Send file contents
            size_t total = 0;
            while (fgets(line, LINELENGTH, fp) != NULL) {
                send(newsocketfd, line, strlen(line), 0);
                total += strlen(line);
                memset(line, 0, LINELENGTH);
            }

            fclose(fp);
            close(newsocketfd);
            printf("Completed transferring %zu bytes.\n", total);

            exit(0); // child exits
        }

        close(newsocketfd); // parent closes client socket
    }

    close(listenfd);
    return 0;
}
