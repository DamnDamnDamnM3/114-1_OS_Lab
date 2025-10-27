#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4444
#define IP_ADDRESS "127.0.0.1"
#define BUF_SIZE 1024

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUF_SIZE];

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        perror("[-] Error in creating Client Socket");
        exit(1);
    }
    printf("[+] Client Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("[-] Error in connection");
        exit(1);
    }
    printf("[+] Connected to Server.\n");

    while (1) {
        printf("Client : ");
        scanf("%s", buffer);

        if (strcmp(buffer, ":exit") == 0) {
            close(clientSocket);
            printf("[-] Disconnected from server.\n");
            exit(0);
        }

        if (send(clientSocket, buffer, strlen(buffer), 0) < 0) {
            perror("[-] Error in sending data");
            break;
        }

        memset(buffer, 0, BUF_SIZE);
        ssize_t r = recv(clientSocket, buffer, BUF_SIZE-1, 0);
        if (r < 0) {
            perror("[-] Error in receiving data");
            break;
        }
        buffer[r] = '\0';
        printf("Server : %s\n", buffer);
    }

    close(clientSocket);
    return 0;
}
