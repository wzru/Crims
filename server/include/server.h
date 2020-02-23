#ifndef SERVER_H
#define SERVER_H

#define LISTEN_PORT_LENGTH 5
#define DEFAULT_LISTEN_PORT 8000
#define BUFFER_LENGTH 1024

extern int listen_port;//监听端口号
extern int address_length;
extern char send_buffer[BUFFER_LENGTH];

inline int run_server();

#endif