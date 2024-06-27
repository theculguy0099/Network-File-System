#ifndef __RETRIEVE_H
#define __RETRIEVE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
void retrieves(char** );

#define NM_SERVER_IP "127.0.0.1"
#define NM_PORT 12347
#define BUFFER_SIZE 1024
#endif