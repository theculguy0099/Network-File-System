#include "client.h"
#include "makedir.h"
void makedirs(char **command)
{
    int storage_socket;
    struct sockaddr_in nm_addr;
    socklen_t nm_addr_size;

    storage_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (storage_socket < 0)
    {
        perror("Error in storage server socket creation");
        exit(1);
    }

    nm_addr.sin_family = AF_INET;
    nm_addr.sin_port = htons(NM_PORT);
    nm_addr.sin_addr.s_addr = inet_addr(NM_SERVER_IP);

    if (connect(storage_socket, (struct sockaddr *)&nm_addr, sizeof(nm_addr)) < 0)
    {
        perror("Error in connecting to the naming server");
        close(storage_socket);
        exit(1);
    }
    struct timeval timeout1;
        timeout1.tv_sec = 1;
        timeout1.tv_usec = 0;

        if (setsockopt(storage_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout1, sizeof(timeout1)) < 0)
        {
            perror("setsockopt for receive timeout");
            close(storage_socket);
        }

        // Example: Attempt to receive data with a timeout
        char buffer1[1024];
        ssize_t bytesRead1;

        bytesRead1 = recv(storage_socket, buffer1, sizeof(buffer1), 0);

        if (bytesRead1 == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                printf("Receive operation timed out.\n");
            }
            else
            {
                perror("recv");
            }
        }
        else
        {
            printf("Received acknowledgment from naming server connection successful: %s\n", buffer1);
        }
    printf("Connected to the naming server\n");
    char command3[1024];
    strcpy(command3, command[0]);
    strcat(command3, " ");
    strcat(command3, command[1]);
    strcat(command3, " ");
    strcat(command3, command[2]);
    send(storage_socket, command3, strlen(command3), 0);
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (setsockopt(storage_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt for receive timeout");
        close(storage_socket);
    }

    // Example: Attempt to receive data with a timeout
    char buffer[1024];
    ssize_t bytesRead;

    bytesRead = recv(storage_socket, buffer, sizeof(buffer), 0);

    if (bytesRead == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            printf("Receive operation timed out.\n");
        }
        else
        {
            perror("recv");
        }
    }
    else
    {

        printf("Received acknowledgment from naming server operation successful: %s\n", buffer);
    }

    close(storage_socket);
}