#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "log.h"
#include "debug.h"
#include "server.h"
#include "getopt.h"
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
            //printf ("%d\n", listen_port);
            break;
        case 'd':
            sscanf (optarg, "%s", database_path);
            break;//可能对带空格的路径支持有问题, 需要加引号
        }
    }
    log ("[INFO]: Reading data from '%s'...\n", database_path);
    //多线程处理, IO不等待
    pthread_t read_db_thread;
    pthread_create (&read_db_thread, NULL, read, database_path);
    initialize_log();
    pthread_join (read_db_thread, NULL);
    // if (read (database_path))
    // {
    //     log ("[ERROR]: Cannot read data file\n");
    //     return -1;
    // }
    log ("[INFO]: Start server on port %d...\n", listen_port);
    return run_server();
}