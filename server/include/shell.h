#ifndef SHELL_H
#define SHELL_H

#define command_prompt '$'
#define COMMAND_BUFFER_LENGTH 1024
extern char command_buffer[COMMAND_BUFFER_LENGTH];
#define SHELL_EXIT 20001027

inline int shell (int argc, char *argv[]);

#endif 