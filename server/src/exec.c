#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

#include "database.h"
#include "strptime.h"
#include "analyze.h"
#include "server.h"
#include "shell.h"
#include "debug.h"
#include "exec.h"
#include "ast.h"

char single_command[COMMAND_BUFFER_LENGTH];

Record rec;
Records recs;
ExprNode error_expr = {.type = EXPR_ERROR},
         null_expr = {.type = EXPR_NULL},
         lazy_expr = {.type = EXPR_LAZY},
         zero_expr = {.type = EXPR_INTNUM, .intval = 0},
         one_expr = {.type = EXPR_INTNUM, .intval = 1};
uint col_cnt, vcol_cnt, gcol_cnt, ocol_cnt;
char col_name[RECORD_COLUMNS][EXPR_LENGTH];
byte col_leng[RECORD_COLUMNS];
byte is_grpby = 0, is_odrby = 0, is_limit = 0;
LimitNode limit;
ExprNode *vcol[RECORD_COLUMNS], *gcol[RECORD_COLUMNS], *ocol[RECORD_COLUMNS];
byte gsc[RECORD_COLUMNS], osc[RECORD_COLUMNS];
u16 col_prop[RECORD_COLUMNS], vcol_prop[RECORD_COLUMNS],
    ocol_prop[RECORD_COLUMNS];
byte query_status;

clock_t op_start, op_end;

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

/*
    加入一张表table到虚拟表rec中
    返回值表示过程是否出错
*/
inline int append_record_table (TableNode *table, Record *rec)
{
    if (table == NULL || rec == NULL)
    {
        return 0;
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
                return 0;
            }
        }
    else
    {
        write_message ("ERROR(%d): Unknown table type.", -UNKNOWN_TABLE);
        return ERROR;
    }
    write_message ("ERROR(%d): Table NOT exists.", -TABLE_NOT_EXIST);
    return ERROR;
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
            memcpy (rec->ptr[rec->rtb[type]] + isiz[type] * (rec->siz[rec->rtb[type]]),
                    & (ct->ct), isiz[type]);
            ++ (rec->siz[rec->rtb[type]]);
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
        }
        load_data_recursively (status, rec, ct, ci, ro->next, TYPE_ORDER);
        break;
    }
}

/*
    加载具体数据
*/
inline void load_data (int status, Record *rec)
{
    return load_data_recursively (status, rec, head->next, NULL, NULL, TYPE_CAR);
}

/*
    把虚拟表rec中的若干表的数据加载进来, 以数组形式放在rec->arr[]中
*/
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

/*
    用table节点构建虚拟表, 存放在rec
*/
inline byte load_tables (TableNode *table, Record *rec)
{
    while (table != NULL)
    {
        byte res = append_record_table (table, rec);
        if (res == ERROR)
        {
            return res;
        }
        table = table->next;
    }
    load_record (rec);
    return 0;
}

inline void swap_byte (char *a, char *b)
{
    char tmp = *a;
    *a = *b;
    *b = tmp;
}

inline int reverse_int (int *x)
{
    int l = sizeof (int);
    char buf[8];
    memcpy (buf, x, l);
    for (int i = 0; i < l / 2; ++i)
    {
        swap_byte (buf + i, buf + l - i - 1);
    }
    return * (int *) buf;
}

