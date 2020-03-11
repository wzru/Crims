#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "define.h"
#include "shell.h"
#include "debug.h"
#include "start.h"
#include "help.h"
#include "exec.h"

byte crims_status;

int main (int argc, char *argv[])
{
#ifdef DEBUG
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif
    switch (argc)
    {
    case 0:
        exit (0);
        break;
    case 1:
        crims_status = STATUS_SHELL;
        return shell (argc, argv); //控制权移交shell
        //return test_read_write();
        break;
    default:
        if (!strcmp (argv[1], "help"))
        {
            return help (argc, argv); //帮助信息
        }
        else if (!strcmp (argv[1], "start"))
        {
            crims_status = STATUS_SERVER;
            return start (argc, argv); //启动后台
        }
        else if (!strcmp (argv[1], "exec"))
        {
            crims_status = STATUS_EXEC;
            return argc > 2 ? (read(database_path), exec (argv[2])) : 0;//直接执行SQL
        }
        else if(!strcmp(argv[1], "debug"))
        {
            //return debug();
        }
    }
    return 0;
}