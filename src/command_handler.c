#include "command_handler.h"
#include "path_utils.h"
#include "folder_utils.h"
#include "arg_parser.h"
#include "file_transfer.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

extern pthread_mutex_t db_mutex;

void handle_command(int argc, char **argv)
{
    struct arguments arguments;

    // Parse command-line arguments
    parse_arguments(argc, argv, &arguments);

    // Initialize the mutex
    if (pthread_mutex_init(&db_mutex, NULL) != 0)
    {
        perror("Mutex failed bro");
        return;
    }

    printf("Command: %s\n", arguments.command);

    if (strcmp(arguments.command, "create") == 0)
    {
        create_sync_folder();
    }
    else if (strcmp(arguments.command, "update") == 0)
    {
        const char *sync_folder_path = get_sync_folder_path();
        printf("sync folder path is: %s\n", sync_folder_path);
        store_folders_recursively((void *)sync_folder_path); // Cast to void *
    }
    else if (strcmp(arguments.command, "send") == 0)
    {
        const char *sync_folder_path = "/home/alex/Desktop/sync"; // Update this path as nee:wa
        const char *server_ip = "192.168.1.5";                   // Update this IP as needed
        int server_port = 12345;                               // Update this port as needed

        // send_files(sync_folder_path, server_ip, server_port);
        printf("Sending files...");
    }
    else if (strcmp(arguments.command, "listen") == 0)
    {
        int port = 12345;
        printf("Listening on port %d\n", port);
        listen_for_files(port);

    }
    else
    {
        fprintf(stderr, "Unknown command: %s\n", arguments.command);
        return;
    }

    // Destroy the mutex
    pthread_mutex_destroy(&db_mutex);
}
