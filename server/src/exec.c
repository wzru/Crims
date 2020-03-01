#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include "database.h"
#include "server.h"
#include "shell.h"
#include "debug.h"
#include "exec.h"
#include "ast.h"

char single_command[COMMAND_BUFFER_LENGTH];
Record rec;
Records recs;
ExprNode error_expr = {EXPR_ERROR};
uint col_cnt;
char col_name[RECORD_COLUMNS][EXPR_LENGTH];
//CarType dbgct;

inline int write_message (char *s, ...)
{
    va_list ap;
    va_start (ap, s);
    switch (crims_status)
    {
    case STATUS_SHELL:
        vfprintf (stderr, s, ap);
        fprintf (stderr, "\n");
        break;
    }
}

TableColumnArray table_column_array;

inline void append_table_column_array (TableColumnArray *p, char *table,
                                       char *column)
{
    memcpy (p->table[p->count], table, strlen (table));
    memcpy (p->column[p->count], column, strlen (column));
    ++ (p->count);
}

inline int check_table (char *table, TableColumnArray *p)
{
    for (int i = 0; i < catalog.tc; ++i)
        if (!strcmp (catalog.tbls[i].name, table))
        {
            for (int j = 0; j < catalog.tbls[i].cc; ++j)
            {
                append_table_column_array (p, table, catalog.tbls[i].cols[j].name);
            }
            return 0;
        }
    write_message ("ERROR: Unknown table reference '%s'", table);
    return 1;
}

inline int check_table_column (char *table, char *column, TableColumnArray *p)
{
    if (p != NULL)
        for (int i = 0; i < p->count; ++i)
        {
            if (!strcmp (p->table, table) && !strcmp (p->column, column))
            {
                return 0;
            }
        }
    write_message ("ERROR: Unknown column reference '%s.%s'", table, column);
    return 1;
}

inline int check_val_list (ExprNode *expr, TableColumnArray *p)
{
    int res = 0, tmp;
    for (ExprNode *ep = expr; ep; ep = ep->next)
    {
        tmp = ep->op, ep->op = ep->type, ep->type = tmp;
        res = check_expr (ep, p);
        tmp = ep->op, ep->op = ep->type, ep->type = tmp;
        if (res)
        {
            return res;
        }
    }
    return res;
}

inline int check_expr (ExprNode *expr, TableColumnArray *p)
{
    if (expr == NULL)
    {
        return 0;
    }
    int res = 0, tmp;
    switch (expr->type)
    {
    case EXPR_NAME:
        return check_table (expr->strval, p);
        break;
    case EXPR_TABLE_COLUMN:
        return check_columns_tables (expr->table, expr->strval, p);
        break;
    case EXPR_STRING:
    case EXPR_INTNUM:
    case EXPR_APPROXNUM:
    case EXPR_DATETIME:
        return 0;
    case EXPR_ADD:
    case EXPR_SUB:
    case EXPR_MUL:
    case EXPR_DIV:
    case EXPR_MOD:
    case EXPR_AND:
    case EXPR_OR:
    case EXPR_XOR:
    case EXPR_EQ:
    case EXPR_NE:
    case EXPR_LT:
    case EXPR_GT:
    case EXPR_LE:
    case EXPR_GE:
    case EXPR_IN_VAL_LIST:
    case EXPR_NOT_IN_VAL_LIST:
    case EXPR_LIKE:
    case EXPR_NOT_LIKE:
        return check_expr (expr->l, p) || check_expr (expr->r, p);
    case EXPR_NEG:
    case EXPR_NOT:
        return check_expr (expr->r, p);
    case EXPR_SELECT:
        return check_select (expr->select, p);
    case EXPR_IN_SELECT:
    case EXPR_NOT_IN_SELECT:
        return check_expr (expr->l, p) || check_select (expr->select, p);
    case EXPR_VAL_LIST:
        return check_val_list (expr, p);
    case EXPR_FUNC:
        //TODO
        return check_val_list (expr->r, p);
    case EXPR_COUNT_ALL:
    case EXPR_SUM_ALL:
        return 0;
    case EXPR_COUNT:
    case EXPR_SUM:
        return check_val_list (expr->r, p);
    case EXPR_CASE_EXPR:
    case EXPR_CASE_EXPR_ELSE:
    case EXPR_CASE:
    case EXPR_CASE_ELSE:
        return check_expr (expr->l, p) || check_expr (expr->r, p)
               || check_expr (expr->case_head, p);
    case EXPR_CASE_NODE:
        return check_expr (expr->l, p) || check_expr (expr->r, p)
               || check_expr (expr->next, p);
    case ORDERBY:
    case GROUPBY:
        tmp = expr->op, expr->op = expr->type, expr->type = tmp;
        res = check_expr (expr, p);
        tmp = expr->op, expr->op = expr->type, expr->type = tmp;
        return res;
    default:
        return 1;
    }
}

