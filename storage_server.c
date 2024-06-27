#include "storage_server.h"
int client_port;
struct ServerInfo server_info;
int index1 = 0;
int index2 = 0;
int index3 = 0;
struct for_copy backup;
struct for_copy ss_recovery;
char backup_send[100][1024];

void replaceSubstring(char *str, const char *oldSubstr, const char *newSubstr)
{
    char *pos = strstr(str, oldSubstr);
    while (pos != NULL)
    {
        size_t prefixLength = pos - str;
        size_t suffixLength = strlen(pos + strlen(oldSubstr));

        char *temp = malloc(prefixLength + strlen(newSubstr) + suffixLength + 1);
        if (temp == NULL)
        {
            perror("Memory allocation error");
            return;
        }
        strncpy(temp, str, prefixLength);
        strcpy(temp + prefixLength, newSubstr);
        strcpy(temp + prefixLength + strlen(newSubstr), pos + strlen(oldSubstr));
        temp[prefixLength + strlen(newSubstr) + suffixLength] = '\0';
        strcpy(str, temp);
        free(temp);

        pos = strstr(str, oldSubstr);
    }
}

void handle_send_requests(char *path, char *original, int depth, char **to_be_send)
{
    DIR *dir;
    struct dirent *entry;
    dir = opendir(path);
    if (dir == NULL)
    {
        strcpy(to_be_send[index1], "FILE ");
        char new_path[1024];
        strcpy(new_path, path);

        replaceSubstring(path, original, ".");
        strcat(to_be_send[index1], path);

        FILE *file = fopen(new_path, "r");
        if (file != NULL)
        {
            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            char *content = (char *)malloc(fileSize + 1); // +1 for null-terminator
            if (content == NULL)
            {
                perror("Error allocating memory");
                fclose(file);
            }

            // Read the file content into the array
            size_t bytesRead = fread(content, 1, fileSize, file);
            if (bytesRead != fileSize)
            {
                perror("Error reading file");
                free(content);
                fclose(file);
            }

            // Null-terminate the content
            content[fileSize] = '\0';

            // Close the file
            fclose(file);

            strcat(to_be_send[index1], " $");
            strcat(to_be_send[index1++], content);

            // Don't forget to free the allocated memory
            free(content);
        }
        else
        {
            perror("Not a valid accessible path");
            exit(1);
        }
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            if (entry->d_type == DT_DIR)
            {
                char newPath[BUFFER_SIZE];
                char newPath_copy[BUFFER_SIZE];
                snprintf(newPath, sizeof(newPath), "%s/%s", path, entry->d_name);
                strcpy(newPath_copy, newPath);
                replaceSubstring(newPath, original, ".");
                strcpy(to_be_send[index1], "DIR ");
                strncat(to_be_send[index1++], newPath, sizeof(newPath));

                handle_send_requests(newPath_copy, original, depth + 1, to_be_send);
            }
            else
            {
                // Build the path for the file
                char filePath[BUFFER_SIZE];
                char filePath_copy[BUFFER_SIZE];
                snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);
                strcpy(filePath_copy, filePath);
                // Store the relative path in the accessible_path array
                replaceSubstring(filePath, original, ".");
                strcpy(to_be_send[index1], "FILE ");
                strncat(to_be_send[index1], filePath, sizeof(filePath));

                // Read the file
                FILE *file = fopen(filePath_copy, "r");
                if (file != NULL)
                {
                    fseek(file, 0, SEEK_END);
                    long fileSize = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    // Allocate memory to store the file content
                    char *content = (char *)malloc(fileSize + 1); // +1 for null-terminator
                    if (content == NULL)
                    {
                        perror("Error allocating memory");
                        fclose(file);
                    }

                    // Read the file content into the array
                    size_t bytesRead = fread(content, 1, fileSize, file);
                    if (bytesRead != fileSize)
                    {
                        perror("Error reading file");
                        free(content);
                        fclose(file);
                    }

                    // Null-terminate the content
                    content[fileSize] = '\0';

                    // Close the file
                    fclose(file);

                    strcat(to_be_send[index1], " $");
                    strcat(to_be_send[index1++], content);
                    // Don't forget to free the allocated memory
                    free(content);
                }
            }
        }
    }

    closedir(dir);
}

