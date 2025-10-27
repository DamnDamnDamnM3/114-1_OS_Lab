#include #include #include #include #include #include #include #include
#define PORT 4444
#define IP_ADDRESS "127.0.0.1"

int main()
{

int clientSocket;
struct sockaddr_in serverAddr;
int checkConnect, checkRecv;
char buffer[1024];

/* replace this line with socket() */
if (/* check error */)
{
printf("[-]Error in creating Client Socket.\n");
exit(1);
}
printf("[+]Client Socket is created.\n");

memset(&serverAddr, '\0', sizeof(serverAddr));
serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(PORT);
serverAddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);

/* replace this line with connect() */
if (/* check error */)
{
printf("[-]Error in connection.\n");
exit(1);
}
printf("[+]Connected to Server.\n");

while (1)
{
printf("Client : ");
scanf("%s", &buffer[0]);
/* replace this line with send() */

if (strcmp(buffer, ":exit") == 0)
{
close(clientSocket);
printf("[-]Disconnected from server.\n");
exit(1);
}

/* replace this line with recv() */
if (/* check error */)
{
printf("[-]Error in receiving data.\n");
}
else
{
printf("Server : %s\n", buffer);
}
}

return 0;
}