inline int check_columns_tables (ExprNode *column_head, TableNode *table_head,
                                 TableColumnArray *table_column_array)
{
    if (column_head == NULL || table_head == NULL)
    {
        return 0;
    }
    for (TableNode *p = table_head; p; p = p->next)
    {
        //首先检查表是否合法
        int res = 0;
        if (p->type == TABLE_DEFAULT)
        {
            res = check_table (p->table, table_column_array);
        }
        else if (p->type == TABLE_SUBQUERY)
        {
            res = check_select (p->select, table_column_array);
        }
        if (res)
        {
            return res;
        }
    }
    if (column_head->type != SELECT_ALL)
        for (ExprNode *p = column_head; p; p = p->next)
        {
            //检查列是否合法
            int res = check_expr (p, table_column_array);
            if (res)
            {
                return res;
            }
        }
    return 0;
}

inline int check_select (SelectNode *select, TableColumnArray *p)
{
    if (select == NULL)
    {
        return 0;
    }
    if (p == NULL) //是否为子查询
    {
        table_column_array.count = 0;
        p = &table_column_array;
    }
    int res = check_columns_tables (select->table_head, select->column_head, p);
    if (res)
    {
        return res;
    }
    return check_expr (select->where, p) || check_expr (select->group, p)
           || check_expr (select->order, p);
}

inline int check_delete (DeleteNode *delete, TableColumnArray *p)
{
    if (delete == NULL)
    {
        return 0;
    }
    if (p != NULL)
    {
        p->count = 0;
    }
    int res = check_table (delete->table, p);
    if (res)
    {
        return res;
    }
    return check_expr (delete->where, p);
}

inline int check_insert (InsertNode *insert, TableColumnArray *p)
{
    if (insert == NULL)
    {
        return 0;
    }
}

inline void append_record_table (TableNode *table, Record *rec)
{
    if (table == NULL || rec == NULL)
    {
        return;
    }
    if (table->type == TABLE_DEFAULT)
        for (int i = 0; i < DATABASE_TABLE_COUNT; ++i)
        {
            if (!stricmp (table->table, catalog.tbls[i].name))
            {
                for (int j = 0, k = rec->beg[rec->cnt]; j < catalog.tbls[i].cc; ++j)
                {
                    rec->item[j + k].type = catalog.tbls[i].cols[j].type;
                    //rec->item[j + k].alias = strdup (catalog.tbls[i].cols[j].name);
                    rec->table[j + k] = strdup (catalog.tbls[i].name);
                    rec->name[j + k] = strdup (catalog.tbls[i].cols[j].name);
                    rec->alias[j + k] = strdup (table->alias);
                }
                rec->tbl[rec->cnt] = i;
                rec->rtb[i] = rec->cnt;
                rec->beg[rec->cnt + 1] = rec->beg[rec->cnt] + catalog.tbls[i].cc;
                ++rec->cnt;
                return;
            }
        }
    else
    {
        write_message ("ERROR(%d): Unknown table type.", -UNKNOWN_TABLE);
        return;
    }
    write_message ("ERROR(%d): Table NOT exists.", -TABLE_NOT_EXIST);
}

#define move_data(tp)  \
    memcpy (rec->ptr[rec->rtb[type]] + isiz[type] * (rec->siz[rec->rtb[type]]), & (tp->tp), isiz[type]); \
    ++(rec->siz[rec->rtb[type]]);