void listFiles(char *path, char *original, int depth)
{
    DIR *dir;
    struct dirent *entry;
    dir = opendir(path);
    if (dir == NULL)
    {
        // printf("%s\n", path);
        char new_file_path[BUFFER_SIZE];

        strcpy(new_file_path, path);
        replaceSubstring(path, original, ".");
        strcpy(backup_send[index2], "FILE ");
        strcat(backup_send[index2], path);

        FILE *file = fopen(new_file_path, "r");
        if (file != NULL)
        {
            fseek(file, 0, SEEK_END);
            long fileSize = ftell(file);
            fseek(file, 0, SEEK_SET);

            char *content = (char *)malloc(fileSize + 1); // +1 for null-terminator
            if (content == NULL)
            {
                perror("Error allocating memory");
                fclose(file);
            }

            // Read the file content into the array
            size_t bytesRead = fread(content, 1, fileSize, file);
            if (bytesRead != fileSize)
            {
                perror("Error reading file");
                free(content);
                fclose(file);
            }

            // Null-terminate the content
            content[fileSize] = '\0';

            // Close the file
            fclose(file);

            strcat(backup_send[index2], " $");
            strcat(backup_send[index2++], content);
            // Now 'content' contains the content of the file
            // printf("File content:\n%s\n", content);

            // Don't forget to free the allocated memory
            free(content);
        }
        else
        {
            perror("Not a valid accessible path");
            exit(1);
        }
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            if (entry->d_type == DT_DIR)
            {
                // Build the new path for the subdirectory
                char newPath[BUFFER_SIZE];
                char newPath_copy[BUFFER_SIZE];
                snprintf(newPath, sizeof(newPath), "%s/%s", path, entry->d_name);
                strcpy(newPath_copy, newPath);
                // Store the relative path in the accessible_path array
                replaceSubstring(newPath, original, ".");

                strcpy(backup_send[index2], "DIR ");
                strncat(backup_send[index2++], newPath, sizeof(newPath));
                strncpy(server_info.accessible_path[server_info.no_of_paths], newPath, sizeof(server_info.accessible_path[0]));
                server_info.no_of_paths++;

                // Recursively list files in the subdirectory
                listFiles(newPath_copy, original, depth + 1);
            }
            else
            {
                // Build the path for the file
                char filePath[BUFFER_SIZE];
                char filePath_copy[BUFFER_SIZE];
                snprintf(filePath, sizeof(filePath), "%s/%s", path, entry->d_name);
                strcpy(filePath_copy, filePath);
                // Store the relative path in the accessible_path array
                replaceSubstring(filePath, original, ".");
                strcpy(backup_send[index2], "FILE ");
                strncat(backup_send[index2], filePath, sizeof(filePath));

                strncpy(server_info.accessible_path[server_info.no_of_paths], filePath, sizeof(server_info.accessible_path[0]));
                server_info.no_of_paths++;

                // Read the file
                FILE *file = fopen(filePath_copy, "r");
                if (file != NULL)
                {
                    fseek(file, 0, SEEK_END);
                    long fileSize = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    // Allocate memory to store the file content
                    char *content = (char *)malloc(fileSize + 1); // +1 for null-terminator
                    if (content == NULL)
                    {
                        perror("Error allocating memory");
                        fclose(file);
                    }

                    // Read the file content into the array
                    size_t bytesRead = fread(content, 1, fileSize, file);
                    if (bytesRead != fileSize)
                    {
                        perror("Error reading file");
                        free(content);
                        fclose(file);
                    }

                    // Null-terminate the content
                    content[fileSize] = '\0';

                    // Close the file
                    fclose(file);
                    strcat(backup_send[index2], " $");
                    strcat(backup_send[index2++], content);

                    // Now 'content' contains the content of the file
                    // printf("File content:\n%s\n", content);

                    // Don't forget to free the allocated memory
                    free(content);
                }
            }
        }
    }

    closedir(dir);
}

