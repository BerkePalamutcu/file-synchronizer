#ifndef FOLDER_UTILS_H
#define FOLDER_UTILS_H

#include <pthread.h>

extern pthread_mutex_t db_mutex;

void create_sync_folder();

int is_directory(const char *path);

void create_synclist(const char *desktop_file_path);

void *store_folders_recursively(void *arg);

#endif
