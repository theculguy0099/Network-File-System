#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <ftw.h>


#define _XOPEN_SOURCE 500
#define FTW_DEPTH  10
#define FTW_PHYS   20
#define NM_PORT 12345
#define NM_PORT2 12346
#define BUFFER_SIZE 1024
#define NAMING_SERVER_IP "127.0.0.1"

#define NO_OF_PATHS 100
#define MAX_SS 100
#define MAX_CLIENTS 100

struct Info_for_Client
{
    char ip_address[16]; // Assuming IPv4 address
    int port;
};

struct ServerInfo
{
    char ip_address[16]; // Assuming IPv4 address
    int nm_connection_port;
    int client_connection_port;
    int no_of_paths;
    char accessible_path[NO_OF_PATHS][BUFFER_SIZE];
};

struct for_copy
{
    char to_be_sent[100][1024];
};
extern int client_port;
extern int index1;
extern int index2;