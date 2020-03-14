#ifndef EXEC_H
#define EXEC_H

#include <time.h>

#include "define.h"
#include "ast.h"

extern byte crims_table_column_count[DATABASE_TABLE_COUNT];
//char crims_table_name[DATABASE_TABLE_COUNT][TABLE_NAME_LENGTH];

#define RECORD_COLUMNS 30
typedef ExprNode RecordCell;
typedef struct Record
{
    RecordCell item[RECORD_COLUMNS];
    char *table[RECORD_COLUMNS],//表名
         *name[RECORD_COLUMNS],//列名
         *alias[RECORD_COLUMNS];//表名的别名
    byte cnt;//表数量
    byte beg[DATABASE_TABLE_COUNT + 1],//第i张表在item中的起始下标
         tbl[DATABASE_TABLE_COUNT],//tbl[i]:第i张表是几号表(ct,ci,ro)
         rtb[DATABASE_TABLE_COUNT];//rtb[i]:i号表对应当前结构体中tbl的第几张表
    void *ptr[DATABASE_TABLE_COUNT];
    int siz[DATABASE_TABLE_COUNT];
    void *arr[DATABASE_TABLE_COUNT];
    uint next;
} Record;

extern ExprNode error_expr, null_expr, zero_expr, one_expr;

typedef struct Records
{
    Record *recs;
    uint cnt, size;
} Records;

enum
{
    QUERY_BEGIN,
    QUERY_REEVAL
};

#define RECS_INITIAL_LENGTH 128
extern Record rec;
extern Records recs;
extern uint col_cnt, vcol_cnt, gcol_cnt;
extern char col_name[RECORD_COLUMNS][EXPR_LENGTH];
extern byte col_leng[RECORD_COLUMNS];
extern byte is_grpby;
extern ExprNode *vcol[RECORD_COLUMNS], *gcol[RECORD_COLUMNS];
extern byte sc[RECORD_COLUMNS];
extern u16 col_prop[RECORD_COLUMNS], vcol_prop[RECORD_COLUMNS];
extern byte query_status;

extern clock_t op_start, op_end;

inline int exec (char *command);
inline int write_message (char *s, ...);

inline void load_item (Record *rec, int beg);
inline void clear_record (Record *rec);
inline void clear_records (Records *recs);
inline int append_record_table (TableNode *table, Record *rec);
inline void append_record_column (ExprNode *column, Record *rec);
inline int get_next_record (Record *rec);
inline int extract_record (ExprNode *column_head, Record *rec, Records *recs);
inline void add_record (Record *rec, Records *recs);
inline void query_initialize();
inline ExprNode *evaluate_expr (ExprNode *expr, Record *rec);
inline int do_select (SelectNode *select, Record *rec, Records *recs,
                      byte subq, byte grpby);

#endif