void sendString(int socket, const char *sentence)
{
    size_t messageLength = strlen(sentence);

    // Send the actual string
    if (send(socket, sentence, messageLength, 0) == -1)
    {
        perror("Error sending message");
        exit(EXIT_FAILURE);
    }
    printf("Acknowledgement sent to the Naming Server\n");
}

void handle_read_request(int storage_socket, const char *file_path)
{
    char path[1024];
    getcwd(path, sizeof(path));
    strcat(path, file_path + 1);

    FILE *file = fopen(file_path, "r");
    if (file == NULL)
    {
        perror("Error opening file");
    }
    else
    {
        char buffer[1024];
        size_t bytesRead;

        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0)
        {
            send(storage_socket, buffer, bytesRead, 0);
        }

        fclose(file);

        send(storage_socket, "STOP\n", strlen("STOP\n"), 0);
    }
    char ack[] = "ACK\n";
    // printf("Write done!\n");
    send(storage_socket, ack, strlen(ack), 0);
}

void handle_write_request(int storage_socket, const char *file_path)
{
    char option[4];
    size_t totalSize = 0;
    while (1)
    {
        ssize_t bytes_received = recv(storage_socket, option, sizeof(option), 0);

        if (bytes_received > 0)
        {
            // No more data to receive

            option[bytes_received] = '\0';

            break;
        }
    }

    // printf("%s\n", option);

    FILE *file;

    if (option[0] == 'w') // Option 'w' for writing (truncate file)
    {
        file = fopen(file_path, "w");
    }
    else if (option[0] == 'a') // Option 'a' for appending
    {
        file = fopen(file_path, "a");
    }
    else
    {
        perror("Invalid write option received from client");
        return;
    }
    if (file == NULL)
    {
        perror("Error opening file for write");
        return;
    }

    int file_descriptor = fileno(file);

    // Apply an exclusive lock on the file (prevents others from writing)
    if (flock(file_descriptor, LOCK_EX) == -1)
    {
        perror("Error locking file");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    // Perform operations on the locked file (read, write, etc.)

    char buffer[1024];
    bzero(buffer, sizeof(buffer));
    ssize_t bytes_received;
    while (1)
    {
        bytes_received = recv(storage_socket, buffer, sizeof(buffer), 0);

        if (bytes_received > 0)
        {
            // No more data to receive

            buffer[bytes_received] = '\0';

            // Split received data into newline-separated strings
            printf("%s", buffer);
            fprintf(file, "%s", buffer);
            break;
        }
        else
        {
            break;
        }
    }
    // Release the lock
    if (flock(file_descriptor, LOCK_UN) == -1)
    {
        perror("Error unlocking file");
        fclose(file);
        exit(EXIT_FAILURE);
    }
    fclose(file);
    char ack[] = "ACK\n";
    printf("Write done!\n");
    send(storage_socket, ack, strlen(ack), 0);
}

void handle_retrieve_request(int storage_socket, const char *file_path)
{
    // obtaining file information using stat():
    char path[1024];
    getcwd(path, sizeof(path));
    strcat(path, file_path + 1);

    struct stat fileStat;
    if (stat(path, &fileStat) < 0)
    {
        perror("Error obtaining file information");
        return;
    }

    // Send file size to the client
    if (send(storage_socket, &fileStat.st_size, sizeof(off_t), 0) == -1)
    {
        perror("Error sending file size");
        return;
    }
    printf(" Requested File %s Size sent\n", file_path);
    // Send file permissions to the client
    if (send(storage_socket, &fileStat.st_mode, sizeof(mode_t), 0) == -1)
    {
        perror("Error sending file permissions");
        return;
    }
    printf("Requested File %s Permissions sent\n", file_path);
    char ack[] = "ACK";
    send(storage_socket, ack, strlen(ack), 0);
}

int remove_callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);
    if (rv)
    {
        perror(fpath);
    }
    return rv;
}
void handle_delete_requests(int ss_socket, const char *file_path)
{
    char path[1024];
    getcwd(path, sizeof(path));
    strcat(path, file_path + 1);
    int x = strlen(path);
    path[x] = '\0';
    // printf("%s\n", path);

    if (nftw(path, remove_callback, 64, FTW_DEPTH | FTW_PHYS) == 0)
    {
        printf("Removed successfully.\n");
    }
    else
    {
        perror("Error removing directory");
    }
    char ack[] = "ACK";
    send(ss_socket, ack, strlen(ack), 0);
}
void listFiles1()
{
    for (int i = 0; i < server_info.no_of_paths; i++)
    {
        char path[1024];
        getcwd(path, sizeof(path));
        strcat(path, server_info.accessible_path[i] + 1);
        printf("%s\n", path);

        DIR *dir;
        struct dirent *entry;
        dir = opendir(path);
        if (dir == NULL)
        {
            strcpy(ss_recovery.to_be_sent[index3], "FILE ");
            strcat(ss_recovery.to_be_sent[index3], server_info.accessible_path[i]);

            FILE *file = fopen(path, "r");
            if (file != NULL)
            {
                fseek(file, 0, SEEK_END);
                long fileSize = ftell(file);
                fseek(file, 0, SEEK_SET);

                char *content = (char *)malloc(fileSize + 1); // +1 for null-terminator
                if (content == NULL)
                {
                    perror("Error allocating memory");
                    fclose(file);
                }

                // Read the file content into the array
                size_t bytesRead = fread(content, 1, fileSize, file);
                if (bytesRead != fileSize)
                {
                    perror("Error reading file");
                    free(content);
                    fclose(file);
                }

                // Null-terminate the content
                content[fileSize] = '\0';
                // Close the file
                fclose(file);

                strcat(ss_recovery.to_be_sent[index3], " $");
                strcat(ss_recovery.to_be_sent[index3++], content);
                free(content);
            }
            else
            {
                perror("Not a valid accessible path");
                exit(1);
            }
        }
        else
        {
            strcpy(ss_recovery.to_be_sent[index3], "DIR ");
            strcat(ss_recovery.to_be_sent[index3++], server_info.accessible_path[i]);
        }
    }
    ss_recovery.to_be_sent[index3][0] = '\0';
}
void *handle_downing_of_server(void *arg)
{
    int ss_socket = *((int *)arg);
    while (1)
    {
        char BUFFER[1024];
        scanf("%s", BUFFER);
        if (strcmp(BUFFER, "offline") == 0)
        {
            send(ss_socket, BUFFER, strlen(BUFFER), 0);
            printf("THIS STORAGE SERVER IS DOWN NOW!\n");
        }
        else if (strcmp(BUFFER, "online") == 0)
        {
            send(ss_socket, BUFFER, strlen(BUFFER), 0);
            printf("THIS STORAGE SERVER IS BACK ONLINE!\n");
            listFiles1();
            int j = 0;
            while (ss_recovery.to_be_sent[j][0] != '\0')
            {
                printf("%s\n", ss_recovery.to_be_sent[j]);
                j++;
            }
            usleep(1000);
            send(ss_socket, &ss_recovery, sizeof(ss_recovery), 0);
        }
    }
}
void *handle_naming_server_requests(void *arg)
{
    int ss_socket = *((int *)arg);

    while (1)
    {
        char buffer[1024];
        // printf("hi\n");
        ssize_t bytes_received;
        while (1)
        {
            bytes_received = recv(ss_socket, buffer, sizeof(buffer), 0);
            if (bytes_received <= 0)
            {
                perror("Error receiving data from naming server");
                close(ss_socket);
                pthread_exit(NULL);
            }
            if (buffer[0] != '\0')
                break;
        }

        buffer[bytes_received] = '\0';

        char *tk = strtok(buffer, " ");
        if (strcmp(tk, "PRIV_DELETE") == 0)
        {
            tk = strtok(NULL, " \n");
            handle_delete_requests(ss_socket, tk);
        }
        else if (strcmp(tk, "CREATE") == 0)
        {
            tk = strtok(NULL, " \n");
            if (strcmp(tk, "FILE") == 0)
            {
                tk = strtok(NULL, " \n");
                char file_name[1024], path[1024];
                strcpy(file_name, tk);

                tk = strtok(NULL, " \n");
                tk = strtok(NULL, " \n");
                strcpy(path, tk);
                strcat(path, "/");
                strcat(path, file_name);
                char final_path[1024];
                getcwd(final_path, sizeof(final_path));
                strcat(final_path, path + 1);
                FILE *fp;
                fp = fopen(final_path, "w");
                if (fp == NULL)
                {
                    perror("Error creating file");
                    exit(1);
                }
                fclose(fp);
            }
            else if (strcmp(tk, "DIR") == 0)
            {
                tk = strtok(NULL, " \n");
                char file_name[1024], path[1024];
                strcpy(file_name, tk);
                tk = strtok(NULL, " \n");
                tk = strtok(NULL, " \n");
                strcpy(path, tk);
                strcat(path, "/");
                strcat(path, file_name);
                char final_path[1024];
                getcwd(final_path, sizeof(final_path));
                strcat(final_path, path + 1);
                mkdir(final_path, 0777);
            }

            char ack[] = "ACK";
            send(ss_socket, ack, strlen(ack), 0);
        }
        else if (strcmp(tk, "SEND") == 0)
        {
            tk = strtok(NULL, " \n");
            char final_token[1024];
            getcwd(final_token, sizeof(final_token));
            strcat(final_token, tk + 1);

            char **to_be_send;
            to_be_send = (char **)malloc(100 * sizeof(char *));
            for (int i = 0; i < 100; i++)
            {
                to_be_send[i] = (char *)malloc(1024 * sizeof(char));
            }

            char path[] = "";
            strcpy(path, final_token);
            char *lastSlash = strrchr(path, '/');
            char *child;
            char *parent;
            if (lastSlash != NULL)
            {
                // Tokenize the string using the last '/'
                *lastSlash = '\0'; // Replace the last '/' with '\0' to terminate the first part of the string
                parent = path;
                child = lastSlash + 1;
            }

            struct stat st;

            // Use stat to obtain information about the file
            if (stat(final_token, &st) == -1)
            {
                perror("Error getting file information");
            }

            // Check if it is a directory
            if (S_ISDIR(st.st_mode))
            {
                strcpy(to_be_send[index1], "DIR ./");
                strncat(to_be_send[index1++], child, sizeof(child));
                handle_send_requests(final_token, parent, 0, to_be_send);
                to_be_send[index1][0] = '\0';
            }
            else
            {
                handle_send_requests(final_token, parent, 0, to_be_send);
                to_be_send[index1][0] = '\0';
            }

            struct for_copy copy;
            int j = 0;
            while (to_be_send[j][0] != '\0')
            {
                strcpy(copy.to_be_sent[j], to_be_send[j]);
                printf("%s\n", copy.to_be_sent[j]);
                j++;
            }
            copy.to_be_sent[j][0] = '\0';
            usleep(100);
            int bytes_sent = send(ss_socket, &copy, sizeof(copy), 0);
            if (bytes_sent <= 0)
            {
                perror("Error sending data to naming server");
                close(ss_socket);
                exit(1);
            }
            else
                printf("successfully sent!\n");
        }
        else if (strcmp(tk, "RECIEVE") == 0)
        {
            // printf("here\n");
            char to_be_recieve[100][1024];

            int a = 12;
            send(ss_socket, &a, sizeof(a), 0);
            struct for_copy got;
            while (1)
            {
                if (recv(ss_socket, &got, sizeof(got), 0) == -1)
                {
                    perror("Receive failed");
                    exit(EXIT_FAILURE);
                }
                if (got.to_be_sent[0][0] != '\0')
                    break;
            }
            // printf("here2\n");

            tk = strtok(NULL, " \n");
            if (strcmp(tk, "./BACKUP") == 0)
            {
                char path[1024];
                getcwd(path, sizeof(path));
                strcat(path, tk + 1);
                if (access(path, F_OK) == -1)
                {
                    // printf("exist nhi karti\n");
                    // Directory doesn't exist, so create it
                    if (mkdir(path, 0777) == 0)
                    {
                        printf("Directory created successfully.\n");
                    }
                    else
                    {
                        perror("Error creating directory");
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    printf("Directory '%s' already exists.\n", path);
                }
            }
            int j = 0;
            // printf("here3\n");
            while (got.to_be_sent[j][0] != '\0')
            {
                if (got.to_be_sent[j][0] == 'D')
                {
                    char path[1024];
                    getcwd(path, sizeof(path));
                    strcat(path, tk + 1);
                    char *t1 = strtok(got.to_be_sent[j], " \n");
                    t1 = strtok(NULL, " \n");
                    strcat(path, t1 + 1);
                    // printf("%s\n", path);
                    if (mkdir(path, 0777) != 0)
                        perror("Error in making dir");
                }
                else
                {
                    char path[1024];
                    getcwd(path, sizeof(path));
                    strcat(path, tk + 1);

                    char *t2 = strtok(got.to_be_sent[j], "$");
                    char first[1024];
                    strcpy(first, t2);
                    t2 = strtok(NULL, "$");
                    char *t1 = strtok(first, " ");
                    t1 = strtok(NULL, " ");
                    strcat(path, t1 + 1);

                    FILE *fp;
                    fp = fopen(path, "w");
                    if (fp == NULL)
                    {
                        perror("Error creating file");
                        exit(1);
                    }
                    fprintf(fp, "%s", t2);
                    fclose(fp);
                }
                j++;
            }
            char ack[] = "ACK";
            send(ss_socket, ack, strlen(ack), 0);
        }
    }
}

void *handle_client_requests(void *arg)
{
    int ss_socket = *((int *)arg);

    char buffer[1024];
    ssize_t bytes_received = recv(ss_socket, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0)
    {
        perror("Error receiving data from client");
        close(ss_socket);
        pthread_exit(NULL);
    }

    buffer[bytes_received] = '\0';

    char *tk = strtok(buffer, " ");
    if (strcmp(tk, "READ") == 0)
    {
        tk = strtok(NULL, " \n");
        handle_read_request(ss_socket, tk);
    }
    else if (strcmp(tk, "WRITE") == 0)
    {
        tk = strtok(NULL, " \n");
        handle_write_request(ss_socket, tk);
    }
    else if (strcmp(tk, "RETRIEVE") == 0)
    {
        tk = strtok(NULL, " \n");
        handle_retrieve_request(ss_socket, tk);
    }
}
void *as_a_client_thread_function(void *arg)
{
    int ss_socket, ss_socket2;
    struct sockaddr_in ss_addr, ss_addr2;

    ss_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (ss_socket < 0)
    {
        perror("Error in storage server socket creation");
        exit(1);
    }

    ss_socket2 = socket(AF_INET, SOCK_STREAM, 0);
    if (ss_socket2 < 0)
    {
        perror("Error in storage server socket creation");
        exit(1);
    }

    ss_addr.sin_family = AF_INET;
    ss_addr.sin_port = htons(NM_PORT);
    ss_addr.sin_addr.s_addr = inet_addr(NAMING_SERVER_IP);

    ss_addr2.sin_family = AF_INET;
    ss_addr2.sin_port = htons(NM_PORT2);
    ss_addr2.sin_addr.s_addr = inet_addr(NAMING_SERVER_IP);

    if (connect(ss_socket, (struct sockaddr *)&ss_addr, sizeof(ss_addr)) < 0)
    {
        perror("Error connecting to naming server");
        exit(1);
    }
    usleep(1000);
    if (connect(ss_socket2, (struct sockaddr *)&ss_addr2, sizeof(ss_addr2)) < 0)
    {
        perror("Error connecting to naming server");
        exit(1);
    }

    strncpy(server_info.ip_address, NAMING_SERVER_IP, sizeof(server_info.ip_address));
    server_info.nm_connection_port = NM_PORT;         // Replace with the actual NM connection port
    server_info.client_connection_port = client_port; // Replace with the actual client connection port
    server_info.no_of_paths = 0;

    char input[1024];
    printf("Enter the accessible paths::\n");
    while (1)
    {
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            // Handle fgets error or end-of-file condition
            break;
        }
        if (input[0] == '\n')
        {
            if (fgets(input, sizeof(input), stdin) == NULL)
            {
                // Handle fgets error or end-of-file condition
                break;
            }
            if (input[0] == '\n')
                break;
        }

        char current_dir[1024];
        char original[1024];
        getcwd(current_dir, sizeof(current_dir));
        strcpy(original, current_dir);
        strcat(current_dir, input + 1);
        int x = strlen(input);
        input[x - 1] = '\0';

        strncpy(server_info.accessible_path[server_info.no_of_paths++], input, sizeof(server_info.accessible_path[0]));
        // FILE *fd;

        x = strlen(current_dir);
        current_dir[x - 1] = '\0';
        struct stat path_stat;
        if (stat(current_dir, &path_stat) == 0)
        {
            if (S_ISDIR(path_stat.st_mode))
            {
                strcpy(backup_send[index2], "DIR ");
                strncat(backup_send[index2++], input, sizeof(input));
            }
        }

        listFiles(current_dir, original, 0);
        backup_send[index2][0] = '\0';

        int j = 0;
        while (backup_send[j][0] != '\0')
        {
            strcpy(backup.to_be_sent[j], backup_send[j]);
            printf("%s\n", backup.to_be_sent[j]);
            j++;
        }
    }

    pthread_t down;
    if (pthread_create(&down, NULL, handle_downing_of_server, &ss_socket2) != 0)
    {
        perror("Error creating Naming Server thread");
        exit(1);
    }

    int bytes_sent = send(ss_socket, &server_info, sizeof(server_info), 0);
    if (bytes_sent <= 0)
    {
        perror("Error sending data to naming server");
        close(ss_socket);
        exit(1);
    }
    int b;
    while (1)
    {
        int bytes_received = recv(ss_socket, &b, sizeof(b), 0);

        if (b == 12)
        {
            break;
        }
    }
    usleep(1000);

    bytes_sent = send(ss_socket, &backup, sizeof(backup), 0);
    if (bytes_sent <= 0)
    {
        perror("Error sending data to naming server");
        close(ss_socket);
        exit(1);
    }

    printf("Following data is sent to the naming server:\n");
    printf("IP address: %s\n", server_info.ip_address);
    printf("Port for NM Connection: %d\n", server_info.nm_connection_port);
    printf("Port for Client Connection: %d\n", server_info.client_connection_port);
    printf("No. of paths: %d\n", server_info.no_of_paths);
    printf("Accessible paths:\n");
    for (int i = 0; i < server_info.no_of_paths; i++)
    {
        printf("%s\n", server_info.accessible_path[i]);
    }

    handle_naming_server_requests(&ss_socket);
}

