/*
 * win32_dirent.h - Minimal POSIX dirent emulation for Windows
 * Uses FindFirstFile / FindNextFile to emulate opendir / readdir / closedir.
 */

#ifndef WIN32_DIRENT_H
#define WIN32_DIRENT_H

#ifdef _WIN32

#include <windows.h>

#define NAME_MAX 255

struct dirent {
    char d_name[NAME_MAX + 1];
};

typedef struct {
    HANDLE          hFind;
    WIN32_FIND_DATAA findData;
    struct dirent   ent;
    int             first;
} DIR;

static inline DIR *opendir(const char *path) {
    DIR *dir = (DIR *)malloc(sizeof(DIR));
    if (!dir) return NULL;
    memset(dir, 0, sizeof(DIR));

    char pattern[MAX_PATH];
    if (snprintf(pattern, sizeof(pattern), "%s\\*", path) >= (int)sizeof(pattern)) {
        free(dir);
        return NULL;
    }

    dir->hFind = FindFirstFileA(pattern, &dir->findData);
    if (dir->hFind == INVALID_HANDLE_VALUE) {
        free(dir);
        return NULL;
    }
    dir->first = 1;
    return dir;
}

static inline struct dirent *readdir(DIR *dir) {
    if (!dir) return NULL;

    if (dir->first) {
        dir->first = 0;
    } else {
        if (!FindNextFileA(dir->hFind, &dir->findData))
            return NULL;
    }

    strncpy(dir->ent.d_name, dir->findData.cFileName, NAME_MAX);
    dir->ent.d_name[NAME_MAX] = '\0';
    return &dir->ent;
}

static inline int closedir(DIR *dir) {
    if (!dir) return -1;
    FindClose(dir->hFind);
    free(dir);
    return 0;
}

#endif /* _WIN32 */
#endif /* WIN32_DIRENT_H */
