#include <string.h>

#include "database.h"
#include "shell.h"
#include "exec.h"
#include "ast.h"

char single_command[COMMAND_BUFFER_LENGTH];

inline int exec_single(char *sql)
{
    SqlAst *root;
    root = parse_sql (sql);
    print_ast(root, 0);
}

inline int exec (char *command)
{
    int l = strlen (command), start = 0;
    int res = 0;
    for (int i = 0; i < l; ++i)
    {
        if (command[i] == ';')
        {
            strncpy (single_command, command + start, i - start + 1);
            start = i + 1;
            single_command[start] = '\0';
            res = exec_single (single_command);
        }
    }

    return res;
}