/*
    把rec下标从beg开始到cnt-1的表当前对应的数据加载到rec->item[]区域, 便于之后直接进行运算
*/
inline void load_item (Record *rec, int beg)
{
    for (int j = beg; j < rec->cnt; ++j)
    {
        for (int k = rec->beg[j]; k < rec->beg[j + 1]; ++k)
        {
            ColumnInfo *ci = & (catalog.tbls[rec->tbl[j]].cols[k - rec->beg[j]]);
            u16 type = ci->type;
            void *src = (rec->ptr[j]) + ci->offset;
            switch (type)
            {
            case EXPR_INTNUM:
                memcpy (& (rec->item[k].intval), src,  sizeof (int));
                break;
            case EXPR_APPROXNUM:
                memcpy (& (rec->item[k].floatval), src,  sizeof (float));
                break;
            case EXPR_STRING:
            case EXPR_DATETIME:
                rec->item[k].strval = calloc (1, ci->size + 1);
                memcpy (rec->item[k].strval, src, ci->size);
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
    for (uint i = 0; i < rec->beg[rec->cnt]; ++i)
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
#define can_comp(lhs, rhs) (max(lhs->type, rhs->type)<=EXPR_APPROXNUM || (min(lhs->type, rhs->type)>=EXPR_STRING && max(lhs->type, rhs->type)<=EXPR_DATETIME))
#define get_number(expr) ((expr)->type==EXPR_INTNUM?(expr)->intval:(expr)->floatval)
#define get_logic(expr) ((expr)->intval!=0)

inline time_t make_datetime (char *s)
{
    struct tm tp;
    memset (&tp, 0, sizeof (tp));
    strptime (s, CRIMS_DATETIME_FORMAT, &tp);
    time_t t = mktime (&tp);
    return t;
}

#define DEFINE_BINARY_ARI_OP(name, op) inline ExprNode* MAKE_##name(ExprNode *l, ExprNode *r, Record *rec) {\
        if (l == NULL || r == NULL) return &error_expr; \
        ExprNode *lhs = eval_expr (l, rec), *rhs = eval_expr (r, rec), *res = calloc(1,sizeof(ExprNode));\
        if(is_lazy(lhs) || is_lazy(rhs)) return &lazy_expr;\
        if (can_asmd (lhs) && can_asmd (rhs)){ \
            if (max (lhs->type, rhs->type) == EXPR_APPROXNUM){res->type = EXPR_APPROXNUM; res->floatval = get_number (lhs) op get_number (rhs);}\
            else{ res->type = EXPR_INTNUM;res->intval = lhs->intval op rhs->intval;}}\
        return res;}

#define DEFINE_BINARY_COM_OP(name, op) inline ExprNode* MAKE_##name(ExprNode *l, ExprNode *r, Record *rec) {\
        if(l==NULL || r==NULL) return &error_expr;\
        ExprNode *lhs = eval_expr(l, rec), *rhs = eval_expr(r, rec), *res = calloc(1, sizeof(ExprNode));\
        if(is_lazy(lhs) || is_lazy(rhs)) return &lazy_expr;\
        if(can_comp(lhs, rhs)) { res->type=EXPR_INTNUM;\
            if(max(lhs->type, rhs->type)<=EXPR_APPROXNUM) {  res->intval = get_number(lhs) op get_number(rhs);}\
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
    return MAKE_##type(eval_expr (expr->l, rec), eval_expr (expr->r,rec), rec);
#define reflect_com(type) case type:\
    return MAKE_##type(eval_expr(expr->l, rec), eval_expr(expr->r, rec), rec);

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
        return res;
    }
    return &error_expr;
}

inline ExprNode *is_in_val_list (ExprNode *expr, ExprNode *list, Record *rec)
{
    ExprNode *res = calloc (1, sizeof (ExprNode)), tmp;
    res->type = EXPR_INTNUM;
    res->intval = 1;
    expr = eval_expr (expr, rec);
    if (expr->type == EXPR_ERROR)
    {
        return &error_expr;
    }
    while (list)
    {
        tmp = *eval_expr (list, rec);
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
                if (!stricmp (expr->strval, tmp.strval))
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

inline ExprNode *is_in_select (ExprNode *expr, SelectNode *select, Record *rec)
{
    Record *subrec;
    Records *subrecs;
    ExprNode *res = calloc (1, sizeof (ExprNode));
    res->type = EXPR_INTNUM;
    res->intval = 1;
    if (select->recs == NULL)
    {
        subrec = calloc (1, sizeof (Record));
        subrecs = calloc (1, sizeof (Records));
        clear_records (subrecs);
        do_select (select, subrec, subrecs, 1, 0, 0);
    }
    else
    {
        subrecs = select->recs;
    }
    if (!subrecs->cnt) //如果没找到任何结果
    {
        res->intval = 0;
    }
    else if (subrecs->recs[0].cnt != 1) //如果有多列
    {
        write_message ("Operand should contain 1 column");
        return &error_expr;
    }
    else
    {
        for (uint i = 0; i < subrecs->cnt; ++i)
        {
            res = MAKE_EXPR_EQ (& (subrecs->recs[i].item[0]), expr, rec);
            if (res && res->type == EXPR_INTNUM && res->intval == 1)
            {
                return res;
            }
        }
    }
    res->intval = 0;
    return res;
}

inline int is_error (ExprNode *expr)
{
    return !expr || expr->type == EXPR_ERROR;
}

inline int is_true (ExprNode *expr)
{
    return expr && expr->type == EXPR_INTNUM && expr->intval == 1;
}

inline int is_case_expr (u16 type)
{
    return type == EXPR_CASE_EXPR || type == EXPR_CASE_EXPR_ELSE;
}

inline int is_agr_func (ExprNode *expr)
{
    return expr && expr->type > EXPR_AGR_FUNC_BEGIN
           && expr->type < EXPR_AGR_FUNC_END;
}

inline int is_lazy (ExprNode *expr)
{
    return expr && expr->type == EXPR_LAZY;
}

inline int is_datetime (ExprNode *expr)
{
    return expr && (expr->type == EXPR_STRING || expr->type == EXPR_DATETIME);
}

inline ExprNode *transform_op_expr (ExprNode *grp)
{
    if (grp == NULL)
    {
        return NULL;
    }
    ExprNode *expr = calloc (1, sizeof (ExprNode));
    memcpy (expr, grp, sizeof (ExprNode));
    expr->type = expr->op;
    return expr;
}

inline ExprNode *make_int_expr (int x)
{
    ExprNode *expr = calloc (1, sizeof (ExprNode));
    expr->type = EXPR_INTNUM;
    expr->intval = x;
    return expr;
}

inline ExprNode *make_float_expr (float f)
{
    ExprNode *expr = calloc (1, sizeof (ExprNode));
    expr->type = EXPR_APPROXNUM;
    expr->floatval = f;
    return expr;
}

inline ExprNode *eval_case (ExprNode *case_node, Record *rec)
{
    ExprNode *case_head = case_node->case_head,
              *expr = eval_expr (case_node->l, rec),
               *res;
    u16 type = case_node->type;
    if (is_case_expr (type) && is_error (expr))
    {
        return &error_expr;
    }
    while (case_head)
    {
        ExprNode *l = eval_expr (case_head->l, rec),
                  *r = eval_expr (case_head->r, rec);
        if (is_error (l) || is_error (r))
        {
            return &error_expr;
        }
        res = is_case_expr (type) ? MAKE_EXPR_EQ (expr, l, rec) : l;
        if (is_error (res))
        {
            return &error_expr;
        }
        else if (is_true (res))
        {
            return r;
        }
        case_head = case_head->next;
    }
}

inline ExprNode *eval_func (ExprNode *func, Record *rec)
{
    ExprNode *a0, *a1, *a2, *t1, *t2;
    if (!stricmp (func->strval, "TIMESTAMPDIFF"))
    {
        a0 = func->r;
        if (a0 && a0->next && a0->next->next)
        {
            a1 = transform_op_expr (a0->next), a2 = transform_op_expr (a1->next);
            t1 = eval_expr (a1, rec), t2 = eval_expr (a2, rec);
            if (!is_datetime (t1) || !is_datetime (t2))
            {
                return &error_expr;
            }
            float diff =  make_datetime (t2->strval) - make_datetime (t1->strval);
            if (!stricmp (a0->strval, "DAY"))
            {
                return make_float_expr (diff / 3600 / 24);
            }
            else if (!stricmp (a0->strval, "HOUR"))
            {
                return make_float_expr (diff / 3600);
            }
            else if (!stricmp (a0->strval, "MINUTE"))
            {
                return make_float_expr (diff / 60);
            }
            else if (!stricmp (a0->strval, "SECOND"))
            {
                return make_float_expr (diff);
            }
        }
    }
    write_message ("Error(%d): Invalid function statement %s.",
                   INVALID_FUNCTION_STATEMENT, func->text);
    return &error_expr;
}

inline ExprNode *eval_expr (ExprNode *expr, Record *rec)
{
    if (expr == NULL)
    {
        return &error_expr;
    }
    ExprNode *l, *r, *res = calloc (1, sizeof (ExprNode));
    int flag = 0;
    if (is_lazy (expr->l) || is_lazy (expr->r))
    {
        return &lazy_expr;
    }
    switch (expr->type)
    {
    case EXPR_INTNUM:
    case EXPR_APPROXNUM:
    case EXPR_STRING:
    case EXPR_DATETIME:
    case EXPR_NULL:
    case EXPR_ERROR:
    case EXPR_LAZY:
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
        l = eval_expr (expr->l, rec), r = eval_expr (expr->r, rec);
        if (l->type == EXPR_INTNUM && r->type == EXPR_INTNUM)
        {
            res->type = EXPR_INTNUM;
            res->intval = l->intval % r->intval;
            return res;
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
        return calc_logic (expr->type, eval_expr (expr->l, rec),
                           eval_expr (expr->r, rec));
        reflect_com (EXPR_EQ);
        reflect_com (EXPR_NE);
        reflect_com (EXPR_LT);
        reflect_com (EXPR_GT);
        reflect_com (EXPR_LE);
        reflect_com (EXPR_GE);
    case EXPR_NEG:
        r = eval_expr (expr->r, rec);
        if (r->type == EXPR_INTNUM)
        {
            res->type = EXPR_INTNUM;
            res->intval = - (r->intval);
            return res;
        }
        else
        {
            write_message ("ERROR(%d): There is no matching operator -.",
                           -NO_MATCHING_OPERATOR);
            return &error_expr;
        }
    case EXPR_NOT:
        r = eval_expr (expr->r, rec);
        if (r->type == EXPR_INTNUM)
        {
            res->type = EXPR_INTNUM;
            res->intval = ! (r->intval);
            return res;
        }
        else
        {
            write_message ("ERROR(%d): There is no matching operator ~.",
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
        res = is_in_select (expr->l, expr->select, rec);
        res->intval ^= flag;
        return res;
    case EXPR_FUNC:
        return eval_func (expr, rec);
    case EXPR_COUNT:
    case EXPR_COUNT_ALL:
    case EXPR_SUM:
        if (query_status == QUERY_BEGIN)
        {
            return &lazy_expr;
        }
        else if (query_status == QUERY_REEVAL)
        {
            return & (rec->item[col_cnt + expr->intval]);
        }
    case EXPR_CASE:
    case EXPR_CASE_ELSE:
    case EXPR_CASE_EXPR:
    case EXPR_CASE_EXPR_ELSE:
        return eval_case (expr, rec);
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
    ExprNode *res = eval_expr (cond, rec);
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

inline ExprNode *eval_odrby_expr (ExprNode *expr, Record *rec)
{
    if (expr == NULL)
    {
        return &error_expr;
    }
    else if (expr->type == EXPR_STRING || expr->type == EXPR_NAME)
    {
        return & (rec->item[get_column_index (expr->strval)]);
    }
    else
    {
        return eval_expr (expr, rec);
    }
}

/*
    将符合where条件的记录求列值, 统计进recs
*/
inline int extract_record (ExprNode *column_head, Record *rec, Records *recs)
{
    if (column_head == NULL)
    {
        return 0;
    }
    Record *target = calloc (1, sizeof (Record));
    if (column_head != NULL && column_head->type == SELECT_ALL)
    {
        for (uint i = 0; i < rec->beg[rec->cnt]; ++i)
        {
            append_record_column (& (rec->item[i]), target);
        }
    }
    else
    {
        while (column_head)
        {
            ExprNode *res = eval_expr (column_head, rec);
            if (is_error (res))
            {
                return ERROR;
            }
            else
            {
                append_record_column (res, target);
            }
            column_head = column_head->next;
        }
        if (is_grpby)
        {
            for (uint i = 0; i < vcol_cnt; ++i)
            {
                ExprNode *res = eval_expr (vcol[i], rec);
                if (is_error (res))
                {
                    return ERROR;
                }
                else
                {
                    append_record_column (res, target);
                }
            }
            for (uint i = 0; i < gcol_cnt; ++i)
            {
                ExprNode *res = eval_expr (gcol[i], rec);
                if (is_error (res))
                {
                    return ERROR;
                }
                else
                {
                    append_record_column (res, target);
                }
            }
        }
        if (is_odrby)
        {
            for (uint i = 0; i < ocol_cnt; ++i)
            {
                ExprNode *res = eval_odrby_expr (transform_op_expr (ocol[i]), rec);
                if (is_error (res))
                {
                    return ERROR;
                }
                else
                {
                    append_record_column (res, target);
                }
            }
        }
    }
    add_record (target, recs);
    return 0;
}

inline int get_index_by_table_name (char *table)
{
    for (uint i = 0; i < catalog.tc; ++i)
    {
        if (!stricmp (table, catalog.tbls[i].name))
        {
            return i;
        }
    }
    return -1;
}

/*
    计算顶层select的列数量, 结果写入全局变量col_cnt和col_name[]
*/
inline int calc_col_cnt (TableNode *table_head)
{
    int res = 0;
    while (table_head)
    {
        int tmp = get_index_by_table_name (table_head->table);
        if (tmp == -1)
        {
            write_message ("ERROR(%d): Unknown table name '%s'.", UNKNOWN_TABLE,
                           table_head->table);
            return 0;
        }
        else
        {
            for (uint i = 0; i < catalog.tbls[tmp].cc; ++i)
            {
                strcat (col_name[col_cnt++], catalog.tbls[tmp].cols[i].name);
            }
            //res += catalog.tbls[tmp].cc;
        }
        table_head = table_head->next;
    }
    return res;
}

/*
    加载列名, 用于最后的输出信息, 与求值无关
*/
inline void load_column_names (ExprNode *col, TableNode *tbl)
{
    while (col)
    {
        if (col->type == SELECT_ALL)
        {
            calc_col_cnt (tbl);
            //col_cnt = calc_col_cnt (tbl);
            return;
        }
        else
        {
            if (!strlen (col->text))
            {
                write_message ("No column name can be used.");
            }
            if (col->alias && strlen (col->alias))
            {
                strcat (col_name[col_cnt++], col->alias);
            }
            else
            {
                strcat (col_name[col_cnt++], col->text);
            }
        }
        col = col->next;
    }
}

inline int extract_agr (ExprNode *expr)
{
    if (expr == NULL)
    {
        return 0;
    }
    if (is_agr_func (expr))
    {
        vcol[vcol_cnt] = expr->r;
        vcol_prop[vcol_cnt] = expr->type;
        expr->intval = vcol_cnt++;
        return 1;
    }
    return extract_agr (expr->l) || extract_agr (expr->r);
}

inline void build_agr_col (ExprNode *col)
{
    for (uint i = 0; i < col_cnt; ++i)
    {
        col_prop[i] = extract_agr (col);
        col = col->next;
    }
}

inline void build_grp_col (ExprNode *grp)
{
    while (grp)
    {
        gcol[gcol_cnt] = grp;
        gsc[gcol_cnt] = grp->sc;
        grp = grp->next;
        ++gcol_cnt;
    }
}

inline int get_column_index (char *col)
{
    for (uint i = 0; i < col_cnt; ++i)
        if (!stricmp (col, col_name[i]))
        {
            return i;
        }
    return ERROR;
}

inline int build_odr_col (ExprNode *odr)
{
    while (odr)
    {
        ocol[ocol_cnt] = odr;
        osc[ocol_cnt] = odr->sc;
        if ( (ocol_prop[ocol_cnt] = get_column_index (odr->strval)) == ERROR)
        {
            write_message ("Error(%d): Unknown orderby expression %s.",
                           UNKNOWN_ORDERBY_EXPRESSION, odr->strval);
            return ERROR;
        }
        odr = odr->next;
        ++ocol_cnt;
    }
    return 0;
}

inline int cmp_g (Record *rec1, Record *rec2)
{
    for (uint i = 0; i < gcol_cnt; ++i)
    {
        if (is_true (MAKE_EXPR_NE (& (rec1->item[i + col_cnt + vcol_cnt]),
                                   & (rec2->item[i + col_cnt + vcol_cnt]),
                                   NULL)))
        {
            if (is_true (MAKE_EXPR_LT (& (rec1->item[i + col_cnt + vcol_cnt]),
                                       & (rec2->item[i + col_cnt + vcol_cnt]),
                                       NULL)
                        ) ^ gsc[i])
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
    }
    return 0;
}

inline int cmp_o (Record *rec1, Record *rec2)
{
    for (uint i = 0; i < ocol_cnt; ++i)
    {
        if (is_true (MAKE_EXPR_NE (& (rec1->item[ocol_prop[i]]),
                                   & (rec2->item[ocol_prop[i]]),
                                   NULL)))
        {
            if (is_true (MAKE_EXPR_LT (& (rec1->item[ocol_prop[i]]),
                                       & (rec2->item[ocol_prop[i]]),
                                       NULL)
                        ) ^ osc[i])
            {
                return -1;
            }
            else
            {
                return 1;
            }
        }
    }
    return 0;
}

inline void merge_item_left (ExprNode *lhs, ExprNode *rhs, u16 type,
                             uint beg, uint end)
{
    ExprNode *res = lhs;
    switch (type)
    {
    case EXPR_COUNT_ALL://这个直接不管, 可以特殊处理
    case EXPR_COUNT://同上
        res = make_int_expr (end - beg + 1);
        break;
    case EXPR_SUM:
        res = MAKE_EXPR_ADD (lhs, rhs, NULL);
        break;
    }
    memcpy (lhs, res, sizeof (ExprNode));
}

inline ExprNode *get_col_by_index (ExprNode *col_head, uint id)
{
    for (uint i = 0; i < id; ++i)
    {
        col_head = col_head->next;
    }
    return col_head;
}

inline int merge_agr (Records *recs, ExprNode *col_head)
{
    uint beg = 0, end = 1;
    while (end < recs->cnt)
    {
        int res = cmp_g (& (recs->recs[beg]), & (recs->recs[end]));
        if (res != 0)
        {
            for (uint i = 0; i < vcol_cnt; ++i)
            {
                for (uint j = beg + 1; j < end; ++j)
                {
                    merge_item_left (& (recs->recs[beg].item[i + col_cnt]),
                                     & (recs->recs[j].item[i + col_cnt]),
                                     vcol_prop[i], beg, j);
                }
            }
            for (uint i = 0; i < col_cnt; ++i)
                if (col_prop[i])
                {
                    ExprNode *res = eval_expr (get_col_by_index (col_head, i),
                                               & (recs->recs[beg]));
                    memcpy (& (recs->recs[beg].item[i]), res, sizeof (ExprNode));
                }
            recs->recs[beg].next = end;
            beg = end;
        }
        ++end;
    }
    for (uint i = 0; i < vcol_cnt; ++i)
    {
        for (uint j = beg + 1; j < recs->cnt; ++j)
        {
            merge_item_left (& (recs->recs[beg].item[i + col_cnt]),
                             & (recs->recs[j].item[i + col_cnt]),
                             vcol_prop[i], beg, j);
        }
    }
    for (uint i = 0; i < col_cnt; ++i)
        if (col_prop[i])
        {
            ExprNode *res = eval_expr (get_col_by_index (col_head, i),
                                       & (recs->recs[beg]));
            memcpy (& (recs->recs[beg].item[i]), res, sizeof (ExprNode));
        }
    recs->recs[beg].next = recs->cnt;
    return 0;
}

/*
    select为执行的语句AST节点, rec存放虚拟表的每条查找记录, recs存储查找结果, subq标记这是否是子查询
    返回值表示过程是否出错
*/
inline int do_select (SelectNode *select, Record *rec, Records *recs,
                      byte subq, byte grpby, byte odrby)
{
    if (is_limit = (select->limit != NULL))
    {
        limit = * (select->limit);
    }
    if (load_tables (select->table_head, rec) == ERROR)
    {
        return ERROR;
    }
    if (!subq)
    {
        load_column_names (select->column_head, select->table_head);
    }//如果不是子查询就加载列名, 因为最终输出的列名作用于最顶层的select的
    if (grpby)
    {
        build_agr_col (select->column_head);
        build_grp_col (select->group);
    }//将聚合函数单独找出来
    if (odrby)
    {
        if (build_odr_col (select->order) == ERROR)
        {
            return ERROR;
        }
    }
    do
    {
        int res = judge_cond (select->where, rec);
        if (res == ERROR)
        {
            return res;
        }
        else if (res == 1)
        {
            extract_record (select->column_head, rec, recs);
        }
    }
    while (get_next_record (rec));
    if (grpby)
    {
        query_status = QUERY_REEVAL;
        qsort (recs->recs, recs->cnt, sizeof (Record), cmp_g);
        int res = merge_agr (recs, select->column_head);
        if (res == ERROR)
        {
            return res;
        }
    }
    select->recs = recs;
    return 0;
}

/*
    执行单句SQL
*/
inline int exec_single (char *sql)
{
    op_start = clock();
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
        byte res = do_select (root->select, &rec, &recs, 0,
                              is_grpby = (root->select->group != NULL),
                              is_odrby = (root->select->order != NULL));
        if (res != ERROR && crims_status == STATUS_SHELL)
        {
            print_result (&recs);
        }
        return res;
    }
    else if (root->type == DELETE_STMT)
    {
        // if (check_select (root->select, NULL))
        // {
        //     return STATUS_ERROR;
        // }
        return 0;
    }
    else if (root->type == INSERT_STMT)
    {
        // if (check_select (root->select, NULL))
        // {
        //     return STATUS_ERROR;
        // }
        return 0;
    }
    else if (root->type == UPDATE_STMT)
    {
        // if (check_select (root->select, NULL))
        // {
        //     return STATUS_ERROR;
        // }
        return 0;
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
    for (uint i = 0; i < col_cnt; ++i)
    {
        col_name[i][0] = '\0';
        col_leng[i] = 0;
    }
    is_grpby = 0, is_odrby = 0, is_limit = 0;
    limit.start = 0, limit.count = INT_MAX;
    col_cnt = 0, vcol_cnt = 0, gcol_cnt = 0;
    memset (gsc, 0, sizeof (gsc));
    memset (osc, 0, sizeof (osc));
    memset (ocol_prop, 0, sizeof (ocol_prop));
    query_status = QUERY_BEGIN;
}