#include <ctype.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

#include "database.h"
#include "exec.h"
#include "shell.h"

char command_buffer[BUFFER_LENGTH], sql[BUFFER_LENGTH];

inline void print_command_prompt()
{
    printf ("%c ", command_prompt);
}

/*
    启动交互式命令行
*/
inline int shell (int argc, char *argv[])
{
    database_initialize();
    system ("chcp 65001");
    while (1)
    {
        print_command_prompt();
        int can_exec = 0, exec_pos = -1;
        memset (command_buffer, 0, sizeof (command_buffer));
        fgets (command_buffer, BUFFER_LENGTH, stdin);
        int l = strlen (command_buffer);
        for (int i = 0; i < l; ++i)
        {
            if (islower (command_buffer[i]))
            {
                // no need
                // command_buffer[i] = toupper (command_buffer[i]);
            }
            else if (command_buffer[i] == '\n')
            {
                command_buffer[i] = ' ';
            }
        }
        for (int i = l - 1; i >= 0; --i)
            if (command_buffer[i] == ';')
            {
                strncat (sql, command_buffer, i + 1);
                can_exec = 1;
                exec_pos = i;
                break;
            }
        if (can_exec)
        {
            // printf ("What will be exec:\"%s\"\n", sql);
            int command_result = exec (sql);
            memset (sql, 0, sizeof (sql));
            if (command_result == SHELL_EXIT)
            {
                if (!is_saved)
                {
                    printf ("There are some changes NOT saved!\n");
                    printf ("[0]:save and exit [1]:DISCARD changes "
                            "[Others]:cancel\n");
                    printf ("Please input:");
                    int x;
                    scanf ("%d", &x);
                    switch (x)
                    {
                    case 0:
                        printf ("Saving...");
                        int write_result = write_db (database_path);
                        if (write_result)
                        {
                            printf ("Saving ERROR!");
                            break;
                        }
                        else
                        {
                            printf ("Saved! Shell EXIT...");
                            return 0;
                        }
                    case 1:
                        printf ("DISCARD changes! Shell EXIT...");
                        return 0;
                    default:
                        break;
                    }
                }
                else
                {
                    printf ("Exerything saved. Shell EXIT...");
                    return 0;
                }
            }
        }
        strcat (sql, command_buffer + exec_pos + 1);
    }
}