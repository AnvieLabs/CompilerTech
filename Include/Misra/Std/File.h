/// file      : misra/std/file.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, All rights reserved.
///
/// File helper utilities

#ifndef MISRA_FILE_H
#define MISRA_FILE_H

#include <stddef.h>
#include <stdint.h>

// ct
#include <Misra/Std/Container/Str.h>

typedef enum {
    DIR_ENTRY_TYPE_UNKNOWN,
    DIR_ENTRY_TYPE_REGULAR_FILE,
    DIR_ENTRY_TYPE_DIRECTORY,
    DIR_ENTRY_TYPE_PIPE,
    DIR_ENTRY_TYPE_SOCKET,
    DIR_ENTRY_TYPE_CHARACTER_DEVICE,
    DIR_ENTRY_TYPE_BLOCK_DEVICE,
    DIR_ENTRY_TYPE_SYMBOLIC_LINK
} DirEntryType;

///
/// Convert given entry type to a NULL terminated string.
/// Provided string must not be freed as it's not allocated.
///
/// type[in] : Entry type to get string of.
///
/// RETURN : Null terminated string.
///
const char *DirEntryTypeToZStr (DirEntryType type);

typedef struct {
    DirEntryType type;
    size_t       size;
    Str          name;
} DirEntry;

DirEntry *DirEntryInitCopy (DirEntry *dst, DirEntry *src);
DirEntry *DirEntryDeinitCopy (DirEntry *copy);

typedef Vec (DirEntry) DirContents;

///
/// Read directory contents into a vector
/// Current contents of the vector will be cleared out.
///
/// dir[in,out] : Directory contents will be stored here.
/// path[in]    : Path of directory get content of.
///
/// SUCCESS : `dir`
/// FAILURE : NULL
///
DirContents *ReadDirContents (DirContents *dir, const char *path);

///
/// Get size of file without opening it.
///
/// filename[in] : Name/path of file.
///
/// SUCCESS : Non-negative value representing size of file in bytes.
/// FAILURE : -1
///
int64_t GetFileSize (const char *filename);

///
/// Read complete contents of file at once.
///
/// Pointer returned is malloc'd and hence must be freed after use.
/// The returned pointer can also be reused by providing pointer to it
/// in `data` parameter.
///
/// `realloc` is called on `*data` in order to expand it's size.
/// If `*capacity` exceeds the size of file to be loaded, then no reallocation
/// is performed. This means the provided buffer will automatically be expanded
/// if required.
///
/// The returned buffer is null-terminated just-in-case.
///
/// The implementation and API is designed in such a way that it can be used
/// with containers like Vec and Str.
///
/// filename[in]     : Name/path of file to be read.
/// data[in,out]     : Memory buffer where loaded file will be stored.
/// file_size[out]   : Complete size of file in bytes will be stored here.
/// capacity[in,out] : Hints towards current capacity of `data` buffer.
///                    New capacity of `data` buffer is automatically stored here if
///                    realloc is performed.
///
/// SUCCESS : Returns a malloc'd array with read file contents.
/// FAILURE : NULL
///
void *ReadCompleteFile (const char *filename, void **data, size_t *file_size, size_t *capacity);

#endif // MISRA_FILE_H
