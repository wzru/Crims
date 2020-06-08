#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "debug.h"
#include "server.h"
#include "getopt.h"
#include "pthread.h"
#include "database.h"

inline int start (int argc, char *argv[])
{
    int opt = 0;
    while ( (opt = getopt (argc, argv, "p:d:")) != -1) //解析参数
    {
        switch (opt)
        {
        case 'p':
            sscanf (optarg, "%d", &listen_port);
            printf ("%d\n", listen_port);
            break;
        case 'd':
            sscanf (optarg, "%s", database_path);
            break;//可能对带空格的路径支持有问题, 需要加引号
        }
    }
    initialize_log();
    log ("[INFO]: Start server on port %d...\n", listen_port);
    pthread_t server_thread;
    int server_thread_index = pthread_create (&server_thread, NULL,
                              (void *) &run_server, NULL);
    log ("[INFO]: Reading data from '%s'...\n", database_path);
    if (read (database_path))
    {
        log ("[ERROR]: Cannot read data file\n");
        return -1;
    }
    int *server_thread_result = malloc (sizeof (int));
    pthread_join (server_thread, (void **) &server_thread_result);
    if (server_thread_result == NULL || *server_thread_result)
    {
        log ("[ERROR]: Server failed to start\n");
        return -1;
    }
    return 0;
}