#include "db_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include <time.h>
#include <sys/types.h>

void store_folder_info(const char *name, const char *path, off_t size, time_t last_modified)
{
    const char *conninfo = "dbname=folder_sync_db user=folder_sync_admin password=souer19ok";
    PGconn *conn = PQconnectdb(conninfo);

    printf("Testing db connection\n");

    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to db failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return;
    }

    printf("DB ACCESS SUCCESSFUL TRYING THE QUERY\n");

    // Convert last_modified to string
    char last_modified_str[256];
    struct tm *lt = localtime(&last_modified);
    strftime(last_modified_str, sizeof(last_modified_str), "%Y-%m-%d %H:%M:%S", lt);

    // Check if the entry already exists
    char check_query[1024];
    snprintf(check_query, sizeof(check_query),
             "SELECT id FROM folder_info WHERE path = '%s'", path);

    PGresult *check_res = PQexec(conn, check_query);

    if (PQresultStatus(check_res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "SELECT command failed: %s\n", PQerrorMessage(conn));
        PQclear(check_res);
        PQfinish(conn);
        return;
    }

    if (PQntuples(check_res) > 0)
    {
        // Entry exists, update it
        char update_query[1024];
        snprintf(update_query, sizeof(update_query),
                 "UPDATE folder_info SET name = '%s', size = %ld, last_modified = '%s' WHERE path = '%s'",
                 name, size, last_modified_str, path);

        PGresult *update_res = PQexec(conn, update_query);

        if (PQresultStatus(update_res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "UPDATE command failed: %s\n", PQerrorMessage(conn));
            PQclear(update_res);
            PQfinish(conn);
            return;
        }

        PQclear(update_res);
    }
    else
    {
        // Entry does not exist, insert it
        char insert_query[1024];
        snprintf(insert_query, sizeof(insert_query),
                 "INSERT INTO folder_info (name, path, size, last_modified) VALUES ('%s', '%s', %ld, '%s')",
                 name, path, size, last_modified_str);

        PGresult *insert_res = PQexec(conn, insert_query);

        if (PQresultStatus(insert_res) != PGRES_COMMAND_OK)
        {
            fprintf(stderr, "INSERT command failed: %s\n", PQerrorMessage(conn));
            PQclear(insert_res);
            PQfinish(conn);
            return;
        }

        PQclear(insert_res);
    }

    PQclear(check_res);
    PQfinish(conn);
}