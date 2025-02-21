/// file      : file.c
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, All rights reserved.
///
/// File helper utilities implementation

#include <stdlib.h>

// beam
#include <Misra/Std/File.h>
#include <Misra/Std/Log.h>

// platform
#include <dirent.h>
#include <sys/stat.h>

const char *DirEntryTypeToZStr (DirEntryType type) {
    switch (type) {
        case DIR_ENTRY_TYPE_UNKNOWN :
            return "Unknown";
        case DIR_ENTRY_TYPE_REGULAR_FILE :
            return "Regular File";
        case DIR_ENTRY_TYPE_DIRECTORY :
            return "Directory";
        case DIR_ENTRY_TYPE_PIPE :
            return "Pipe";
        case DIR_ENTRY_TYPE_SOCKET :
            return "Socket";
        case DIR_ENTRY_TYPE_CHARACTER_DEVICE :
            return "Character Device";
        case DIR_ENTRY_TYPE_BLOCK_DEVICE :
            return "Block Device";
        case DIR_ENTRY_TYPE_SYMBOLIC_LINK :
            return "Symbolic Link";
        default :
            return "Invalid Type";
    }
}


DirEntry *DirEntryInitCopy (DirEntry *dst, DirEntry *src) {
    if (!dst || !src) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    dst->type = src->type;
    StrInitCopy (&dst->name, &src->name);

    return dst;
}


DirEntry *DirEntryDeinitCopy (DirEntry *copy) {
    if (!copy) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    StrDeinitCopy (&copy->name);
    copy->type = 0;

    return copy;
}


DirContents *ReadDirContents (DirContents *dir_contents, const char *path) {
    if (!dir_contents || !path) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    // Make sure dir vec is cleared out and also has copy initer and deiniter methods
    VecInit (dir_contents, DirEntryInitCopy, DirEntryDeinitCopy);

    DIR *dir = opendir (path);
    if (NULL == dir) {
        LOG_ERROR ("opendir() failed : %s.", strerror (errno));
        return NULL;
    }

#define DNAME(idx) entry->d_name[idx]

    struct dirent *entry = NULL;
    while (NULL != (entry = readdir (dir))) {
#if __APPLE__
        size_t namelen = entry->d_namlen;
#elif __linux__
        size_t namelen = entry->d_reclen;
#endif

        if ('.' == DNAME (0) && 0 == DNAME (1)) {
            continue;
        } else if ('.' == DNAME (0) && '.' == DNAME (1) && 0 == DNAME (2)) {
            continue;
        } else if (0 == strncmp (entry->d_name, ".git", 4)) {
            continue;
        } else {
            DirEntry direntry = {0};
            switch (entry->d_type) {
                case DT_REG :
                    direntry.type = DIR_ENTRY_TYPE_REGULAR_FILE;
                    break;
                case DT_DIR :
                    direntry.type = DIR_ENTRY_TYPE_DIRECTORY;
                    break;
                case DT_FIFO :
                    direntry.type = DIR_ENTRY_TYPE_PIPE;
                    break;
                case DT_SOCK :
                    direntry.type = DIR_ENTRY_TYPE_SOCKET;
                    break;
                case DT_CHR :
                    direntry.type = DIR_ENTRY_TYPE_CHARACTER_DEVICE;
                    break;
                case DT_BLK :
                    direntry.type = DIR_ENTRY_TYPE_BLOCK_DEVICE;
                    break;
                case DT_LNK :
                    direntry.type = DIR_ENTRY_TYPE_SYMBOLIC_LINK;
                    break;
                case DT_UNKNOWN :
                default :
                    direntry.type = DIR_ENTRY_TYPE_UNKNOWN;
            }
            TempStrFromCStr (&direntry.name, entry->d_name, namelen);
            VecPushBack (dir_contents, &direntry);
        }
    }

#undef DNAME

    closedir (dir);

    return dir_contents;
}


int64_t GetFileSize (const char *filename) {
    if (!filename) {
        LOG_ERROR ("invalid arguments.");
        return -1;
    }

    struct stat file_stat;
    if (0 == stat (filename, &file_stat)) {
        return file_stat.st_size;
    } else {
        LOG_ERROR ("failed to get file size : %s.", strerror (errno));
        return -1;
    }
}


void *ReadCompleteFile (const char *filename, void **data, size_t *file_size, size_t *capacity) {
    if (!filename || !data || !file_size || !capacity) {
        LOG_ERROR ("invalid arguments.");
        return NULL;
    }

    // get actual size of file
    int64_t size = GetFileSize (filename);
    if (-1 == size) {
        LOG_ERROR ("failed to get file size");
        return NULL;
    }

    // allocate memory to hold the file contents if required
    void *buffer = *data;
    if (*capacity < (size_t)size) {
        buffer = realloc (buffer, size + 1);
        if (!buffer) {
            LOG_ERROR ("malloc() failed : %s.", strerror (errno));
            return NULL;
        }

        *capacity = size + 1;
    }

    // Open the file in binary mode
    FILE *file = fopen (filename, "rb");
    if (!file) {
        LOG_ERROR ("fopen() failed : %s.", strerror (errno));
        free (buffer);
        return NULL;
    }

    // Read the entire file into the buffer
    if (size != (int64_t)fread (buffer, 1, size, file)) {
        LOG_ERROR ("failed to read complete file. : %s", strerror (errno));
        fclose (file);
        return NULL;
    }

    // Close the file and return the buffer
    fclose (file);

    ((char *)buffer)[size] = 0; // null-termination for just in case.
    *data                  = buffer;
    *file_size             = size;
    return buffer;
}