#define check_need(status, type) (((status)>>type)&1)

inline void load_data_recursively (int status, Record *rec, CarTypeNode *ct,
                                   CarInfoNode *ci, RentOrderNode *ro, int type)
{
    switch (type)
    {
    case TYPE_CAR:
        if (ct == NULL)
        {
            return;
        }
        if (check_need (status, TYPE_CAR))
        {
            //memcpy(&dbgct, &(ct->ct), isiz[type]);
            memcpy (rec->ptr[rec->rtb[type]] + isiz[type] * (rec->siz[rec->rtb[type]]),
                    & (ct->ct), isiz[type]);
            ++ (rec->siz[rec->rtb[type]]);
            //move_data (ct);
        }
        load_data_recursively (status, rec, ct, ct->head->next, ro, TYPE_INFO);
        break;
    case TYPE_INFO:
        if (ci == NULL)
        {
            return load_data_recursively (status, rec, ct->next, NULL, NULL, TYPE_CAR);
        }
        if (check_need (status, TYPE_INFO))
        {
            memcpy (rec->ptr[rec->rtb[type]] + isiz[type] * (rec->siz[rec->rtb[type]]),
                    & (ci->ci), isiz[type]);
            ++ (rec->siz[rec->rtb[type]]);
            //move_data (ci);
        }
        load_data_recursively (status, rec, ct, ci, ci->head->next, TYPE_ORDER);
        break;
    case TYPE_ORDER:
        if (ro == NULL)
        {
            return load_data_recursively (status, rec, ct, ci->next, NULL, TYPE_INFO);
        }
        if (check_need (status, TYPE_ORDER))
        {
            memcpy (rec->ptr[rec->rtb[type]] + isiz[type] * (rec->siz[rec->rtb[type]]),
                    & (ro->ro), isiz[type]);
            ++ (rec->siz[rec->rtb[type]]);
            //move_data (ro);
        }
        load_data_recursively (status, rec, ct, ci, ro->next, TYPE_ORDER);
        break;
    }
}

inline void load_data (int status, Record *rec)
{
    return load_data_recursively (status, rec, head->next, NULL, NULL, TYPE_CAR);
}

inline void load_record (Record *rec)
{
    if (rec == NULL)
    {
        return;
    }
    int status = 0;
    for (int i = 0; i < rec->cnt; ++i)
    {
        //rec->siz[i] = isiz[rec->tbl[i]];
        rec->siz[i] = 0;
        rec->ptr[i] = rec->arr[i] = malloc (isiz[rec->tbl[i]] * icnt[rec->tbl[i]]);
        status |= (1 << rec->tbl[i]);
    }
    load_data (status, rec);
    load_item (rec, 0);
}

inline void  load_tables (TableNode *table_head, Record *rec)
{
    while (table_head != NULL)
    {
        append_record_table (table_head, rec);
        table_head = table_head->next;
    }
    load_record (rec);
}

inline void swap_byte(char *a, char *b)
{
    char tmp = *a;
    *a=*b;
    *b=tmp;
}

inline int reverse_int(int* x)
{
    int l = sizeof(int);
    char buf[8];
    memcpy(buf, x, l);
    for(int i=0; i<l/2; ++i)
        swap_byte(buf+i, buf+l-i-1);
    return *(int *)buf;
}

inline void load_item (Record *rec, int beg)
{
    //CarType *ct;
    for (int j = beg; j < rec->cnt; ++j)
    {
        for (int k = rec->beg[j]; k < rec->beg[j + 1]; ++k)
        {
            ColumnInfo *ci = &(catalog.tbls[rec->tbl[j]].cols[k - rec->beg[j]]);
            u16 type = ci->type;
            void *src = (rec->ptr[j]) + ci->offset;
            switch (type)
            {
            case EXPR_INTNUM:
                //ct = (rec->ptr[j]);
                rec->item[k].intval = reverse_int(src);
                //memcpy (& (rec->item[k].intval), src,  sizeof (int));
                break;
            case EXPR_APPROXNUM:
                memcpy (& (rec->item[k].floatval), src,  sizeof (float));
                break;
            case EXPR_STRING:
            case EXPR_DATETIME:
                //rec->item[k].strval = strdup((char *)src);
                rec->item[k].strval = calloc(1, ci->size + 1);
                memcpy(rec->item[k].strval, src, ci->size);
                break;
            }
        }
    }
}

