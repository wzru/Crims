#include <stdio.h>
#include <time.h>

#include "stdarg.h"
#include "debug.h"
#include "log.h"

#define LOG_TIME_FMT "%d/%02d/%02d %02d:%02d:%02d "

FILE *log_stream = NULL;
char log_path[PATH_LENGTH] = "../data/crims_server.log";

inline void initialize_log()
{
    log_stream = fopen (log_path, "a");
    if (log_stream == NULL)
    {
        printf ("ERROR! Cannot open log file.\n");
    }
}

inline void log (const char *fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    time_t now;
    struct tm *tm_now;
    time (&now);
    tm_now = localtime (&now) ;
    switch (crims_status)
    {
    case STATUS_SHELL:
    case STATUS_EXEC:
        vfprintf (stderr, fmt, args);
        break;
    case STATUS_SERVER:
        fprintf (log_stream, LOG_TIME_FMT, tm_now->tm_year + 1900,
                  tm_now->tm_mon + 1, tm_now->tm_mday,
                  tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
        vfprintf (log_stream, fmt, args);
        break;
    }
}