#include <stdio.h>
#include <windows.h>

#include "shell.h"
#include "exec.h"
#include "database.h"

char command_buffer[COMMAND_BUFFER_LENGTH];

inline int shell (int argc, char *argv[])
{
    system ("cls");
    while (1)
    {
        printf("%c ", command_prompt);
        fgets (command_buffer, COMMAND_BUFFER_LENGTH, stdin);
        int command_result = exec (command_buffer);
        if (command_result == SHELL_EXIT)
        {
            if (!is_saved)
            {
                printf ("There are some changes NOT saved!\n");
                printf ("[0]:save and exit [1]:DISCARD changes [2]:cancel\n");
                printf ("Please input:");
                int x;
                scanf ("%d", &x);
                switch (x)
                {
                case 0:
                    printf ("Saving...");
                    int write_result = write (database_path);
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
                printf ("Nothing need to save. Shell EXIT...");
                return 0;
            }
        }
    }
}