#include "client.h"
#include "read.h"

struct Info_for_Client
{
    char ip_address[16]; // Assuming IPv4 address
    int port;
    int flag_for_backup;
};
void storage(int port1, char *ip1, char **command2, int flag)
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
    nm_addr.sin_port = htons(port1);
    nm_addr.sin_addr.s_addr = inet_addr(ip1);

    if (connect(storage_socket, (struct sockaddr *)&nm_addr, sizeof(nm_addr)) < 0)
    {
        perror("Error in connecting to the storage server");
        close(storage_socket);
        exit(1);
    }
    
    printf("Connected to the storage server\n");
    char command3[1024];
    strcpy(command3, command2[0]);

    if (flag)
    {
        strcat(command3, " ");
        strcat(command3, "./BACKUP");
        strcat(command3, command2[1] + 1);
    }
    else
    {
        strcat(command3, " ");
        strcat(command3, command2[1]);
    }
    printf("%s", command3);
    send(storage_socket, command3, strlen(command3), 0);
    char buffer[1024]; // Define buffer size according to your needs
    size_t totalSize = 0;
    char *command1[10];
    while (1)
    {
        ssize_t bytes_received = recv(storage_socket, buffer, sizeof(buffer), 0);

        if (bytes_received > 0)
        {
            // No more data to receive

            buffer[bytes_received] = '\0';

            // Split received data into newline-separated strings
            printf("%s", buffer);
            printf("\n");
            if (strstr(buffer, "STOP") != NULL)
            {
                printf("Received STOP packet. Ending communication.\n");
                break;
            }
        }
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
            printf("Received acknowledgment from storage server operation successful: %s\n", buffer1);
        }
    close(storage_socket);
    // Add a NULL pointer at the end to indicate the end of the array
}
void reads(char **command)
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
    printf("Connected to the naming server\n");
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
        ssize_t bytesRead;

        bytesRead = recv(storage_socket, buffer1, sizeof(buffer1), 0);

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
           printf("Received acknowledgment from naming server connection successful: %s\n", buffer1);
        }
    char command3[1024];
    strcpy(command3, command[0]);
    strcat(command3, " ");
    strcat(command3, command[1]);
    send(storage_socket, command3, strlen(command3), 0);

    struct Info_for_Client info_for_client;
    while (1)
    {
        int bytes_received = recv(storage_socket, &info_for_client, sizeof(info_for_client), 0);

        if (bytes_received > 0)
        {
            // Error or connection closed, break out of the loop
            break;
        }
    }
    // Send the Info_for_Client data to the naming server
    int port = info_for_client.port;
    char *ip = strdup(info_for_client.ip_address);
    int flag = info_for_client.flag_for_backup;
    storage(port, ip, command, flag);
    char ack[] = "ACK";

    send(storage_socket, ack, strlen(ack), 0);
    close(storage_socket);
}