inline int get_next_record (Record *rec)
{
    int i;
    for (i = rec->cnt - 1; i >= 0; --i)
    {
        if (rec->ptr[i] < rec->arr[i] + isiz[rec->tbl[i]] * (rec->siz[i] - 1))
        {
            break;
        }
    }
    if (i < 0)
    {
        return 0;
    }
    else
    {
        int j = i;
        rec->ptr[i] += isiz[rec->tbl[i]];
        for (++i; i < rec->cnt; ++i)
        {
            rec->ptr[i] = rec->arr[i];
        }
        load_item (rec, j);
        return 1;
    }
}

inline int get_index_by_name (char *name, Record *rec)
{
    int cnt = 0, res = -1;
    for (uint i = 0; i < rec->beg[rec->cnt]; ++i)
    {
        if (!stricmp (name, rec->name[i]))
        {
            res = i;
            ++cnt;
        }
    }
    if (!cnt)
    {
        write_message ("ERROR(%d): Unknown column '%s'.", -UNKNOWN_TABLE, name);
        return UNKNOWN_COLUMN;
    }
    else if (cnt > 1)
    {
        write_message ("ERROR(%d): Ambiguous column '%s'.", -AMBIGUOUS_COLUMN, name);
        return AMBIGUOUS_COLUMN;
    }
    else
    {
        return res;
    }
}

inline int get_index_by_table_column (char *table, char *column, Record *rec)
{
    int cnt = 0, res = -1;
    for (uint i = 0; i < rec->cnt; ++i)
    {
        if (!stricmp (column, rec->name[i]) && (!stricmp (table, rec->table[i])
                                                || !stricmp (table, rec->alias[i])))
        {
            res = i;
            ++cnt;
        }
    }
    if (!cnt)
    {
        write_message ("ERROR(%d): Unknown column '%s.%s'.", -UNKNOWN_TABLE, table,
                       column);
        return UNKNOWN_COLUMN;
    }
    else if (cnt > 1)
    {
        write_message ("ERROR(%d): Ambiguous column '%s.%s'.", -AMBIGUOUS_COLUMN, table,
                       column);
        return AMBIGUOUS_COLUMN;
    }
    else
    {
        return res;
    }
}

inline ExprNode *make_expr_by_name (char *name, Record *rec)
{
    int id = get_index_by_name (name, rec);
    if (id >= 0)
    {
        return & (rec->item[id]);
    }
    else
    {
        return &error_expr;
    }
}

inline ExprNode *make_expr_by_table_column (char *table, char *col, Record *rec)
{
    int id = get_index_by_table_column (table, col, rec);
    if (id >= 0)
    {
        return & (rec->item[id]);
    }
    else
    {
        return &error_expr;
    }
}

#define can_asmd(expr) ((expr)->type==EXPR_INTNUM || (expr)->type==EXPR_APPROXNUM)
#define can_comp(lhs, rhs) (max(lhs->type, rhs->type)==EXPR_APPROXNUM || (min(lhs->type, rhs->type)==EXPR_STRING && max(lhs->type, rhs->type)==EXPR_DATETIME))
#define get_number(expr) ((expr)->type==EXPR_INTNUM?(expr)->intval:(expr)->floatval)
#define get_logic(expr) ((expr)->intval!=0)

inline time_t make_datetime (char *s)
{
    struct tm tp;
    strptime (s, CRIMS_DATETIME_FORMAT, &tp);
    return mktime (&tp);
}

