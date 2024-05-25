// #include "folder_utils.h"
// #include "error_handling.h"
#include "db_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <time.h>
#include <sys/types.h>

void store_folder_info(const char *name, const char *path, off_t size, time_t last_modified)
{
    const char *conninfo = "dbname=folder_sync_db user=folder_sync_admin password=mypassword";
    PGconn *conn = PQconnectdb(conninfo);

    printf("Testing db connection\n");

    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to db failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    printf("DB ACCESS SUCCESSFUL TRYING THE QUERY\n");

    char query[1024];
    struct tm *lt = localtime(&last_modified);
    char last_modified_str[256];
    strftime(last_modified_str, sizeof(last_modified_str), "%Y-%m-%d %H:%M:%S", lt);

    snprintf(query, sizeof(query),
             "INSERT INTO folder_info (name, path, size, last_modified) VALUES ('%s', '%s', '%ld', '%s')",
             name, path, size, last_modified_str);

    PGresult *response = PQexec(conn, query);

    if (PQresultStatus(response) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "INSERT COMMAND FAILED: %s", PQerrorMessage(conn));
        PQclear(response);
        PQfinish(conn);
        return;
    }

    PQclear(response);
    PQfinish(conn);
}