#include "naming_server.h"

int storage_server_count = 0;
int client_count = 0;
struct ServerInfo storage_servers[MAX_SS];
struct for_copy backups[MAX_SS];

int storage_server_sockets[MAX_SS];
int ss_is_down[MAX_SS];
int client_sockets[MAX_SS];
struct recently_accessed last;
Trie T;
FILE *file;
int ip5 = -1;
int port5 = -1;
Queue *q;

// Let 10 different pages can be requested (pages to be
// referenced are numbered from 0 to 9
Hash *hash;
// A utility function to create a new Queue Node. The queue
// Node will store the given 'pageNumber'
QNode *newQNode(unsigned pageNumber, int hj)
{
    // Allocate memory and assign 'pageNumber'
    QNode *temp = (QNode *)malloc(sizeof(QNode));
    temp->pageNumber = pageNumber;
    temp->ip = hj;

    // Initialize prev and next as NULL
    temp->prev = temp->next = NULL;

    return temp;
}

// A utility function to create an empty Queue.
// The queue can have at most 'numberOfFrames' nodes
Queue *createQueue(int numberOfFrames)
{
    Queue *queue = (Queue *)malloc(sizeof(Queue));

    // The queue is empty
    queue->count = 0;
    queue->front = queue->rear = NULL;

    // Number of frames that can be stored in memory
    queue->numberOfFrames = numberOfFrames;

    return queue;
}

// A utility function to create an empty Hash of given
// capacity
Hash *createHash(int capacity)
{
    // Allocate memory for hash
    Hash *hash = (Hash *)malloc(sizeof(Hash));
    hash->capacity = capacity;

    // Create an array of pointers for referring queue nodes
    hash->array = (QNode **)malloc(hash->capacity * sizeof(QNode *));

    // Initialize all hash entries as empty
    int i;
    for (i = 0; i < hash->capacity; ++i)
        hash->array[i] = NULL;

    return hash;
}

// A function to check if there is slot available in memory
int AreAllFramesFull(Queue *queue)
{
    return queue->count == queue->numberOfFrames;
}

// A utility function to check if queue is empty
int isQueueEmpty(Queue *queue)
{
    return queue->rear == NULL;
}

// A utility function to delete a frame from queue
void deQueue(Queue *queue)
{
    if (isQueueEmpty(queue))
        return;

    // If this is the only node in list, then change front
    if (queue->front == queue->rear)
        queue->front = NULL;

    // Change rear and remove the previous rear
    QNode *temp = queue->rear;
    queue->rear = queue->rear->prev;

    if (queue->rear)
        queue->rear->next = NULL;

    free(temp);

    // decrement the number of full frames by 1
    queue->count--;
}
void deQueue1(Queue *queue, int ha)
{
    if (isQueueEmpty(queue))
        return;

    // If this is the only node in list, then change front
    if (queue->front == queue->rear)
    {
        queue->front = NULL;
        return;
    }
    QNode *temp = queue->front;
    // Change rear and remove the previous rear
    while (temp->ip != ha || temp->next != NULL)
    {
        temp = temp->next;
    }
    QNode *temp1 = temp->prev;
    int flag = 0;
    int flag1 = 0;
    if (!temp1)
    {
        temp1->next = temp->next;
        flag = 1;
    }
    if (!temp->next)
    {
        temp->next->prev = temp1;
        flag1 = 2;
    }

    temp->next = NULL;
    temp->prev = NULL;

    free(temp);

    queue->count--;
}
// A function to add a page with given 'pageNumber' to both
// queue and hash
void Enqueue(Queue *queue, Hash *hash, unsigned pageNumber, int hj)
{
    // If all frames are full, remove the page at the rear
    if (AreAllFramesFull(queue))
    {
        // remove page from hash
        hash->array[queue->rear->pageNumber] = NULL;
        deQueue(queue);
    }

    // Create a new node with given page number,
    // And add the new node to the front of queue
    QNode *temp = newQNode(pageNumber, hj);
    temp->next = queue->front;

    // If queue is empty, change both front and rear
    // pointers
    if (isQueueEmpty(queue))
        queue->rear = queue->front = temp;
    else // Else change the front
    {
        queue->front->prev = temp;
        queue->front = temp;
    }

    // Add page entry to hash also
    hash->array[pageNumber] = temp;

    // increment number of full frames
    queue->count++;
}

