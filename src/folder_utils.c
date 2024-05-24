#include "folder_utils.h"
#include "error_handling.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <pwd.h>
#endif




int main(){
    create_sync_folder();
    return 0;
}