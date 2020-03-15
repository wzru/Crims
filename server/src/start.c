#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"
#include "server.h"
#include "pthread.h"
#include "database.h"

inline int start (int argc, char *argv[])
{
    char opt = 0;
    while ( (opt = getopt (argc, argv, "p:d:")) != -1) //解析参数
    {
        switch (opt)
        {
        case 'p':
            sprintf_s (optarg, LISTEN_PORT_LENGTH, "%d", listen_port);
            break;
        case 'd':
            sprintf_s (optarg, DATABASE_PATH_LENGTH, "%s", database_path);
            break;//可能对带空格的路径支持有问题
        }
    }
    printf ("Start server...\n");
    pthread_t server_thread;
    int server_thread_index = pthread_create (&server_thread, NULL,
                              (void *) &run_server, NULL);
    if (read (database_path))
    {
        printf ("Database start ERROR!\n");
        return -1;
    }
    int *server_thread_result = malloc (sizeof (int));
    pthread_join (server_thread, (void**)&server_thread_result);
    if (server_thread_result == NULL || *server_thread_result)
    {
        printf ("Server ERROR!\n");
        return -1;
    }
    return 0;
}