unsigned hashing(char *string)
{
    unsigned sum = 0;
    long long int a = 13;
    int i = 0;
    while (string[i] != '\0')
    {
        sum += (int)(string[i]);
        sum = sum * a;
        sum = sum % 99997;

        i++;
    }

    return sum % 99997;
}
void ReferencePage(Queue *queue, Hash *hash, unsigned pageNumber)
{
    QNode *reqPage = hash->array[pageNumber];

    // the page is not in cache, bring it
    if (reqPage == NULL)
    {
        // Enqueue(queue, hash, pageNumber);
        return;
    }
    ip5 = reqPage->ip;
}
void yh(Queue *queue, Hash *hash, unsigned pageNumber)
{
    QNode *reqPage = hash->array[pageNumber];
    // page is there and not at front, change pointer
    if (reqPage != queue->front)
    {
        // Unlink rquested page from its current location
        // in queue.

        reqPage->prev->next = reqPage->next;
        if (reqPage->next)
            reqPage->next->prev = reqPage->prev;

        // If the requested page is rear, then change rear
        // as this node will be moved to front
        if (reqPage == queue->rear)
        {
            queue->rear = reqPage->prev;
            queue->rear->next = NULL;
        }

        // Put the requested page before current front
        reqPage->next = queue->front;
        reqPage->prev = NULL;

        // Change prev of current front
        reqPage->next->prev = reqPage;

        // Change front to the requested page
        queue->front = reqPage;
    }
}
void *handle_downing_of_server(void *arg)
{
    ThreadArgs *myArgs = (ThreadArgs *)arg;
    char BUFFER[1024];
    while (1)
    {
        bzero(BUFFER, sizeof(BUFFER));
        recv(myArgs->arg1, BUFFER, sizeof(BUFFER), 0);
        // printf("buffer is    %s\n", BUFFER);
        if (strcmp(BUFFER, "offline") == 0)
        {
            ss_is_down[myArgs->arg2] = 1;
            printf("STORAGE SERVER %d IS DOWN NOW!\n", myArgs->arg2 + 1);
        }
        else if (strcmp(BUFFER, "online") == 0)
        {
            ss_is_down[myArgs->arg2] = 0;
            printf("STORAGE SERVER %d IS BACK ONLINE!\n", myArgs->arg2 + 1);
            struct for_copy new_backup;
            recv(myArgs->arg1, &new_backup, sizeof(new_backup), 0);
            int j = 0;
            while (new_backup.to_be_sent[j][0] != '\0')
            {
                printf("%s\n", new_backup.to_be_sent[j]);
                j++;
            }

            char command[1024];
            char command1[1024];
            strcpy(command, "PRIV_DELETE ./BACKUP");
            strcpy(command1, "RECIEVE ./BACKUP");
            if (myArgs->arg2 == 0)
            {
                send(storage_server_sockets[1], command, strlen(command), 0);
                usleep(100);
                send(storage_server_sockets[2], command, strlen(command), 0);
                usleep(1000);
                send(storage_server_sockets[1], command1, strlen(command1), 0);
                int b;
                while (1)
                {
                    recv(storage_server_sockets[1], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[1], &new_backup, sizeof(new_backup), 0);

                usleep(1000);
                send(storage_server_sockets[1], command1, strlen(command1), 0);
                while (1)
                {
                    recv(storage_server_sockets[1], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[1], &backups[2], sizeof(new_backup), 0);

                usleep(1000);
                send(storage_server_sockets[2], command1, strlen(command1), 0);
                while (1)
                {
                    recv(storage_server_sockets[2], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[2], &new_backup, sizeof(new_backup), 0);

                usleep(1000);
                send(storage_server_sockets[2], command1, strlen(command1), 0);
                while (1)
                {
                    recv(storage_server_sockets[2], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[2], &backups[1], sizeof(new_backup), 0);
            }
            else if (myArgs->arg2 == 1)
            {
                send(storage_server_sockets[0], command, strlen(command), 0);
                usleep(100);
                send(storage_server_sockets[2], command, strlen(command), 0);

                usleep(1000);
                send(storage_server_sockets[0], command1, strlen(command1), 0);
                int b;
                while (1)
                {
                    recv(storage_server_sockets[0], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[0], &new_backup, sizeof(new_backup), 0);

                usleep(1000);
                send(storage_server_sockets[0], command1, strlen(command1), 0);
                while (1)
                {
                    recv(storage_server_sockets[0], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[0], &backups[2], sizeof(new_backup), 0);

                usleep(1000);
                send(storage_server_sockets[2], command1, strlen(command1), 0);
                while (1)
                {
                    recv(storage_server_sockets[2], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[2], &new_backup, sizeof(new_backup), 0);

                usleep(1000);
                send(storage_server_sockets[2], command1, strlen(command1), 0);
                while (1)
                {
                    recv(storage_server_sockets[2], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[2], &backups[0], sizeof(new_backup), 0);
            }
            else
            {
                send(storage_server_sockets[myArgs->arg2 - 1], command, strlen(command), 0);
                usleep(100);
                send(storage_server_sockets[myArgs->arg2 - 2], command, strlen(command), 0);

                usleep(1000);
                send(storage_server_sockets[myArgs->arg2 - 1], command1, strlen(command1), 0);
                int b;
                while (1)
                {
                    recv(storage_server_sockets[myArgs->arg2 - 1], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[myArgs->arg2 - 1], &new_backup, sizeof(new_backup), 0);

                usleep(1000);
                send(storage_server_sockets[myArgs->arg2 - 1], command1, strlen(command1), 0);
                while (1)
                {
                    recv(storage_server_sockets[myArgs->arg2 - 1], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[myArgs->arg2 - 1], &backups[myArgs->arg2 - 2], sizeof(new_backup), 0);

                usleep(1000);
                send(storage_server_sockets[myArgs->arg2 - 2], command1, strlen(command1), 0);
                while (1)
                {
                    recv(storage_server_sockets[myArgs->arg2 - 2], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[myArgs->arg2 - 2], &new_backup, sizeof(new_backup), 0);

                usleep(1000);
                send(storage_server_sockets[myArgs->arg2 - 2], command1, strlen(command1), 0);
                while (1)
                {
                    recv(storage_server_sockets[myArgs->arg2 - 2], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                send(storage_server_sockets[myArgs->arg2 - 2], &backups[myArgs->arg2 - 1], sizeof(new_backup), 0);
            }
        }
    }
}
void handle_nm_ss_comm(int socket)
{
    struct ServerInfo server_info;
    struct for_copy backup;

    while (1)
    {
        int bytes_received = recv(socket, &server_info, sizeof(server_info), 0);

        if (bytes_received > 0)
        {
            break;
        }
    }
    usleep(1000);
    int a = 12;
    int bytes_sent = send(socket, &a, sizeof(a), 0);
    if (bytes_sent <= 0)
    {
        perror("Error sending data to naming server");
        close(socket);
        exit(1);
    }
    while (1)
    {
        if (recv(socket, &backup, sizeof(backup), 0) == -1)
        {
            perror("Receive failed");
            exit(EXIT_FAILURE);
        }
        if (backup.to_be_sent[0][0] != '\0')
            break;
    }

    int ss_id = storage_server_count;
    // printf("%d\n", storage_server_sockets[ss_id]);
    storage_servers[storage_server_count] = server_info;
    backups[storage_server_count++] = backup;

    printf("Received data from storage server:\n");
    printf("IP address: %s\n", server_info.ip_address);
    printf("Port for NM Connection: %d\n", server_info.nm_connection_port);
    printf("Port for Client Connection: %d\n", server_info.client_connection_port);
    printf("No. of paths: %d\n", server_info.no_of_paths);
    printf("Accessible paths:\n");

    for (int i = 0; i < server_info.no_of_paths; i++)
    {
        printf("%s\n", server_info.accessible_path[i]);
        T = Insert(T, server_info.accessible_path[i], ss_id);
    }

    int j = 0;
    while (backup.to_be_sent[j][0] != '\0')
    {
        printf("%s\n", backup.to_be_sent[j]);
        j++;
    }

    if (storage_server_count == 3)
    {
        char command[1024];
        strcpy(command, "RECIEVE ./BACKUP");

        int sent_bytes = send(storage_server_sockets[0], command, strlen(command), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        int b;
        while (1)
        {
            recv(storage_server_sockets[0], &b, sizeof(b), 0);
            if (b == 12)
                break;
        }

        usleep(1000);
        sent_bytes = send(storage_server_sockets[0], &backups[2], sizeof(backups[2]), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        usleep(1000);

        sent_bytes = send(storage_server_sockets[1], command, strlen(command), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        while (1)
        {
            recv(storage_server_sockets[1], &b, sizeof(b), 0);
            if (b == 12)
                break;
        }
        usleep(1000);

        sent_bytes = send(storage_server_sockets[1], &backups[2], sizeof(backups[2]), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        // backup sent to 0 and 1 of 2
        usleep(10000);

        sent_bytes = send(storage_server_sockets[2], command, strlen(command), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        while (1)
        {
            recv(storage_server_sockets[2], &b, sizeof(b), 0);
            if (b == 12)
                break;
        }
        usleep(1000);
        sent_bytes = send(storage_server_sockets[2], &backups[1], sizeof(backups[1]), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }

        usleep(1000);
        sent_bytes = send(storage_server_sockets[0], command, strlen(command), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        while (1)
        {
            recv(storage_server_sockets[0], &b, sizeof(b), 0);
            if (b == 12)
                break;
        }
        usleep(1000);
        sent_bytes = send(storage_server_sockets[0], &backups[1], sizeof(backups[1]), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }

        // backup sent to 0 and 2 of 1
        usleep(1000);

        sent_bytes = send(storage_server_sockets[1], command, strlen(command), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        while (1)
        {
            recv(storage_server_sockets[1], &b, sizeof(b), 0);
            if (b == 12)
                break;
        }
        usleep(1000);
        sent_bytes = send(storage_server_sockets[1], &backups[0], sizeof(backups[0]), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        usleep(1000);

        sent_bytes = send(storage_server_sockets[2], command, strlen(command), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        while (1)
        {
            recv(storage_server_sockets[2], &b, sizeof(b), 0);
            if (b == 12)
                break;
        }
        usleep(1000);
        sent_bytes = send(storage_server_sockets[2], &backups[0], sizeof(backups[0]), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        // backup sent to 1 and 2 of 0
    }
    else if (storage_server_count > 3)
    {
        char command[1024];
        strcpy(command, "RECIEVE ./BACKUP");

        int sent_bytes = send(storage_server_sockets[storage_server_count - 3], command, strlen(command), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        int b;
        while (1)
        {
            recv(storage_server_sockets[storage_server_count - 3], &b, sizeof(b), 0);
            if (b == 12)
                break;
        }
        usleep(1000);
        sent_bytes = send(storage_server_sockets[storage_server_count - 3], &backups[storage_server_count - 1], sizeof(backups[storage_server_count - 1]), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        usleep(1000);

        sent_bytes = send(storage_server_sockets[storage_server_count - 2], command, strlen(command), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        while (1)
        {
            recv(storage_server_sockets[storage_server_count - 2], &b, sizeof(b), 0);
            if (b == 12)
                break;
        }
        usleep(1000);
        sent_bytes = send(storage_server_sockets[storage_server_count - 2], &backups[storage_server_count - 1], sizeof(backups[storage_server_count - 1]), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        // backup sent to i-2 and i-1
    }
}

void *ss_thread_function(void *arg)
{
    int nm_server_socket, nm_server_socket2, client_soc;
    struct sockaddr_in nm_server_addr, nm_server_addr2, client_addr;
    socklen_t client_len = sizeof(client_addr);

    nm_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (nm_server_socket < 0)
    {
        perror("Error in storage_server socket creation");
        exit(1);
    }

    nm_server_socket2 = socket(AF_INET, SOCK_STREAM, 0);
    if (nm_server_socket2 < 0)
    {
        perror("Error in storage_server socket creation");
        exit(1);
    }
    nm_server_addr.sin_family = AF_INET;
    nm_server_addr.sin_addr.s_addr = INADDR_ANY;
    nm_server_addr.sin_port = htons(NM_PORT_FOR_SERVER);

    nm_server_addr2.sin_family = AF_INET;
    nm_server_addr2.sin_addr.s_addr = INADDR_ANY;
    nm_server_addr2.sin_port = htons(NM_PORT_FOR_SERVER2);

    if (bind(nm_server_socket, (struct sockaddr *)&nm_server_addr, sizeof(nm_server_addr)) == -1)
    {
        perror("NM Socket bind failed for SS");
        close(nm_server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(nm_server_socket, 5) == -1)
    {
        perror("NM Socket listen failed for SS");
        close(nm_server_socket);
        exit(EXIT_FAILURE);
    }

    if (bind(nm_server_socket2, (struct sockaddr *)&nm_server_addr2, sizeof(nm_server_addr2)) == -1)
    {
        perror("NM Socket bind failed for SS");
        close(nm_server_socket2);
        exit(EXIT_FAILURE);
    }

    if (listen(nm_server_socket2, 5) == -1)
    {
        perror("NM Socket listen failed for SS");
        close(nm_server_socket2);
        exit(EXIT_FAILURE);
    }

    printf("NM Server listening for Storage Servers on ports %d and %d...\n", NM_PORT_FOR_SERVER, NM_PORT_FOR_SERVER2);

    while (1)
    {
        ss_is_down[storage_server_count] = 0;
        storage_server_sockets[storage_server_count] = accept(nm_server_socket, (struct sockaddr *)&client_addr, &client_len);

        client_soc = accept(nm_server_socket2, (struct sockaddr *)&client_addr, &client_len);
        if (storage_server_sockets[storage_server_count] == -1)
        {
            perror("Error accepting SS connection");
            continue;
        }

        printf("Storage server %d connected: %s::%d\n", (storage_server_count + 1), inet_ntoa(nm_server_addr.sin_addr), ntohs(nm_server_addr.sin_port));

        ThreadArgs *myArgs;

        // Allocate memory for the arguments
        myArgs = (ThreadArgs *)malloc(sizeof(ThreadArgs));

        // Set values for the arguments
        myArgs->arg1 = client_soc;
        myArgs->arg2 = storage_server_count;

        pthread_t down;
        if (pthread_create(&down, NULL, handle_downing_of_server, (void *)myArgs) != 0)
        {
            perror("Error creating Naming Server thread");
            exit(1);
        }

        handle_nm_ss_comm(storage_server_sockets[storage_server_count]);
    }
    close(nm_server_socket);
    pthread_exit(NULL);
}

void *handle_nm_client_comm(void *arg)
{
    int socket = *((int *)arg);

    char client_request[1024];
    char ack[] = "ACK";
    send(socket, ack, strlen(ack), 0);

    int bytes_received = recv(socket, &client_request, sizeof(client_request), 0);
    if (bytes_received <= 0)
    {
        perror("Error receiving data from client server");
        exit(1);
    }

    FILE *file1 = fopen("record.txt", "a");
    fprintf(file1, "request recieved from client : ");
    fprintf(file1, "%s ", client_request);
    fprintf(file1, "PORT:%d IP:%s\n", NM_PORT_FOR_CLIENT, NAMING_SERVER_IP);
    fclose(file1);
    char copy[1024];
    strcpy(copy, client_request);
    char *t = strtok(client_request, " \n");
    if (strcmp(t, "READ") == 0 || strcmp(t, "RETRIEVE") == 0)
    {
        t = strtok(NULL, " \n");

        int ss;
        unsigned k = hashing(t);
        ReferencePage(q, hash, k);
        if (ip5 != -1)
        {
            yh(q, hash, k);
            ss = ip5;
            printf("Cache Hit\n");
            ip5 = -1;
        }

        else
        {
            ss = Search(T, t);
            ip5 = -1;
            printf("Cache Miss\n");
            Enqueue(q, hash, k, ss);
        }
        // printf("   %d    \n", ss);
        if (ss < 0)
        {
            perror("File not found");
        }
        struct Info_for_Client Information;
        if (ss_is_down[ss] == 1)
        {
            if (ss == 0)
            {
                if (ss_is_down[1] != 1)
                {
                    strcpy(Information.ip_address, storage_servers[1].ip_address);
                    Information.port = storage_servers[1].client_connection_port;
                }
                else if (ss_is_down[2] != 1)
                {
                    strcpy(Information.ip_address, storage_servers[2].ip_address);
                    Information.port = storage_servers[2].client_connection_port;
                }
            }
            else if (ss == 1)
            {
                if (ss_is_down[0] != 1)
                {
                    strcpy(Information.ip_address, storage_servers[0].ip_address);
                    Information.port = storage_servers[0].client_connection_port;
                }
                else if (ss_is_down[2] != 1)
                {
                    strcpy(Information.ip_address, storage_servers[2].ip_address);
                    Information.port = storage_servers[2].client_connection_port;
                }
            }
            else
            {
                if (ss_is_down[ss - 1] != 1)
                {
                    strcpy(Information.ip_address, storage_servers[ss - 1].ip_address);
                    Information.port = storage_servers[ss - 1].client_connection_port;
                }
                else if (ss_is_down[ss - 2] != 1)
                {
                    strcpy(Information.ip_address, storage_servers[ss - 2].ip_address);
                    Information.port = storage_servers[ss - 2].client_connection_port;
                }
            }
            Information.flag_for_backup = 1;
        }
        else
        {
            // printf("yahan pachun gye\n");
            // printf("   %d    \n", ss);
            strcpy(Information.ip_address, storage_servers[ss].ip_address);
            Information.port = storage_servers[ss].client_connection_port;
            Information.flag_for_backup = 0;
        }

        int sent_bytes = send(socket, &Information, sizeof(Information), 0);
        if (sent_bytes < 0)
        {
            perror("Send failed");
        }
        FILE *file1 = fopen("record.txt", "a");
        fprintf(file1, "%s", "Sending IP and PORT OF Storage Server");
        fprintf(file1, "%s", Information.ip_address);
        fprintf(file1, "%d\n", Information.port);
        fclose(file1);

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        {
            perror("setsockopt for receive timeout");
            close(storage_server_sockets[ss]);
        }

        // Example: Attempt to receive data with a timeout
        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = recv(socket, buffer, sizeof(buffer), 0);

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

            printf("Received acknowledgment from client operation successful: %s\n", buffer);
            FILE *file2 = fopen("record.txt", "a");
            fprintf(file2, "Received acknowledgment from client operation successful\n");
            fclose(file2);
        }
    }
    else if (strcmp(t, "WRITE") == 0)
    {
        t = strtok(NULL, " \n");
        int ss;
        unsigned k = hashing(t);
        ReferencePage(q, hash, k);
        if (ip5 != -1)
        {
            yh(q, hash, k);
            ss = ip5;
            printf("Cache Hit\n");
            ip5 = -1;
        }

        else
        {
            ss = Search(T, t);
            ip5 = -1;
            printf("Cache Miss\n");
            Enqueue(q, hash, k, ss);
        }
        if (ss < 0)
        {
            perror("File not found");
        }
        struct sum Information;
        strcpy(Information.ip1, storage_servers[ss].ip_address);
        Information.port1 = storage_servers[ss].client_connection_port;
        Information.port2 = -1;
        Information.port3 = -1;
        if (storage_server_count >= 3)
        {
            if (ss == 0)
            {
                strcpy(Information.ip2, storage_servers[1].ip_address);
                Information.port2 = storage_servers[1].client_connection_port;

                strcpy(Information.ip3, storage_servers[2].ip_address);
                Information.port3 = storage_servers[2].client_connection_port;
            }
            else if (ss == 1)
            {
                strcpy(Information.ip2, storage_servers[0].ip_address);
                Information.port2 = storage_servers[0].client_connection_port;

                strcpy(Information.ip3, storage_servers[2].ip_address);
                Information.port3 = storage_servers[2].client_connection_port;
            }
            else
            {
                strcpy(Information.ip2, storage_servers[ss - 1].ip_address);
                Information.port2 = storage_servers[ss - 1].client_connection_port;

                strcpy(Information.ip3, storage_servers[ss - 2].ip_address);
                Information.port3 = storage_servers[ss - 2].client_connection_port;
            }
        }

        int sent_bytes = send(socket, &Information, sizeof(Information), 0);
        if (sent_bytes < 0)
        {
            perror("Send failed");
        }

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        {
            perror("setsockopt for receive timeout");
            close(storage_server_sockets[ss]);
        }

        // Example: Attempt to receive data with a timeout
        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = recv(socket, buffer, sizeof(buffer), 0);

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
            printf("Received acknowledgment from client operation successful: %s\n", buffer);
            FILE *file2 = fopen("record.txt", "a");
            fprintf(file2, "Received acknowledgment from client operation successful\n");
            fclose(file2);
        }
    }
    else if (strcmp(t, "CREATE") == 0)
    {
        t = strtok(NULL, " \n");
        if (strcmp(t, "FILE") == 0)
        {
            t = strtok(NULL, " \n");

            // printf("%s\n", t);
            int ss;

            char path[] = "";
            strcpy(path, t);
            char *lastSlash = strrchr(path, '/');

            if (lastSlash != NULL)
            {
                // Tokenize the string using the last '/'
                *lastSlash = '\0'; // Replace the last '/' with '\0' to terminate the first part of the string
                char *parent_directory = path;
                char *child = lastSlash + 1;

                // Print the tokenized parts
                // printf("Directory: %s\n", parent_directory);
                // printf("Filename: %s\n", child);

                int ss;
                unsigned k = hashing(t);
                ss = Search(T, parent_directory);
                if (ss < 0)
                {
                    perror("File not found");
                }
                ip5 = -1;
                Enqueue(q, hash, k, ss);
                T = Insert(T, t, ss);

                char command_to_be_sent_to_ss[1024];
                strcpy(command_to_be_sent_to_ss, "CREATE FILE ");
                strcat(command_to_be_sent_to_ss, child);
                strcat(command_to_be_sent_to_ss, " IN ");
                strcat(command_to_be_sent_to_ss, parent_directory);

                // printf("%d      %s\n", ss, command_to_be_sent_to_ss);

                int sent_bytes = send(storage_server_sockets[ss], command_to_be_sent_to_ss, strlen(command_to_be_sent_to_ss), 0);
                if (sent_bytes < 0)
                {
                    perror("Send failed");
                }
                if (storage_server_count >= 3)
                {
                    char command_to_be_sent_to_backups[1024];
                    strcpy(command_to_be_sent_to_backups, "CREATE FILE ");
                    strcat(command_to_be_sent_to_backups, child);
                    strcat(command_to_be_sent_to_backups, " IN ./BACKUP");
                    strcat(command_to_be_sent_to_backups, parent_directory + 1);
                    // printf("%s\n", command_to_be_sent_to_backups);

                    if (ss == 0)
                    {
                        sent_bytes = send(storage_server_sockets[1], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                        usleep(1000);
                        sent_bytes = send(storage_server_sockets[2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                    }
                    else if (ss == 1)
                    {
                        sent_bytes = send(storage_server_sockets[0], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                        usleep(1000);
                        sent_bytes = send(storage_server_sockets[2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                    }
                    else
                    {
                        sent_bytes = send(storage_server_sockets[ss - 1], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                        usleep(1000);
                        sent_bytes = send(storage_server_sockets[ss - 2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                    }
                }

                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                if (setsockopt(storage_server_sockets[ss], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
                {
                    perror("setsockopt for receive timeout");
                    close(storage_server_sockets[ss]);
                }

                // Example: Attempt to receive data with a timeout
                char buffer[1024];
                ssize_t bytesRead;

                bytesRead = recv(storage_server_sockets[ss], buffer, sizeof(buffer), 0);

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

                    printf("Received acknowledgment from storage server operation successful: %s\n", buffer);
                    FILE *file2 = fopen("record.txt", "a");
                    fprintf(file2, "Received acknowledgment from storage server operation successful\n");
                    fclose(file2);
                    char ack[] = "ACK";
                    // sleep(2);
                    send(socket, ack, strlen(ack), 0);
                }
            }
            else
            {
                // No '/' found in the string
                printf("Invalid path\n");
            }
        }
        else if (strcmp(t, "DIR") == 0)
        {
            t = strtok(NULL, " \n");

            // printf("%s\n", t);
            int ss;

            char path[] = "";
            strcpy(path, t);
            char *lastSlash = strrchr(path, '/');

            if (lastSlash != NULL)
            {
                // Tokenize the string using the last '/'
                *lastSlash = '\0'; // Replace the last '/' with '\0' to terminate the first part of the string
                char *parent_directory = path;
                char *child = lastSlash + 1;

                // Print the tokenized parts
                // printf("Directory: %s\n", parent_directory);
                // printf("Filename: %s\n", child);

                int ss;
                unsigned k = hashing(t);
                ss = Search(T, parent_directory);
                if (ss < 0)
                {
                    perror("File not found");
                }
                ip5 = -1;
                Enqueue(q, hash, k, ss);
                T = Insert(T, t, ss);

                char command_to_be_sent_to_ss[1024];
                strcpy(command_to_be_sent_to_ss, "CREATE DIR ");
                strcat(command_to_be_sent_to_ss, child);
                strcat(command_to_be_sent_to_ss, " IN ");
                strcat(command_to_be_sent_to_ss, parent_directory);

                // printf("%d     %s\n", ss, command_to_be_sent_to_ss);

                int sent_bytes = send(storage_server_sockets[ss], command_to_be_sent_to_ss, strlen(command_to_be_sent_to_ss), 0);
                if (sent_bytes < 0)
                {
                    perror("Send failed");
                }

                if (storage_server_count >= 3)
                {
                    char command_to_be_sent_to_backups[1024];
                    strcpy(command_to_be_sent_to_backups, "CREATE DIR ");
                    strcat(command_to_be_sent_to_backups, child);
                    strcat(command_to_be_sent_to_backups, " IN ./BACKUP");
                    strcat(command_to_be_sent_to_backups, parent_directory + 1);
                    // printf("%s\n", command_to_be_sent_to_backups);
                    if (ss == 0)
                    {
                        sent_bytes = send(storage_server_sockets[1], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                        usleep(1000);
                        sent_bytes = send(storage_server_sockets[2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                    }
                    else if (ss == 1)
                    {
                        sent_bytes = send(storage_server_sockets[0], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                        usleep(1000);
                        sent_bytes = send(storage_server_sockets[2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                    }
                    else
                    {
                        sent_bytes = send(storage_server_sockets[ss - 1], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                        usleep(1000);
                        sent_bytes = send(storage_server_sockets[ss - 2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                    }
                }

                struct timeval timeout;
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;

                if (setsockopt(storage_server_sockets[ss], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
                {
                    perror("setsockopt for receive timeout");
                    close(storage_server_sockets[ss]);
                }

                // Example: Attempt to receive data with a timeout
                char buffer[1024];
                ssize_t bytesRead;

                bytesRead = recv(storage_server_sockets[ss], buffer, sizeof(buffer), 0);

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

                    printf("Received acknowledgment from storage server operation successful: %s\n", buffer);
                    FILE *file2 = fopen("record.txt", "a");
                    fprintf(file2, "Received acknowledgment from storage server operation successful\n");
                    fclose(file2);
                    char ack[] = "ACK";
                    // sleep(2);
                    send(socket, ack, strlen(ack), 0);
                }
            }
            else
            {
                // No '/' found in the string
                printf("Invalid path\n");
            }
        }
    }
    else if (strcmp(t, "DELETE") == 0)
    {
        t = strtok(NULL, " \n");
        int ss;
        unsigned k = hashing(t);
        ReferencePage(q, hash, k);
        if (hash->array[k] != NULL)
        {
            ss = ip5;
            deQueue1(q, k);
            printf("CACHE HIT\n");
            ip5 = -1;
            hash->array[k] = NULL;
        }
        else
        {
            printf("CACHE MISS\n");
            ss = Search(T, t);
            // printf("%d", ss);
            ip5 = -1;
        }
        if (ss < 0)
        {
            perror("File not found");
        }

        Delete(T, t);
        char command_to_be_sent_to_ss[1024];
        strcpy(command_to_be_sent_to_ss, "PRIV_");
        strcat(command_to_be_sent_to_ss, copy);

        // printf("%d %s\n", ss, command_to_be_sent_to_ss);
        // printf("%d\n", storage_server_sockets[ss]);

        int sent_bytes = send(storage_server_sockets[ss], command_to_be_sent_to_ss, strlen(command_to_be_sent_to_ss), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        else
        {
            FILE *file = fopen("record.txt", "a");
            fprintf(file, "%s", "Delete Command sent to SS");
            fprintf(file, "%s", command_to_be_sent_to_ss);
            fclose(file);
            if (storage_server_count >= 3)
            {
                char command_to_be_sent_to_backups[1024];
                strcpy(command_to_be_sent_to_backups, "PRIV_DELETE ./BACKUP");
                strcat(command_to_be_sent_to_backups, t + 1);
                // printf("%s\n", command_to_be_sent_to_backups);
                if (ss == 0)
                {
                    sent_bytes = send(storage_server_sockets[1], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                    usleep(1000);
                    sent_bytes = send(storage_server_sockets[2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                }
                else if (ss == 1)
                {
                    sent_bytes = send(storage_server_sockets[0], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                    usleep(1000);
                    sent_bytes = send(storage_server_sockets[2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                }
                else
                {
                    sent_bytes = send(storage_server_sockets[ss - 1], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                    usleep(1000);
                    sent_bytes = send(storage_server_sockets[ss - 2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                }
            }

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            if (setsockopt(storage_server_sockets[ss], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
            {
                perror("setsockopt for receive timeout");
                close(storage_server_sockets[ss]);
            }

            // Example: Attempt to receive data with a timeout
            char buffer[1024];
            ssize_t bytesRead;

            bytesRead = recv(storage_server_sockets[ss], buffer, sizeof(buffer), 0);

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

                printf("Received acknowledgment from storage server operation successful: %s\n", buffer);
                FILE *file2 = fopen("record.txt", "a");
                fprintf(file2, "Received acknowledgment from storage server operation successful\n");
                fclose(file2);
                char ack[] = "ACK";

                send(socket, ack, strlen(ack), 0);
            }
        }
    }
    else if (strcmp(t, "COPY") == 0)
    {
        t = strtok(NULL, " \n");
        char copy1[1024];
        strcpy(copy1, t);
        int ss1;
        int ss2;

        // printf("start00\n");
        unsigned k = hashing(t);
        ReferencePage(q, hash, k);
        // printf("start01\n");
        if (ip5 != -1)
        {
            yh(q, hash, k);
            ss1 = ip5;
            printf("Cache Hit\n");
            ip5 = -1;
        }

        else
        {
            ss1 = Search(T, t);
            ip5 = -1;
            printf("Cache Miss\n");
            Enqueue(q, hash, k, ss1);
        }
        if (ss1 < 0)
        {
            perror("File not found");
        }

        t = strtok(NULL, " \n");
        k = hashing(t);
        ReferencePage(q, hash, k);
        if (ip5 != -1)
        {
            yh(q, hash, k);
            ss2 = ip5;
            printf("Cache Hit\n");
            ip5 = -1;
        }
        else
        {
            ss2 = Search(T, t);
            ip5 = -1;
            printf("Cache Miss\n");
            Enqueue(q, hash, k, ss2);
        }
        if (ss2 < 0)
        {
            perror("File not found");
        }

        // printf("start         %d         %d\n", ss1, ss2);
        char command_to_be_sent_to_ss1[1024];
        strcpy(command_to_be_sent_to_ss1, "SEND ");
        strcat(command_to_be_sent_to_ss1, copy1);

        int sent_bytes = send(storage_server_sockets[ss1], command_to_be_sent_to_ss1, strlen(command_to_be_sent_to_ss1), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }

        struct for_copy to_be_sent_again;
        while (1)
        {
            if (recv(storage_server_sockets[ss1], &to_be_sent_again, sizeof(to_be_sent_again), 0) == -1)
            {
                perror("Receive failed");
                exit(EXIT_FAILURE);
            }

            if (to_be_sent_again.to_be_sent[0][0] != '\0')
                break;
        }
        int j = 0;
        while (to_be_sent_again.to_be_sent[j][0] != '\0')
        {
            printf("%s\n", to_be_sent_again.to_be_sent[j]);
            j++;
        }

        char command_to_be_sent_to_ss2[1024];
        strcpy(command_to_be_sent_to_ss2, "RECIEVE ");
        strcat(command_to_be_sent_to_ss2, t);

        sent_bytes = send(storage_server_sockets[ss2], command_to_be_sent_to_ss2, strlen(command_to_be_sent_to_ss2), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        int b;
        while (1)
        {
            recv(storage_server_sockets[ss2], &b, sizeof(b), 0);
            if (b == 12)
                break;
        }
        // printf("ho gaya copy!\n");
        usleep(100);
        sent_bytes = send(storage_server_sockets[ss2], &to_be_sent_again, sizeof(to_be_sent_again), 0);
        if (sent_bytes <= 0)
        {
            perror("Send failed");
        }
        if (storage_server_count >= 3)
        {
            char command_to_be_sent_to_backups[1024];
            strcpy(command_to_be_sent_to_backups, "RECIEVE ./BACKUP");
            strcat(command_to_be_sent_to_backups, t + 1);
            // printf("%s\n", command_to_be_sent_to_backups);
            if (ss2 == 0)
            {
                sent_bytes = send(storage_server_sockets[1], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                while (1)
                {
                    recv(storage_server_sockets[1], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                sent_bytes = send(storage_server_sockets[1], &to_be_sent_again, sizeof(to_be_sent_again), 0);
                usleep(1000);
                sent_bytes = send(storage_server_sockets[2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                while (1)
                {
                    recv(storage_server_sockets[2], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                sent_bytes = send(storage_server_sockets[2], &to_be_sent_again, sizeof(to_be_sent_again), 0);
            }
            else if (ss2 == 1)
            {
                sent_bytes = send(storage_server_sockets[0], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                while (1)
                {
                    recv(storage_server_sockets[0], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                sent_bytes = send(storage_server_sockets[0], &to_be_sent_again, sizeof(to_be_sent_again), 0);
                usleep(1000);
                sent_bytes = send(storage_server_sockets[2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                while (1)
                {
                    recv(storage_server_sockets[2], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                sent_bytes = send(storage_server_sockets[2], &to_be_sent_again, sizeof(to_be_sent_again), 0);
            }
            else
            {
                sent_bytes = send(storage_server_sockets[ss2 - 1], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                while (1)
                {
                    recv(storage_server_sockets[ss2 - 1], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                sent_bytes = send(storage_server_sockets[ss2 - 1], &to_be_sent_again, sizeof(to_be_sent_again), 0);
                usleep(1000);
                sent_bytes = send(storage_server_sockets[ss2 - 2], command_to_be_sent_to_backups, strlen(command_to_be_sent_to_backups), 0);
                while (1)
                {
                    recv(storage_server_sockets[ss2 - 2], &b, sizeof(b), 0);
                    if (b == 12)
                        break;
                }
                usleep(100);
                sent_bytes = send(storage_server_sockets[ss2 - 2], &to_be_sent_again, sizeof(to_be_sent_again), 0);
            }
        }

        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        if (setsockopt(storage_server_sockets[ss2], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        {
            perror("setsockopt for receive timeout");
            close(storage_server_sockets[ss2]);
        }

        // Example: Attempt to receive data with a timeout
        char buffer[1024];
        ssize_t bytesRead;

        bytesRead = recv(storage_server_sockets[ss2], buffer, sizeof(buffer), 0);

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

            printf("Received acknowledgment from storage server operation successful: %s\n", buffer);
            FILE *file2 = fopen("record.txt", "a");
            fprintf(file2, "Received acknowledgment from storage server operation successful\n");
            fclose(file2);
            char ack[] = "ACK";
            // sleep(2);
            send(socket, ack, strlen(ack), 0);
        }
    }
}

void *client_thread_function(void *arg)
{
    int nm_server_socket;
    struct sockaddr_in nm_server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    nm_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (nm_server_socket < 0)
    {
        perror("Error in client socket creation");
        exit(1);
    }

    nm_server_addr.sin_family = AF_INET;
    nm_server_addr.sin_addr.s_addr = INADDR_ANY;
    nm_server_addr.sin_port = htons(NM_PORT_FOR_CLIENT);

    if (bind(nm_server_socket, (struct sockaddr *)&nm_server_addr, sizeof(nm_server_addr)) == -1)
    {
        perror("NM Socket bind failed for Client");
        close(nm_server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(nm_server_socket, 5) == -1)
    {
        perror("NM Socket listen failed for Client");
        close(nm_server_socket);
        exit(EXIT_FAILURE);
    }

    printf("NM Server listening for Clients on port %d...\n", NM_PORT_FOR_CLIENT);

    while (1)
    {
        client_sockets[client_count] = accept(nm_server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_sockets[client_count] == -1)
        {
            perror("Error accepting client connection");
            continue;
        }

        printf("Client %d connected: %s::%d\n", (client_count + 1), inet_ntoa(nm_server_addr.sin_addr), ntohs(nm_server_addr.sin_port));

        pthread_t client_request_thread;
        if (pthread_create(&client_request_thread, NULL, handle_nm_client_comm, &client_sockets[client_count]) != 0)
        {
            perror("Error creating Client thread");
            exit(1);
        }

        client_count++;
    }
    close(nm_server_socket);
    pthread_exit(NULL);
}

int main()
{
    q = createQueue(4);
    // Let 10 different pages can be requested (pages to be
    // referenced are numbered from 0 to 9
    hash = createHash(1000000);
    // Open a file for writing (creates the file if it doesn't exist, truncates it to zero length otherwise)
    file = fopen("record.txt", "a");

    if (file == NULL)
    {
        fprintf(stderr, "Could not open the file for writing.\n");
        return 1; // Return an error code
    }
    fclose(file);

    T = NULL;

    pthread_t ss_thread;
    if (pthread_create(&ss_thread, NULL, ss_thread_function, NULL) != 0)
    {
        perror("Error creating Storage Server thread");
        exit(1);
    }

    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, client_thread_function, NULL) != 0)
    {
        perror("Error creating Client thread");
        exit(1);
    }

    if (pthread_join(ss_thread, NULL) != 0)
    {
        perror("Error joining Storage Server thread");
        exit(1);
    }
    if (pthread_join(client_thread, NULL) != 0)
    {
        perror("Error joining client thread");
        exit(1);
    }

    return 0;
}
