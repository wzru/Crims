#include <stdio.h>

#include "help.h"

char *help_info = "\
usage: ./crims_server.exe [--version] [--help] <command> [<args>]\n\n\
These are crims_server commands:\n\n\
start a server\n\
\tstart [-p port] [-d db_path]\n\n\
work in interactive SQL shell\n\
\tshell\n\n\
directly execute SQL commands\n\
\texec \"some SQL commands;\"\n\n\
test(just for debug)\n\
\ttest\n\n\
";

inline int help (int argc, char *argv[])
{
    printf ("%s\n", help_info);
    return 0;
}