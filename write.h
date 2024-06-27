#ifndef __WRITE_H
#define __WRITE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
void writes(char** );

#define NM_SERVER_IP "127.0.0.1"
#define NM_PORT 12347
#define BUFFER_SIZE 1024
#endif