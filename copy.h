#ifndef __COPY_H
#define __COPY_H
void copys(char** command);
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#define NM_SERVER_IP "127.0.0.1"
#define NM_PORT 12347
#define BUFFER_SIZE 1024
#endif