void *as_a_server_thread_function(void *arg)
{
    int nm_server_socket, client_socket;
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
    nm_server_addr.sin_port = htons(client_port);

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

    int count = 0;
    while (1)
    {
        client_socket = accept(nm_server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1)
        {
            perror("Error accepting client connection");
            continue;
        }

        printf("Client %d connected: %s::%d\n", (++count), inet_ntoa(nm_server_addr.sin_addr), ntohs(nm_server_addr.sin_port));

        pthread_t client_request_thread;
        if (pthread_create(&client_request_thread, NULL, handle_client_requests, &client_socket) != 0)
        {
            perror("Error creating Client thread");
            exit(1);
        }
    }
    close(nm_server_socket);
    pthread_exit(NULL);
}

int main()
{
    printf("Enter the port number of the client: ");
    scanf("%d", &client_port);
    pthread_t naming_server_thread, client_thread;

    if (pthread_create(&naming_server_thread, NULL, as_a_client_thread_function, NULL) != 0)
    {
        perror("Error creating Naming Server thread");
        exit(1);
    }

    if (pthread_create(&client_thread, NULL, as_a_server_thread_function, NULL) != 0)
    {
        perror("Error creating Client thread");
        exit(1);
    }

    if (pthread_join(naming_server_thread, NULL) != 0)
    {
        perror("Error joining Naming Server thread");
        exit(1);
    }
    if (pthread_join(client_thread, NULL) != 0)
    {
        perror("Error joining Client thread");
        exit(1);
    }

    return 0;
}