#define DEFINE_BINARY_ARI_OP(name, op) inline ExprNode* MAKE_##name(ExprNode *l, ExprNode *r, Record *rec) {\
        if (l == NULL || r == NULL) return &error_expr; \
        ExprNode *lhs = evaluate_expr (l, rec), *rhs = evaluate_expr (r, rec), *res = calloc(1,sizeof(ExprNode));\
        if (can_asmd (lhs) && can_asmd (rhs)){ \
            if (max (lhs->type, rhs->type) == EXPR_APPROXNUM){res->type = EXPR_APPROXNUM; res->floatval = get_number (lhs) + get_number (rhs);}\
            else{ res->type = EXPR_INTNUM;res->intval = lhs->intval op rhs->intval;}}\
        return res;}

#define DEFINE_BINARY_COM_OP(name, op) inline ExprNode* MAKE_##name(ExprNode *l, ExprNode *r, Record *rec) {\
        if(l==NULL || r==NULL) return &error_expr;\
        ExprNode *lhs = evaluate_expr(l, rec), *rhs = evaluate_expr(r, rec), *res = calloc(1, sizeof(ExprNode));\
        if(can_comp(lhs, rhs)) { res->type=EXPR_INTNUM;\
            if(max(lhs->type, rhs->type)==EXPR_APPROXNUM) {  res->intval = get_number(lhs) op get_number(rhs);}\
            else if(lhs->type==EXPR_STRING && rhs->type==EXPR_STRING){ res->intval = strcmp(lhs->strval, rhs->strval) op 0; }\
            else res->intval = make_datetime(lhs->strval) op make_datetime( rhs->strval);}\
        return res;}

DEFINE_BINARY_ARI_OP (EXPR_ADD, +)
DEFINE_BINARY_ARI_OP (EXPR_SUB, -)
DEFINE_BINARY_ARI_OP (EXPR_MUL, *)
DEFINE_BINARY_ARI_OP (EXPR_DIV, /)

DEFINE_BINARY_COM_OP (EXPR_EQ, ==)
DEFINE_BINARY_COM_OP (EXPR_NE, !=)
DEFINE_BINARY_COM_OP (EXPR_LT, <)
DEFINE_BINARY_COM_OP (EXPR_GT, >)
DEFINE_BINARY_COM_OP (EXPR_LE, <=)
DEFINE_BINARY_COM_OP (EXPR_GE, >=)

#define reflect_ari(type) case type:\
    return MAKE_##type(evaluate_expr (expr->l, rec), evaluate_expr (expr->r,rec), rec);
#define reflect_com(type) case type:\
    return MAKE_##type(evaluate_expr(expr->l, rec), evaluate_expr(expr->r, rec), rec);

inline ExprNode *calc_logic (uint type, ExprNode *l, ExprNode *r)
{
    if (l == NULL || r == NULL)
    {
        return &error_expr;
    }
    if (l->type == EXPR_INTNUM && r->type == EXPR_INTNUM)
    {
        ExprNode *res = calloc (1, sizeof (ExprNode));
        res->type = EXPR_INTNUM;
        if (type == EXPR_AND)
        {
            res->intval = get_logic (l) && get_logic (r);
        }
        else if (type == EXPR_OR)
        {
            res->intval = get_logic (l) || get_logic (r);
        }
        else if (type == EXPR_XOR)
        {
            res->intval = get_logic (l) ^ get_logic (r);
        }
    }
    return &error_expr;
}

inline ExprNode *is_in_val_list (ExprNode *expr, ExprNode *list, Record *rec)
{
    ExprNode *res = calloc (1, sizeof (ExprNode)), tmp;
    res->type = EXPR_INTNUM;
    res->intval = 1;
    expr = evaluate_expr (expr, rec);
    if (expr->type == EXPR_ERROR)
    {
        return &error_expr;
    }
    while (list)
    {
        tmp = *evaluate_expr (list, rec);
        if (tmp.type == EXPR_ERROR)
        {
            return &error_expr;
        }
        if (tmp.type == expr->type)
            switch (expr->type)
            {
            case EXPR_INTNUM:
                if (expr->intval == tmp.intval)
                {
                    return res;
                }
                break;
            case EXPR_APPROXNUM:
                if (expr->floatval == tmp.floatval)
                {
                    return res;
                }
                break;
            case EXPR_STRING:
                if (!strcmp (expr->strval, tmp.strval))
                {
                    return res;
                }
                break;
            case EXPR_DATETIME:
                if (make_datetime (expr->strval) == make_datetime (tmp.strval))
                {
                    return res;
                }
                break;
            }
        list = list->next;
    }
    res->intval = 0;
    return res;
}

