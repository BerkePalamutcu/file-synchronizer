#include "command_handler.h"
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

extern pthread_mutex_t db_mutex;

int main(int argc, char **argv)
{
    if (pthread_mutex_init(&db_mutex, NULL) != 0)
    {
        perror("Mutex initialization failed");
        return 1;
    }

    handle_command(argc, argv);

    pthread_mutex_destroy(&db_mutex);
    return 0;
}