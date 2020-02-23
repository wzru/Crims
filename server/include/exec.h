#ifndef EXEC_H
#define EXEC_H

#include "define.h"

typedef struct TableColumnArray
{
    int count;
    char table[TABLE_COLUMN_COUNT][TABLE_NAME_LENGTH],
         column[TABLE_COLUMN_COUNT][COLUMN_NAME_LENGTH];
} TableColumnArray;
extern TableColumnArray table_column_array;

inline int exec (char *command);
inline int write_message(char *s, ...);

#endif