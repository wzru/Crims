#ifndef SELECT_H
#define SELECT_H

#include "database.h"
#include "ast.h"

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

typedef struct Records
{
    Record *recs;
    uint cnt, size;
} Records;

#define RECS_INITIAL_LENGTH 128
extern Record rec;
extern Records recs;
extern uint col_cnt, vcol_cnt, gcol_cnt, ocol_cnt;
extern char col_name[RECORD_COLUMNS][EXPR_LENGTH];
extern byte col_leng[RECORD_COLUMNS];
extern byte is_grpby, is_odrby, is_limit;
extern LimitNode limit;
extern ExprNode *vcol[RECORD_COLUMNS], *gcol[RECORD_COLUMNS],
       *ocol[RECORD_COLUMNS];
extern byte gsc[RECORD_COLUMNS], osc[RECORD_COLUMNS];
extern u16 col_prop[RECORD_COLUMNS], vcol_prop[RECORD_COLUMNS],
       ocol_prop[RECORD_COLUMNS];

inline int cmp_o (Record *rec1, Record *rec2);

inline void load_item (Record *rec, int beg);
inline void clear_record (Record *rec);
inline void clear_records (Records *recs);
inline int append_record_table (TableNode *table, Record *rec);
inline void append_record_column (ExprNode *column, Record *rec);
inline int get_next_record (Record *rec);
inline int extract_record (ExprNode *column_head, Record *rec, Records *recs);
inline void add_record (Record *rec, Records *recs);

inline int is_lazy (ExprNode *expr);
inline int get_column_index (char *col);

inline void query_initialize();
inline ExprNode *eval_expr (ExprNode *expr, Record *rec);
inline int do_select (SelectNode *select, Record *rec, Records *recs,
                      byte subq, byte grpby, byte odrby);
inline void print_result (Records *recs);

inline uint ustrlen (const char *ustr);
inline byte calc_length (RecordCell *rc);

#endif