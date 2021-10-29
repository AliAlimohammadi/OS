#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define BUFFER_SIZE 8192 + 1   /* 8 kilobytes + 1 byte */
#define MAX_NUMBER_OF_CONNECTIONS 10

int number_of_clients = 0;   /* Counting the number of connected clients. */

int main(int argc, char *argv[])
{

    // Handling command-line arguments:
    char *input_server_ip, *input_server_port;
    bool ip_arg = false, port_arg = false;

    for (int i = 1; i < argc; i++)   /* Skip argv[0] (program name). */
    {
        if (!strcmp(argv[i], "-h"))   /* Process the given arguments. */
        {
            ip_arg = true;
            if (i + 1 < argc)   /* Check if there are enough arguments in argv. */
            {
                input_server_ip = argv[++i];   /* Increment the index so that we do not check
                                                * these arguments the next time through the loop. */
            }
            else
            {
                printf("Necessary argument(s) is/are missing.\n");
                exit(1);
            }
        }
        else if (!strcmp(argv[i], "-p"))
        {
            port_arg = true;
            if (i + 1 < argc)   /* Check if there are enough arguments in argv. */
            {
                input_server_port = argv[++i];   /* Increment the index so that we do not check
                                                  * these arguments the next time through the loop. */
            }
            else
            {
                printf("Necessary argument(s) is/are missing.\n");
                exit(1);
            }
        }
    }

    // Check if necessary arguments were provided in argv:
    if (!ip_arg)
    {
        printf("No server IP address was given.\n");
        exit(1);
    }
    if (!port_arg)
    {
        printf("No server port number was given.\n");
        exit(1);
    }

    int sockfd, ret;
    struct sockaddr_in server_address;

    int newSocket;
    struct sockaddr_in newAddr;

    socklen_t addr_size;

    char buffer[BUFFER_SIZE];
    pid_t childpid;

    // Create server socket:
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Server Socket is created.\n");

    // Server details:
    memset(&server_address, '\0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(input_server_ip);
    server_address.sin_port = htons(atoi(input_server_port));

    // Bind to port:
    ret = bind(sockfd, (struct sockaddr*)&server_address, sizeof(server_address));
    if (ret < 0)
    {
        printf("[-]Error in binding.\n");
        exit(1);
    }
    printf("[+]Bind to port %d\n", atoi(input_server_port));

    if (!listen(sockfd, MAX_NUMBER_OF_CONNECTIONS))
        printf("[+]Listening....\n");
    else
        printf("[-]Error in binding.\n");

    // Listen (infinite loop):
    while (true)
    {
        // Accept new connections:
        newSocket = accept(sockfd, (struct sockaddr*)&newAddr, &addr_size);
        if (newSocket < 0)
            exit(1);

        printf("Number of Clients: %d\n", ++number_of_clients);

        printf("Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        if (!(childpid = fork()))
        {
            close(sockfd);

            while (true)
            {
                // Receive messages:
                recv(newSocket, buffer, BUFFER_SIZE, 0);
                if (!strcmp(buffer, ":exit"))
                {
                    printf("Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                }
                else
                {
                    printf("Client: %s\n", buffer);
                    char *temp = "Received";
                    send(newSocket, temp, strlen(temp), 0);
                    bzero(buffer, sizeof(buffer));
                }
            }
        }

    }

    close(newSocket);

    return 0;
}
