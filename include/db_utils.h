#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <sys/types.h>
#include <time.h>
#include <libpq-fe.h>

void store_folder_info(const char *name, const char *path, off_t size, time_t last_modified);
int is_file_modified(const char *filename);
PGconn *connect_db();

#endif