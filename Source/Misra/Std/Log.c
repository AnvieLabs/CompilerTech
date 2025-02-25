#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>

// linux
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

// beam
#include <Misra/Std/Log.h>

static FILE           *stderror = NULL;
static pthread_mutex_t lock     = PTHREAD_MUTEX_INITIALIZER;

#define LOCK()   pthread_mutex_lock(&lock);
#define UNLOCK() pthread_mutex_unlock(&lock);

void LogInit(bool redirect) {
    if(redirect) {
        // Get the current time
        time_t     raw_time;
        struct tm *time_info;
        char       time_buffer[20] = {0};

        time(&raw_time);
        time_info = localtime(&raw_time);
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d-%H-%M-%S", time_info);

        // generate log file name
        char file_name[128] = {0};
        snprintf(file_name, 127, "/tmp/beam-%s", time_buffer);
        printf("storing logs in %s\n", file_name);

        // Open the file for writing (create if it doesn't exist, overwrite if it does)
        stderror = freopen(file_name, "w", stderr);

        if(stderror == NULL) {
            printf("error opening file : %s\n", strerror(errno));
            return;
        }

        setbuf(stderror, 0);
    } else {
        setbuf(stderr, 0);
    }
}


void LogDeinit() {
    if(stderror) {
        fclose(stderror);
    }
}


void LogWrite(LogMessageType type, const char *tag, int line, const char *format, ...) {
    if(!tag || !format) {
        LOG_ERROR("invalid arguments.");
        return;
    }

    // Get the current time
    time_t     raw_time;
    struct tm *time_info;
    char       time_buffer[20];

    time(&raw_time);
    time_info = localtime(&raw_time);
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", time_info);

    const char *msg_type = NULL;
    switch(type) {
        case LOG_MESSAGE_TYPE_INFO :
            msg_type = "INFO";
            break;
        case LOG_MESSAGE_TYPE_ERROR :
            msg_type = "INFO";
            break;
        case LOG_MESSAGE_TYPE_FATAL :
            msg_type = "INFO";
            break;
        default :
            msg_type = "UNKNOWN_MESSAGE_TYPE";
            break;
    }

    LOCK();

    // Print the log prefix to stderr
    fprintf(stderr, "[%s] [%s] [%s:%d] ", msg_type, time_buffer, tag, line);

    // Process the variadic arguments and print the log message
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    // Print a newline for better readability
    fprintf(stderr, "\n");

    UNLOCK();
}
