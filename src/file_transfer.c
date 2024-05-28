#include "path_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <pthread.h>

#define PORT 12345
#define BUFFER_SIZE 1024

typedef struct
{
    char file_path[1024];
    char server_ip[256];
    int port;
} file_transfer_data_t;



void *send_file_thread(void *arg)
{
    file_transfer_data_t *data = (file_transfer_data_t *)arg;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(data->port);

    if (inet_pton(AF_INET, data->server_ip, &serv_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        close(sock);
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Connection Failed");
        close(sock);
        return NULL;
    }

    FILE *file = fopen(data->file_path, "rb");
    if (file == NULL)
    {
        perror("Failed to open file");
        close(sock);
        return NULL;
    }

    // Send file name
    char *filename = strrchr(data->file_path, '/') + 1;
    send(sock, filename, strlen(filename) + 1, 0); // +1 to include null terminator

    // Send file content
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
    {
        send(sock, buffer, bytes_read, 0);
    }

    fclose(file);
    close(sock);
    free(data);
    return NULL;
}

void listen_for_files(int port)
{
    int server_fd, new_socket;
    struct sockaddr_in address;

    int opt = 1;
    int addrlen = sizeof(address);

    char buffer[BUFFER_SIZE] = {0};
    char *sync_folder_path = get_sync_folder_path();

    // CREATING SOCKET DESCRIPTOR HERE!
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // ATTACH SOCKET TO THE PORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsocketopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // BIND THE SOCKET TO THE ADDRESS HERE
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failure");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // ACCEPT INCOMING CONNECTIONS
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&address)) < 0)
    {
        perror("accepting socket error");
        exit(EXIT_FAILURE);
    }

    long valread = read(new_socket, buffer, BUFFER_SIZE);
    printf("%s\n", buffer);

    save_to_sync_folder(buffer, valread, sync_folder_path);

    close(new_socket);
    close(server_fd);
}

void save_to_sync_folder(char *data, size_t size, const char *folder_path)
{
    // Construct the full path for the file to be saved
    char file_path[1024];
    snprintf(file_path, sizeof(file_path), "%s/received_file", folder_path);

    // Open the file for writing in binary mode/
    FILE *file = fopen(file_path, "wb");
    if (file == NULL)
    {
        perror("Failed to open file for writing");
        return;
    }

    // Write the data to the file
    size_t bytes_written = fwrite(data, sizeof(char), size, file);
    if (bytes_written < size)
    {
        perror("Failed to write all data to file");
    }
    else
    {
        printf("Data successfully saved to %s\n", file_path);
    }

    // Close the file
    fclose(file);
}

void send_files(const char *folder_path, const char *server_ip, int port)
{
    DIR *dir = opendir(folder_path);
    if (dir == NULL)
    {
        perror("Could not open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        { // Check if it's a regular file
            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", folder_path, entry->d_name);

            // Check if the file has been modified
            if (!is_file_modified(entry->d_name))
            {
                continue; // Skip sending this file
            }

            file_transfer_data_t *data = malloc(sizeof(file_transfer_data_t));
            strcpy(data->file_path, file_path);
            strcpy(data->server_ip, server_ip);
            data->port = port;

            pthread_t thread;
            pthread_create(&thread, NULL, send_file_thread, data);
            pthread_detach(thread); // Detach the thread to handle sending independently
        }
    }

    closedir(dir);
}