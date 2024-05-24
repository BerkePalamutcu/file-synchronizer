#ifndef FILE_SYNCHRONIZER_ERROR_HANDLING_H
#define FILE_SYNCHRONIZER_ERROR_HANDLING_H

#include <stdio.h>

void handle_error(const char *error_message){
    fprintf(stderr, "Error: %s\n", error_message);
}

#endif //FILE_SYNCHRONIZER_ERROR_HANDLING_H
