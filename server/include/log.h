#ifndef LOG_H
#define LOG_H

#include "define.h"

// extern FILE *log_stream = NULL;
// extern char log_path[PATH_LENGTH];

inline void plog (const char *fmt, ...);
inline void initialize_log();

#endif