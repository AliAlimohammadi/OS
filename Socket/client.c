#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 8192 + 1   /* 8 kilobytes + 1 byte */

int main(int argc, char *argv[])
{
    // Handling command-line arguments:
    char *input_server_ip, *input_server_port, *text;
    bool ip_arg = false, port_arg = false, text_arg = false;

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
        else
        {
            text_arg = true;
            text = argv[i];
            printf("\n%s\n", text);
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
    if (!text_arg)
    {
        printf("No text message was given.\n");
        exit(1);
    }

    int clientSocket, ret;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // Create client socket:
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Client Socket is created.\n");

    // Server details:
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(input_server_ip);
    serverAddr.sin_port = htons(atoi(input_server_port));

    // Establish a connection with server:
    ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (ret < 0)
    {
        printf("[-]Error in connection.\n");
        exit(1);
    }
    printf("[+]Connected to Server.\n");

    // Listen (infinite loop):
    while (true)
    {
        printf("Client: \t");

        // Calculate the time taken by a cycle of exchanging messages.
        clock_t t = clock();

        send(clientSocket, text, strlen(text), 0);

        if (recv(clientSocket, buffer, BUFFER_SIZE, 0) < 0)
        {
            printf("[-]Error in receiving data.\n");
        }
        else
        {
            t = clock() - t;
            double elapsed_time = ((double) t * 1000) / CLOCKS_PER_SEC;   /* Time spent in milliseconds */

            printf("A cycle of exchanging messages took %f milliseconds to execute.\n", elapsed_time);
            printf("Server: \t%s\n", buffer);
            send(clientSocket, ":exit", strlen(":exit"), 0);
            close(clientSocket);
            printf("[-]Disconnected from server.\n");
            break;
        }
    }

    return 0;
}
