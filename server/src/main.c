#include <stdlib.h>
#include <string.h>

#include "database.h"
#include "define.h"
#include "debug.h"
#include "shell.h"
#include "start.h"
#include "help.h"
#include "exec.h"

byte crims_status;

int main (int argc, char *argv[])
{
#ifdef DEBUG
    setbuf (stdout, NULL);
    setbuf (stderr, NULL);
#endif
    switch (argc)
    {
    case 0:
        exit (0);
        break;
    case 1:
        //crims_status = STATUS_SHELL;
        return help (argc, argv); //默认输出帮助信息
    default:
        if (!stricmp (argv[1], "--help") || !stricmp (argv[1], "help"))
        {
            return help (argc, argv); //打印帮助信息
            return 0;
        }
        else if (!stricmp (argv[1], "--version") || !stricmp (argv[1], "version"))
        {
            printf ("crims_server version %d.%d.%d, compiled at %s, %s\n", MAJOR_VERSION,
                    MINOR_VERSION, PATCH_VERSION,  __TIME__, __DATE__);
            return 0;
        }
        else if (!stricmp (argv[1], "shell"))
        {
            crims_status = STATUS_SHELL;
            return shell (argc, argv); //控制权移交shell
        }
        else if (!stricmp (argv[1], "start"))
        {
            crims_status = STATUS_SERVER;
            return start (argc, argv); //启动后台
        }
        else if (!stricmp (argv[1], "exec"))
        {
            crims_status = STATUS_EXEC;
            return argc > 2 ? (read (database_path), exec (argv[2])) : 0; //直接执行SQL
        }
        else if (!stricmp (argv[1], "test"))
        {
            return test_read_write();//测试读写
        }
    }
    return 0;
}