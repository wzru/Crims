#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
    #include "getopt.h"
#else
    #include "getopt.h"
    //#include <getopt.h>
    //#include <unistd.h>
#endif

#include "database.h"
#include "debug.h"
#include "log.h"
#include "server.h"

inline int start (int argc, char *argv[])
{
    system ("chcp 65001");
    int opt = 0;
    while ( (opt = getopt (argc, argv, "p:d:")) != -1) //解析参数
    {
        switch (opt)
        {
        case 'p':
            sscanf (optarg, "%d", &listen_port);
            // printf ("%d\n", listen_port);
            break;
        case 'd':
            sscanf (optarg, "%s", database_path);
            break; //可能对带空格的路径支持有问题, 需要加引号
        }
    }
    plog ("[INFO]: Reading data from '%s'...\n", database_path);
    //多线程处理, IO不等待
    pthread_t read_db_thread;
    pthread_create (&read_db_thread, NULL, (pf) read_db, database_path);
    initialize_log();
    pthread_join (read_db_thread, NULL);
    // if (read_db (database_path))
    // {
    //     plog ("[ERROR]: Cannot read data file\n");
    //     return -1;
    // }
    plog ("[INFO]: Start server on port %d...\n", listen_port);
    return run_server();
}