inline ExprNode *is_in_select (ExprNode *expr, SelectNode *select)
{
    Record *rec;
    Records *recs;
    ExprNode *res = calloc (1, sizeof (ExprNode));
    res->type = EXPR_INTNUM;
    res->intval = 1;
    if (select->recs == NULL)
    {
        rec = calloc (1, sizeof (Record));
        recs = calloc (1, sizeof (Records));
        clear_records (recs);
        do_select (select, rec, recs, 1);
    }
    else
    {
        recs = select->recs;
    }
    if (!recs->cnt) //如果没找到任何结果
    {
        res->intval = 0;
    }
    else if (recs->recs[0].cnt != 1) //如果有多列
    {
        write_message ("Operand should contain 1 column");
        return &error_expr;
    }
    else
    {
        for (uint i = 0; i < recs->cnt; ++i)
        {
            if (MAKE_EXPR_EQ (& (recs->recs[i].item), expr, rec))
            {
                return res;
            }
        }
    }
    res->intval = 0;
    return res;
}

inline ExprNode *evaluate_expr (ExprNode *expr, Record *rec)
{
    if (expr == NULL)
    {
        return &error_expr;
    }
    ExprNode *l, *r, *res = calloc (1, sizeof (ExprNode));
    int flag = 0;
    switch (expr->type)
    {
    case EXPR_STRING:
    case EXPR_INTNUM:
    case EXPR_APPROXNUM:
    case EXPR_ERROR:
        return expr;
    case EXPR_NAME:
        return make_expr_by_name (expr->strval, rec);
    case EXPR_TABLE_COLUMN:
        return make_expr_by_table_column (expr->table, expr->strval, rec);
        reflect_ari (EXPR_ADD);
        reflect_ari (EXPR_SUB);
        reflect_ari (EXPR_MUL);
        reflect_ari (EXPR_DIV);
    case EXPR_MOD:
        l = evaluate_expr (expr->l, rec), r = evaluate_expr (expr->r, rec);
        if (l->type == EXPR_INTNUM && r->type == EXPR_INTNUM)
        {
            return l->intval % r->intval;
        }
        else
        {
            write_message ("ERROR(%d): There is no matching operator \%.",
                           -NO_MATCHING_OPERATOR);
            return &error_expr;
        }
    case EXPR_AND:
    case EXPR_OR:
    case EXPR_XOR:
        return calc_logic (expr->type, evaluate_expr (expr->l, rec),
                           evaluate_expr (expr->r, rec));
        reflect_com (EXPR_EQ);
        reflect_com (EXPR_NE);
        reflect_com (EXPR_LT);
        reflect_com (EXPR_GT);
        reflect_com (EXPR_LE);
        reflect_com (EXPR_GE);
    case EXPR_NEG:
        r = evaluate_expr (expr->r, r);
        if (r->type == EXPR_INTNUM)
        {
            res->type = EXPR_INTNUM;
            res->intval = - (r->intval);
            return res;
        }
        else
        {
            write_message ("ERROR(%d): There is no matching operator \%.",
                           -NO_MATCHING_OPERATOR);
            return &error_expr;
        }
    case EXPR_NOT:
        r = evaluate_expr (expr->r, r);
        if (r->type == EXPR_INTNUM)
        {
            res->type = EXPR_INTNUM;
            res->intval = ! (r->intval);
            return res;
        }
        else
        {
            write_message ("ERROR(%d): There is no matching operator \%.",
                           -NO_MATCHING_OPERATOR);
            return &error_expr;
        }
    case EXPR_NOT_IN_VAL_LIST:
        flag = 1;
    case EXPR_IN_VAL_LIST:
        res = is_in_val_list (expr->l, expr->r, rec);
        res->intval ^= flag;
        return res;
    //TODO:LIKE
    case EXPR_NOT_IN_SELECT:
        flag = 1;
    case EXPR_IN_SELECT:
        res = is_in_select (expr->l, expr->select);
        res->intval ^= flag;
        return res;
    default:
        write_message ("ERROR(%d): Unknown expression.", -UNKNOWN_EXPRESSION);
        return &error_expr;
    }
}

