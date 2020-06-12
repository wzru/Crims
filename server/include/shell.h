#ifndef SHELL_H
#define SHELL_H

#include "server.h"

#define command_prompt '$'
extern char command_buffer[BUFFER_LENGTH];
#define SHELL_EXIT 20001027

inline int shell (int argc, char *argv[]);

#endif 