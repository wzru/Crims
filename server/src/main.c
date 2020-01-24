#include <stdlib.h>
#include <string.h>

#include "define.h"
#include "shell.h"
#include "start.h"
#include "help.h"
#include "exec.h"

int main (int argc, char *argv[])
{
    switch (argc)
    {
    case 0:
        exit (0);
        break;
    case 1:
        return shell (argc, argv); //控制权移交shell
        break;
    default:
        if (!strcmp (argv[1], "help"))
        {
            return help (argc, argv); //帮助信息
        }
        else if (!strcmp (argv[1], "start"))
        {
            return start (argc, argv); //启动后台
        }
        else if (!strcmp (argv[1], "exec"))
        {
            return argc > 2 ? exec (argv[2]) : 0;
        }
    }
    return 0;
}