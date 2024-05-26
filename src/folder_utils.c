#include "folder_utils.h"
#include "error_handling.h"
#include "db_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <pwd.h>
#endif
#include <dirent.h>

pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER; // Define db_mutex here

int is_directory(const char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}

int file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

void create_synclist(const char *desktop_file_path)
{
    char syncListPath[4096];
    snprintf(syncListPath, sizeof(syncListPath), "%s/sync/synclist.txt", desktop_file_path);

    if (file_exists(syncListPath))
    {
        printf("Synclist file already exists at: %s\n", syncListPath);
        return;
    }

    int fd = open(syncListPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        perror("Failed to create synclist file");
        handle_error("Failed to create synclist file");
        return;
    }
    close(fd);
    printf("Synclist track file created at: %s\n", syncListPath);
}

void *store_folders_recursively(void *arg)
{
    const char *folder_path = (const char *)arg;
    if (!folder_path)
    {
        fprintf(stderr, "Folder path is NULL\n");
        return NULL;
    }

    printf("Processing folder: %s\n", folder_path);

    DIR *dir = opendir(folder_path);
    if (!dir)
    {
        perror("Failed to open directory");
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue; // Skip the current directory and the parent directory
        }

        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", folder_path, entry->d_name); // Construct the full path for the folder

        struct stat st; // Check if it is a valid directory
        if (stat(path, &st) == -1)
        {
            perror("Failed to stat");
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            // This block will be executed for the folders

            // Lock the thread before accessing the db
            pthread_mutex_lock(&db_mutex);
            store_folder_info(entry->d_name, path, st.st_size, st.st_mtime);
            pthread_mutex_unlock(&db_mutex);

            pthread_t thread;
            char *new_path = strdup(path); // Each thread needs its own path
            if (!new_path)
            {
                perror("Failed to allocate memory for new path");
                continue;
            }
            printf("Pointer value of the new path: %p\n", (void *)new_path);

            // Create the threads
            if (pthread_create(&thread, NULL, store_folders_recursively, new_path) != 0)
            {
                perror("Couldn't create thread");
                free(new_path);
            }
            else
            {
                // Detach the thread to run it independently
                pthread_detach(thread);
            }
        }
        else
        {
            // This block will be executed for the files
            pthread_mutex_lock(&db_mutex);
            store_folder_info(entry->d_name, path, st.st_size, st.st_mtime);
            pthread_mutex_unlock(&db_mutex);
        }
    }

    closedir(dir); // Close the directory
    return NULL;
}

void create_sync_folder()
{
    char desktop_path[4096];

#ifdef _WIN32
    if (SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path) != S_OK)
    {
        handle_error("Failed to get the desktop path");
        return;
    }
    printf("Windows Desktop path: %s\n", desktop_path);
#else
    const char *home_dir = getenv("HOME");
    if (!home_dir)
    {
        printf("Environment variable has not been set!");
        home_dir = getpwuid(getuid())->pw_dir;
    }
    else
    {
        printf("HOME environment variable set to: %s\n", home_dir);
    }
    snprintf(desktop_path, sizeof(desktop_path), "%s/Desktop", home_dir);
    printf("Unix-like Desktop path: %s\n", desktop_path);
#endif

    char sync_folder_path[4096];
    snprintf(sync_folder_path, sizeof(sync_folder_path), "%s/sync", desktop_path);

    printf("Attempting to create sync folder at: %s\n", sync_folder_path);

    if (!is_directory(sync_folder_path))
    {
        if (mkdir(sync_folder_path, 0755) == -1)
        {
            perror("Failed to create sync folder");
            handle_error("Failed to create sync folder");
            return;
        }
        printf("Sync folder created at: %s\n", sync_folder_path);
    }
    else
    {
        printf("Sync folder already exists at: %s\n", sync_folder_path);
    }

    create_synclist(desktop_path);
}
