#include "path_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <shlobj.h>
#endif

char *get_desktop_path()
{
    char *desktop_path = (char *)malloc(4096);
    if (!desktop_path)
    {
        perror("Failed to allocate memory for desktop path");
        return NULL;
    }

#ifdef _WIN32
    if (SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path) != S_OK)
    {
        perror("Failed to get the desktop path");
        free(desktop_path);
        return NULL;
    }
#else
    const char *home_dir = getenv("HOME");
    if (!home_dir)
    {
        home_dir = getpwuid(getuid())->pw_dir;
        if (!home_dir)
        {
            perror("Failed to get the home directory");
            free(desktop_path);
            return NULL;
        }
    }
    snprintf(desktop_path, 4096, "%s/Desktop", home_dir);
#endif

    return desktop_path;
}

char *get_sync_folder_path()
{
    char *desktop_path = get_desktop_path();
    if (!desktop_path)
    {
        return NULL;
    }

    char *sync_folder_path = (char *)malloc(4096);
    if (!sync_folder_path)
    {
        perror("Failed to allocate memory for sync folder path");
        free(desktop_path);
        return NULL;
    }

    snprintf(sync_folder_path, 4096, "%s/sync", desktop_path);
    free(desktop_path);

    return sync_folder_path;
}
