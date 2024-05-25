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

pthread_mutex_t db_mutex;

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
    DIR *dir = opendir(folder_path);
    if (!dir)
    {
        perror("Failed to open directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir) != NULL))
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue; // this means skip the current directory and the parent directory!!!
        }

        char path[4096];
        snprintf(path, sizeof(path), "%s\%s", folder_path, entry->d_name); // we are constructing the full path for the folder here

        struct stat st; // unfortunately checking only if it is valid directory is not enough alone extra stat needs to be created
        if (stat(path, &st) == -1)
        {
            perror("Failed to stat");
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            //!!!FIRST IF BLOCK WILL BE EXECUTED FOR THE FOLDERS!!!

            // must lock the thread before accessing to the db! don't change this!
            pthread_mutex_lock(&db_mutex);
            store_folder_info(entry->d_name, path, st.st_size, st.st_mtime);
            pthread_mutex_unlock(&db_mutex);

            pthread_t thread;
            char *new_path = strdup(path); //!!!EACH THREAD NEEDS IT'S OWN PATH!!!!
            printf("Pointer value of the new path: %p\n", (void *)new_path);

            // HERE WE GO!! LET'S CREATE THE THREADS AND HAVE SOME FUN!!!
            if (pthread_create(&thread, NULL, store_folders_recursively, new_path) != 0)
            {
                perror("COuldn't create thread");
                free(new_path);
            }
            else
            {
                /*
                IF THREAD CREATION IS SUCCESSFUL DETACH IT AND RUN IT INDEPENDENTLY
                SO THAT THERE IS NO NEED TO FREE IT LATER
                */
                pthread_detach(thread);
            }
        }
        else
        {
            //!!!THIS BLOCK WILL BE EXECUTED FOR THE FILES!!!
            pthread_mutex_lock(&db_mutex);
            store_folder_info(entry->d_name, path, st.st_size, st.st_mtime);
            pthread_mutex_unlock(&db_mutex);
        }
    }

    closedir(dir); // the function performed it's duty now let's close the directory.
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
        printf("HOME environment variable not set. Using getpwuid.\n");
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

int main()
{
    create_sync_folder();
    store_folder_info("test", "test2", 123123, 123123);
    return 0;
}
