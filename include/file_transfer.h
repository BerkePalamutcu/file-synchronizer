#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <stddef.h> // For size_t

// Function to start listening for files on a specified port
void listen_for_files(int port);

// Function to send files to a specified IP address and port
void send_files(const char *folder_path, const char *server_ip, int port);

// Function to save received data to the sync folder
void save_to_sync_folder(char *data, size_t size, const char *folder_path);

#endif // FILE_TRANSFER_H