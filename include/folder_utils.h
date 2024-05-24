#ifndef FOLDER_UTILS_H
#define FOLDER_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <error_handling.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>

void create_sync_folder();

int is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}

void create_synclist(char *desktop_file_path) {
    char syncListPath[4096];
    snprintf(syncListPath, sizeof(syncListPath), "%s/sync/synclist.txt", desktop_file_path);

    if (is_directory(syncListPath) == 0) {
        printf("Synclist File Already Exist. \n Checking the indexes \n");
        return;
    }

    int fd = open(syncListPath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd == -1) {
        handle_error("Failed to create synclist file");
        return;
    }
    close(fd);
    printf("Synclist track file created at %s\n", syncListPath);
}

void create_sync_folder() {
    char desktop_path[4096];

#ifdef _WIN32
    if(SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path) != S_OK)
        {
        handle_error("Failed to get the desktop path");
        return;
        }

#else
    const char *home_dir = getenv("HOME");
    if (!home_dir) {
        printf("HOME environment variable not set. Using getpwuid.\n");
        home_dir = getpwuid(getuid())->pw_dir;
    } else {
        printf("HOME environment variable set to: %s\n", home_dir);
    }
    snprintf(desktop_path, sizeof(desktop_path), "%s/Desktop", home_dir);
    printf("Unix-like Desktop path: %s\n", desktop_path);
#endif

    char sync_folder_path[4096];
    snprintf(sync_folder_path, sizeof(sync_folder_path), "%s/sync", desktop_path);

    if (is_directory(sync_folder_path) == -1) {
        if (mkdir(sync_folder_path, 0755) == -1) {
            handle_error("Failed to create a sync folder");
            return;
        }
        printf("Sync folder created at: %s\n", sync_folder_path);
    } else {
        printf("Sync folder already exists at %s\n", sync_folder_path);
    }

    create_synclist(desktop_path);
}

#endif