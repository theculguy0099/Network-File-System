#ifndef __CLIENT_H_
#define __CLIENT_H_
#define MAX_TOKENS 32
#define MAX_TOKEN_LENGTH 256
#define MAX_TOKENS 32
#define MAX_TOKEN_LENGTH 256
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
void reads(char **command);

#endif