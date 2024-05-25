#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <sys/types.h>
#include <time.h>

void store_folder_info(const char *name, const char *path, off_t size, time_t last_modified);

#endif