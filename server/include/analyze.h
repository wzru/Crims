#ifndef ANALYZE_H
#define ANALYZE_H

#include "define.h"

typedef struct TableColumnArray
{
    int count;
    char table[TABLE_COLUMN_COUNT][TABLE_NAME_LENGTH],
         column[TABLE_COLUMN_COUNT][COLUMN_NAME_LENGTH];
} TableColumnArray;
extern TableColumnArray table_column_array;

inline int check_select (SelectNode *select, TableColumnArray *p);
inline int check_delete (DeleteNode *delete, TableColumnArray *p);
inline int check_insert (InsertNode *insert, TableColumnArray *p);

#endif