#include "client.h"
#include "write.h"

struct sum
{
    char ip1[16]; // Assuming IPv4 address
    int port1;
    char ip2[16]; // Assuming IPv4 address
    int port2;
    char ip3[16]; // Assuming IPv4 address
    int port3;
};
struct sum info_for_client;
void storage1(char **command2)
{

    int storage_socket;
    struct sockaddr_in nm_addr;
    socklen_t nm_addr_size;
    int storage_socket1;
    struct sockaddr_in nm_addr1;
    socklen_t nm_addr_size1;
    int storage_socket2;
    struct sockaddr_in nm_addr2;
    socklen_t nm_addr_size2;
    storage_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (storage_socket < 0)
    {
        perror("Error in storage server socket creation");
        exit(1);
    }

    nm_addr.sin_family = AF_INET;
    nm_addr.sin_port = htons(info_for_client.port1);
    nm_addr.sin_addr.s_addr = inet_addr(info_for_client.ip1);

    if (connect(storage_socket, (struct sockaddr *)&nm_addr, sizeof(nm_addr)) < 0)
    {
        perror("Error in connecting to the storage server");
        close(storage_socket);
        exit(1);
    }
    printf("Connected to the storage server\n");
    if (info_for_client.port2 != -1)
    {
        storage_socket1 = socket(AF_INET, SOCK_STREAM, 0);
        storage_socket2 = socket(AF_INET, SOCK_STREAM, 0);
        if (storage_socket1 < 0)
        {
            perror("Error in storage server socket creation");
            exit(1);
        }
        if (storage_socket2 < 0)
        {
            perror("Error in storage server socket creation");
            exit(1);
        }
        nm_addr1.sin_family = AF_INET;
        nm_addr1.sin_port = htons(info_for_client.port2);
        nm_addr1.sin_addr.s_addr = inet_addr(info_for_client.ip2);
        nm_addr2.sin_family = AF_INET;
        nm_addr2.sin_port = htons(info_for_client.port3);
        nm_addr2.sin_addr.s_addr = inet_addr(info_for_client.ip3);

        if (connect(storage_socket1, (struct sockaddr *)&nm_addr1, sizeof(nm_addr1)) < 0)
        {
            perror("Error in connecting to the storage server");
            close(storage_socket1);
            exit(1);
        }
        printf("Connected to the storage server\n");
        if (connect(storage_socket2, (struct sockaddr *)&nm_addr2, sizeof(nm_addr2)) < 0)
        {
            perror("Error in connecting to the storage server");
            close(storage_socket2);
            exit(1);
        }
        printf("Connected to the storage server\n");
        char command3[1024];
        char command5[1024];
        int i = 0;
        int count = 0;
        int count1 = 0;
        while (command2[i] != NULL)
        {
            if (count == 0)
            {
                strcpy(command3, command2[i]);
                strcpy(command5, command2[i]);
            }
            else if (count > 0)
            {
                strcat(command3, " ");
                strcat(command3, command2[i]);
                strcat(command5, " ");
                if (count1 == 1)
                {
                    strcat(command5, "./BACKUP");
                    strcat(command5, command2[i] + 1);
                }
                else
                {
                    strcat(command5, command2[i]);
                }
            }
            count = 1;
            count1 = 1;
            i++;
        }

        printf("%s\n", command5);
        send(storage_socket, command3, strlen(command3), 0);
        send(storage_socket1, command5, strlen(command5), 0);
        send(storage_socket2, command5, strlen(command5), 0);
        char sd[2];
        scanf("%s", sd);
        char cc;
        scanf("%c", &cc);
        send(storage_socket, sd, strlen(sd), 0);
        send(storage_socket1, sd, strlen(sd), 0);
        send(storage_socket2, sd, strlen(sd), 0);
        char command9[4096];
        fgets(command9, 4096, stdin);
        send(storage_socket, command9, strlen(command9), 0);
        send(storage_socket1, command9, strlen(command9), 0);
        send(storage_socket2, command9, strlen(command9), 0);
        close(storage_socket1);
        close(storage_socket2);
    }
    else
    {
        char command3[1024];
        int i = 0;
        int count = 0;
        while (command2[i] != NULL)
        {
            if (count == 0)
            {
                strcpy(command3, command2[i]);
            }
            else if (count > 0)
            {
                strcat(command3, " ");
                strcat(command3, command2[i]);
            }
            count = 1;
            i++;
        }

        send(storage_socket, command3, strlen(command3), 0);
        printf("Enter whether you want to append or overwrite the file: (a/w)\n");
        char sd[2];
        scanf("%s", sd);
        char cc;
        scanf("%c", &cc);
        send(storage_socket, sd, strlen(sd), 0);
        char command9[4096];
        printf("Enter the data to be written to the file: \n");
        fgets(command9, 4096, stdin);
        send(storage_socket, command9, strlen(command9), 0);
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
    bzero(buffer1, sizeof(buffer1));
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
void writes(char **command)
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
    char command3[1024];
    int i = 0;
    int count = 0;
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
    while (command[i] != NULL)
    {
        if (count == 0)
        {
            strcpy(command3, command[i]);
        }
        else if (count > 0)
        {
            strcat(command3, " ");
            strcat(command3, command[i]);
        }
        count = 1;
        i++;
    }

    send(storage_socket, command3, strlen(command3), 0);

    while (1)
    {
        int bytes_received = recv(storage_socket, &info_for_client, sizeof(info_for_client), 0);

        if (bytes_received > 0)
        {
            // Error or connection closed, break out of the loop
            break;
        }
    }

    storage1(command);
    char ack[] = "ACK";
    send(storage_socket, ack, strlen(ack), 0);
    close(storage_socket);
}