/// file      : selfstart/file.h
/// author    : Siddharth Mishra (admin@brightprogrammer.in)
/// copyright : Copyright (c) 2024, Siddharth Mishra, All rights reserved.
///
/// Provides helper functions for easy file manipulation for SelfStart.
///

#ifndef MISRA_SELFSTART_FILE_H
#define MISRA_SELFSTART_FILE_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "Log.h"

///
/// Check whether a file with given name exists or not.
///
#define FILE_EXISTS(x) (access (x, F_OK) == 0)

// Function to extract the directory from a given file path
static inline const char *GetDirFromFilePath (const char *file_path) {
    if (!file_path) {
        LOG_ERROR ("Invalid arguments");
        return NULL; // Handle invalid input
    }

    // Find the last slash in the file path
    const char *last_slash = strrchr (file_path, '/');

    // Calculate the length of the directory path
    size_t dir_len = (last_slash) ? (size_t)(last_slash - file_path) : 1; // +1 for '/', 2 for "./"

    // Copy the directory path into the allocated memory
    if (last_slash) {
        return strndup (file_path, dir_len);
    } else {
        return strdup (".");
    }
}

// Function to dump a const char* to a given filename
static inline void WriteToFile (const char *filename, const char *str) {
    if (!filename || !str) {
        LOG_ERROR ("Invalid arguments");
        return; // Return error if filename or string is NULL
    }

    // Open the file for writing
    FILE *file = fopen (filename, "w");
    if (!file) {
        perror ("Error opening file");
        return; // Return error if file cannot be opened
    }

    // Write the string to the file
    size_t len     = strlen (str);
    size_t written = fwrite (str, sizeof (char), len, file);

    // Check if the write was successful
    if (written != len) {
        perror ("Error writing to file");
        fclose (file);
        return; // Return error if write failed
    }

    // Close the file
    fclose (file);
}

#endif
