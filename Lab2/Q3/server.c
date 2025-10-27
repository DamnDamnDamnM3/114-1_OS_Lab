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
    int serverSocket, newSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size;
    char buffer[BUF_SIZE];

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("[-] Error in creating Server Socket");
        exit(1);
    }
    printf("[+] Server Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("[-] Error in binding");
        exit(1);
    }
    printf("[+] Bind to port %d\n", PORT);

    if (listen(serverSocket, 5) < 0) {
        perror("[-] Error in listening");
        exit(1);
    }
    printf("[+] Listening....\n");

    while (1) {
        addr_size = sizeof(clientAddr);
        newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size);
        if (newSocket < 0) {
            perror("[-] Error in accept");
            continue;
        }
        printf("[+] Connection accepted from %s:%d\n",
               inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        if (fork() == 0) {  // 子程序處理
            close(serverSocket);
            while (1) {
                memset(buffer, 0, BUF_SIZE);
                ssize_t r = recv(newSocket, buffer, BUF_SIZE-1, 0);
                if (r <= 0) {
                    // 客戶端強制關閉或連線中斷
                    printf("[-] Disconnected from %s:%d\n",
                           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                    break;
                }
                buffer[r] = '\0';

                if (strcmp(buffer, ":exit") == 0) {
                    printf("[-] Disconnected from %s:%d\n",
                           inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                    break;
                } else {
                    printf("Client : %s\n", buffer);
                    if (send(newSocket, buffer, strlen(buffer), 0) < 0) {
                        perror("[-] Error in sending data");
                        break;
                    }
                }
            }
            close(newSocket);
            exit(0);
        }
        close(newSocket);
    }

    close(serverSocket);
    return 0;
}
