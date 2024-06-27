#ifndef __READ_H
#define __READ_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
void reads(char** command);

#define NM_SERVER_IP "127.0.0.1"
#define NM_PORT 12347
#define BUFFER_SIZE 1024
#endif