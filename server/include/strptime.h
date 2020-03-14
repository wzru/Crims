#ifndef STRPTIME_H
#define STRPTIME_H

#include <time.h>

#define CRIMS_DATETIME_FORMAT "%Y-%m-%d/%H:%M"
char *strptime (const char *buf, const char *format, struct tm *tm);

#endif