inline int judge_cond (ExprNode *cond, Record *rec)
{
    if (cond == NULL)
    {
        return 1;
    }
    ExprNode *res = evaluate_expr (cond, rec);
    if (res->type == EXPR_ERROR)
    {
        return ERROR;
    }
    else
    {
        return res->intval;
    }
}

inline void append_record_column (ExprNode *val, Record *rec)
{
    memcpy (&rec->item[ (rec->cnt)++], val, sizeof (ExprNode));
}

inline int extract_record (ExprNode *column_head, Record *rec, Record *recs)
{
    if (column_head == NULL)
    {
        return 0;
    }
    Record *target = calloc (1, sizeof (Record));
    for (uint i = 0; i < col_cnt; ++i)
    {
        ExprNode *res = evaluate_expr (column_head, rec);
        if (res == NULL || res->type == EXPR_ERROR)
        {
            return ERROR;
        }
        else
        {
            append_record_column (res, target);
        }
        column_head = column_head->next;
    }
    add_record (target, recs);
}

inline void load_column_names (ExprNode *col)
{
    while (col)
    {
        if (!strlen (col->text))
        {
            write_message ("No column name can be used.");
        }
        strcat (col_name[col_cnt++], col->text);
        col = col->next;
    }
}

inline void do_select (SelectNode *select, Record *rec, Records *recs,
                       byte subq)
{
    load_tables (select->table_head, rec);
    if (!subq)
    {
        load_column_names (select->column_head);
    }
    do
    {
        if (judge_cond (select->where, rec))
        {
            extract_record (select->column_head, rec, recs);
            //add_record (rec, recs);
        }
    }
    while (get_next_record (rec));
    select->recs = recs;
}

inline int exec_single (char *sql)
{
    query_initialize();
    SqlAst *root = parse_sql (sql);
    //print_ast(root, 0);
    if (root == NULL)
    {
        return STATUS_ERROR;
    }
    else if (root->type == SELECT_STMT)
    {
        // if (check_select (root->select, NULL))
        // {
        //     return STATUS_ERROR;
        // }
        do_select (root->select, &rec, &recs, 0);
        if (crims_status == STATUS_SHELL)
        {
            print_result (&recs);
        }
    }
    else if (root->type == DELETE_STMT)
    {
        if (check_select (root->select, NULL))
        {
            return STATUS_ERROR;
        }
    }
    else if (root->type == INSERT_STMT)
    {
        if (check_select (root->select, NULL))
        {
            return STATUS_ERROR;
        }
    }
    else if (root->type == UPDATE_STMT)
    {
        if (check_select (root->select, NULL))
        {
            return STATUS_ERROR;
        }
    }
    else
    {
        return STATUS_UNKNOWN;
    }
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

inline void clear_record (Record *rec)
{
    rec->cnt = 0;
    for (int i = 0; i < DATABASE_TABLE_COUNT; ++i)
    {
        free (rec->arr[i]);
        rec->siz[i] = 0;
    }
}

inline void clear_records (Records *recs)
{
    if (recs->size != 0)
    {
        for (int i = 0; i < recs->cnt; ++i)
        {
            clear_record (recs->recs + i);
        }
    }
    else
    {
        recs->size = RECS_INITIAL_LENGTH;
        recs->recs = malloc (RECS_INITIAL_LENGTH * sizeof (Record));
    }
    recs->cnt = 0;
}

inline void add_record (Record *rec, Records *recs)
{
    if (recs->cnt + 1 >= recs->size)
    {
        recs->size <<= 1;
        recs->recs = realloc (recs, recs->size * sizeof (Record));
    }
    memcpy (recs->recs + recs->cnt, rec, sizeof (Record));
    ++recs->cnt;
}

inline void query_initialize()
{
    clear_record (&rec);
    clear_records (&recs);
    col_cnt = 0;
}