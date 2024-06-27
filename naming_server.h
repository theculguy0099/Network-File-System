#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <fcntl.h>
#define NM_PORT_FOR_SERVER 12345
#define NM_PORT_FOR_SERVER2 12346
#define NM_PORT_FOR_CLIENT 12347
#define BUFFER_SIZE 1024
#define NAMING_SERVER_IP "127.0.0.1"
#define NO_OF_PATHS 100
#define MAX_SS 100
#define MAX_CLIENTS 100

struct Info_for_Client
{
    char ip_address[16]; // Assuming IPv4 address
    int port;
    int flag_for_backup;
};

struct ServerInfo
{
    char ip_address[16]; // Assuming IPv4 address
    int nm_connection_port;
    int client_connection_port;
    int no_of_paths;
    char accessible_path[NO_OF_PATHS][BUFFER_SIZE];
};

typedef struct QNode
{
    struct QNode *prev, *next;
    unsigned pageNumber;
    int ip; // the page number stored in this QNode
} QNode;

// A Queue (A FIFO collection of Queue Nodes)
typedef struct Queue
{
    unsigned count;          // Number of filled frames
    unsigned numberOfFrames; // total number of frames
    QNode *front, *rear;
} Queue;
typedef struct {
    int arg1;
    char arg2;
} ThreadArgs;

// A hash (Collection of pointers to Queue Nodes)
typedef struct Hash
{
    int capacity;  // how many pages can be there
    QNode **array; // an array of queue nodes
} Hash;
struct sum
{
    char ip1[16]; // Assuming IPv4 address
    int port1;
    char ip2[16]; // Assuming IPv4 address
    int port2;
    char ip3[16]; // Assuming IPv4 address
    int port3;
};

struct for_copy
{
    char to_be_sent[100][1024];
};

typedef struct trienode *PtrTrie;
typedef PtrTrie Trie;
typedef struct trienode trienode;
struct trienode
{
    PtrTrie Children[66];
    int end_flag;
    int ss_id;
};
struct recently_accessed
{
    char path[BUFFER_SIZE];
    int ss;
};

extern struct recently_accessed last;
extern int storage_server_count;
extern int client_count;
extern int storage_server_sockets[MAX_SS];
extern int ss_is_down[MAX_SS];
extern int client_sockets[MAX_CLIENTS];
extern struct ServerInfo storage_servers[MAX_SS];
extern struct for_copy backups[MAX_SS];

int value_of_index(char c);
PtrTrie MakeNode();
Trie Insert(Trie T, char *str, int ss_id);
int Search(Trie T, char *str);
void Delete(Trie T, char *str);

void handle_nm_ss_comm(int socket);
void *handle_nm_client_comm(void *arg);
void *ss_thread_function(void *arg);
void *client_thread_function(void *arg);
