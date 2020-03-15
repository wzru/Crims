#include <stdio.h>

#include "help.h"

char main_help[MAIN_HELP_LENGTH] = "I'm HELP!";

inline int help (int argc, char *argv[])
{
    printf ("%s\n", main_help);
    return 0;
}