#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"
#include "exec.h"
#include "log.h"
#include "regex.h"
#include "select.h"
#include "server.h"
#include "strptime.h"

Record rec;
Records recs;
uint col_cnt, vcol_cnt, gcol_cnt, ocol_cnt;
char col_name[RECORD_COLUMNS][EXPR_LENGTH];
byte col_leng[RECORD_COLUMNS];
byte is_grpby = 0, is_odrby = 0, is_limit = 0;
LimitNode limit;
ExprNode *vcol[RECORD_COLUMNS], *gcol[RECORD_COLUMNS], *ocol[RECORD_COLUMNS];
byte gsc[RECORD_COLUMNS], osc[RECORD_COLUMNS];
u16 col_prop[RECORD_COLUMNS], vcol_prop[RECORD_COLUMNS],
    ocol_prop[RECORD_COLUMNS];

/*
    加入一张表table到虚拟表rec中
    返回值表示过程是否出错
*/
inline int append_record_table(TableNode *table, Record *rec)
{
    if (table == NULL || rec == NULL)
    {
        return 0;
    }
    if (table->type == TABLE_DEFAULT)
        for (int i = 0; i < DATABASE_TABLE_COUNT; ++i)
        {
            if (!stricmp(table->table, catalog.tbls[i].name))
            {
                for (int j = 0, k = rec->beg[rec->cnt]; j < catalog.tbls[i].cc;
                     ++j)
                {
                    rec->item[j + k].type = catalog.tbls[i].cols[j].type;
                    // rec->item[j + k].alias = strdup
                    // (catalog.tbls[i].cols[j].name);
                    rec->table[j + k] = strdup(catalog.tbls[i].name);
                    rec->name[j + k] = strdup(catalog.tbls[i].cols[j].name);
                    rec->alias[j + k] = strdup(table->alias);
                }
                rec->tbl[rec->cnt] = i;
                rec->rtb[i] = rec->cnt;
                rec->beg[rec->cnt + 1] =
                    rec->beg[rec->cnt] + catalog.tbls[i].cc;
                ++rec->cnt;
                return 0;
            }
        }
    else
    {
        plog("[ERROR](%d): Unknown table type.\n", -UNKNOWN_TABLE);
        return ERROR;
    }
    plog("[ERROR](%d): Table NOT exists.\n", -TABLE_NOT_EXIST);
    return ERROR;
}

#define move_data(tp)                                                          \
    memcpy(rec->ptr[rec->rtb[type]] + isiz[type] * (rec->siz[rec->rtb[type]]), \
           &(tp->tp), isiz[type]);                                             \
    ++(rec->siz[rec->rtb[type]]);

#define check_need(status, type) (((status) >> type) & 1)

/*
    status的3位分别表示3个表是否需要提取出来数据
*/
inline void load_data_recursively(int status, Record *rec, CarTypeNode *ct,
                                  CarInfoNode *ci, RentOrderNode *ro, int type)
{
    switch (type)
    {
    case TYPE_CAR:
        if (ct == NULL)
        {
            return;
        }
        if (check_need(status, TYPE_CAR))
        {
            memcpy(rec->ptr[rec->rtb[type]] +
                       isiz[type] * (rec->siz[rec->rtb[type]]),
                   &(ct->ct), isiz[type]);
            ++(rec->siz[rec->rtb[type]]);
        }
        load_data_recursively(status, rec, ct, ct->head->next, ro, TYPE_INFO);
        break;
    case TYPE_INFO:
        if (ci == NULL)
        {
            return load_data_recursively(status, rec, ct->next, NULL, NULL,
                                         TYPE_CAR);
        }
        if (check_need(status, TYPE_INFO))
        {
            memcpy(rec->ptr[rec->rtb[type]] +
                       isiz[type] * (rec->siz[rec->rtb[type]]),
                   &(ci->ci), isiz[type]);
            ++(rec->siz[rec->rtb[type]]);
        }
        load_data_recursively(status, rec, ct, ci, ci->head->next, TYPE_ORDER);
        break;
    case TYPE_ORDER:
        if (ro == NULL)
        {
            return load_data_recursively(status, rec, ct, ci->next, NULL,
                                         TYPE_INFO);
        }
        if (check_need(status, TYPE_ORDER))
        {
            memcpy(rec->ptr[rec->rtb[type]] +
                       isiz[type] * (rec->siz[rec->rtb[type]]),
                   &(ro->ro), isiz[type]);
            ++(rec->siz[rec->rtb[type]]);
        }
        load_data_recursively(status, rec, ct, ci, ro->next, TYPE_ORDER);
        break;
    }
}

/*
    加载具体数据
*/
inline void load_data(int status, Record *rec)
{
    return load_data_recursively(status, rec, head->next, NULL, NULL, TYPE_CAR);
}

/*
    把虚拟表rec中的若干表的数据加载进来, 以数组形式放在rec->arr[]中
*/
inline void load_record(Record *rec)
{
    if (rec == NULL)
    {
        return;
    }
    int status = 0;
    for (int i = 0; i < rec->cnt; ++i)
    {
        // rec->siz[i] = isiz[rec->tbl[i]];
        rec->siz[i] = 0;
        rec->ptr[i] = rec->arr[i] =
            calloc(icnt[rec->tbl[i]], isiz[rec->tbl[i]]);
        status |= (1 << rec->tbl[i]);
    }
    load_data(status, rec);
    load_item(rec, 0);
}

/*
    用table节点构建虚拟表, 存放在rec
*/
inline byte load_tables(TableNode *table, Record *rec)
{
    while (table != NULL)
    {
        byte res = append_record_table(table, rec);
        if (res == ERROR)
        {
            return res;
        }
        table = table->next;
    }
    load_record(rec);
    return 0;
}

inline void swap_byte(char *a, char *b)
{
    char tmp = *a;
    *a = *b;
    *b = tmp;
}

inline int reverse_int(int *x)
{
    int l = sizeof(int);
    char buf[8];
    memcpy(buf, x, l);
    for (int i = 0; i < l / 2; ++i)
    {
        swap_byte(buf + i, buf + l - i - 1);
    }
    return *(int *)buf;
}

/*
    把rec下标从beg开始到cnt-1的表当前对应的数据加载到rec->item[]区域,
   便于之后直接进行运算
*/
inline void load_item(Record *rec, int beg)
{
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
                memcpy(&(rec->item[k].intval), src, sizeof(int));
                break;
            case EXPR_APPROXNUM:
                memcpy(&(rec->item[k].floatval), src, sizeof(float));
                break;
            case EXPR_STRING:
            case EXPR_DATETIME:
                rec->item[k].strval = calloc(1, ci->size + 1);
                memcpy(rec->item[k].strval, src, ci->size);
                break;
            }
        }
    }
}

inline int get_next_record(Record *rec)
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
        load_item(rec, j);
        return 1;
    }
}

inline int get_index_by_name(char *name, Record *rec)
{
    int cnt = 0, res = -1;
    for (uint i = 0; i < rec->beg[rec->cnt]; ++i)
    {
        if (!stricmp(name, rec->name[i]))
        {
            res = i;
            ++cnt;
        }
    }
    if (!cnt)
    {
        plog("[ERROR](%d): Unknown column '%s'.\n", -UNKNOWN_TABLE, name);
        return UNKNOWN_COLUMN;
    }
    else if (cnt > 1)
    {
        plog("[ERROR](%d): Ambiguous column '%s'.\n", -AMBIGUOUS_COLUMN, name);
        return AMBIGUOUS_COLUMN;
    }
    else
    {
        return res;
    }
}

inline int get_index_by_table_column(char *table, char *column, Record *rec)
{
    int cnt = 0, res = -1;
    for (uint i = 0; i < rec->beg[rec->cnt]; ++i)
    {
        if (!stricmp(column, rec->name[i]) &&
            (!stricmp(table, rec->table[i]) || !stricmp(table, rec->alias[i])))
        {
            res = i;
            ++cnt;
        }
    }
    if (!cnt)
    {
        plog("[ERROR](%d): Unknown column '%s.%s'.\n", -UNKNOWN_TABLE, table,
             column);
        return UNKNOWN_COLUMN;
    }
    else if (cnt > 1)
    {
        plog("[ERROR](%d): Ambiguous column '%s.%s'.\n", -AMBIGUOUS_COLUMN,
             table, column);
        return AMBIGUOUS_COLUMN;
    }
    else
    {
        return res;
    }
}

inline ExprNode *make_expr_by_name(char *name, Record *rec)
{
    int id = get_index_by_name(name, rec);
    if (id >= 0)
    {
        return &(rec->item[id]);
    }
    else
    {
        return &error_expr;
    }
}

inline ExprNode *make_expr_by_table_column(char *table, char *col, Record *rec)
{
    int id = get_index_by_table_column(table, col, rec);
    if (id >= 0)
    {
        return &(rec->item[id]);
    }
    else
    {
        return &error_expr;
    }
}

#define can_asmd(expr)                                                         \
    ((expr)->type == EXPR_INTNUM || (expr)->type == EXPR_APPROXNUM)
#define can_comp(lhs, rhs)                                                     \
    (max(lhs->type, rhs->type) <= EXPR_APPROXNUM ||                            \
     (min(lhs->type, rhs->type) >= EXPR_STRING &&                              \
      max(lhs->type, rhs->type) <= EXPR_DATETIME))
#define get_number(expr)                                                       \
    ((expr)->type == EXPR_INTNUM ? (expr)->intval : (expr)->floatval)
#define get_logic(expr) ((expr)->intval != 0)

inline time_t make_datetime(char *s)
{
    struct tm tp;
    memset(&tp, 0, sizeof(tp));
    strptime(s, CRIMS_DATETIME_FORMAT, &tp);
    time_t t = mktime(&tp);
    return t;
}

#define DEFINE_BINARY_ARI_OP(name, op)                                         \
    inline ExprNode *eval_##name(ExprNode *l, ExprNode *r, Record *rec)        \
    {                                                                          \
        if (l == NULL || r == NULL)                                            \
            return &error_expr;                                                \
        ExprNode *lhs = eval_expr(l, rec), *rhs = eval_expr(r, rec),           \
                 *res = calloc(1, sizeof(ExprNode));                           \
        if (is_lazy(lhs) || is_lazy(rhs))                                      \
            return &lazy_expr;                                                 \
        if (can_asmd(lhs) && can_asmd(rhs))                                    \
        {                                                                      \
            if (max(lhs->type, rhs->type) == EXPR_APPROXNUM)                   \
            {                                                                  \
                res->type = EXPR_APPROXNUM;                                    \
                res->floatval = get_number(lhs) op get_number(rhs);            \
            }                                                                  \
            else                                                               \
            {                                                                  \
                res->type = EXPR_INTNUM;                                       \
                res->intval = lhs->intval op rhs->intval;                      \
            }                                                                  \
        }                                                                      \
        return res;                                                            \
    }

#define DEFINE_BINARY_COM_OP(name, op)                                         \
    inline ExprNode *eval_##name(ExprNode *l, ExprNode *r, Record *rec)        \
    {                                                                          \
        if (l == NULL || r == NULL)                                            \
            return &error_expr;                                                \
        ExprNode *lhs = eval_expr(l, rec), *rhs = eval_expr(r, rec),           \
                 *res = calloc(1, sizeof(ExprNode));                           \
        if (is_lazy(lhs) || is_lazy(rhs))                                      \
            return &lazy_expr;                                                 \
        if (can_comp(lhs, rhs))                                                \
        {                                                                      \
            res->type = EXPR_INTNUM;                                           \
            if (max(lhs->type, rhs->type) <= EXPR_APPROXNUM)                   \
            {                                                                  \
                res->intval = get_number(lhs) op get_number(rhs);              \
            }                                                                  \
            else if (lhs->type == EXPR_STRING && rhs->type == EXPR_STRING)     \
            {                                                                  \
                res->intval = strcmp(lhs->strval, rhs->strval) op 0;           \
            }                                                                  \
            else                                                               \
                res->intval =                                                  \
                    make_datetime(lhs->strval) op make_datetime(rhs->strval);  \
        }                                                                      \
        return res;                                                            \
    }

DEFINE_BINARY_ARI_OP(add_expr, +)
DEFINE_BINARY_ARI_OP(sub_expr, -)
DEFINE_BINARY_ARI_OP(mul_expr, *)
DEFINE_BINARY_ARI_OP(div_expr, /)

DEFINE_BINARY_COM_OP(eq_expr, ==)
DEFINE_BINARY_COM_OP(ne_expr, !=)
DEFINE_BINARY_COM_OP(lt_expr, <)
DEFINE_BINARY_COM_OP(gt_expr, >)
DEFINE_BINARY_COM_OP(le_expr, <=)
DEFINE_BINARY_COM_OP(ge_expr, >=)

inline ExprNode *calc_logic(uint type, ExprNode *l, ExprNode *r)
{
    if (l == NULL || r == NULL)
    {
        return &error_expr;
    }
    if (l->type == EXPR_INTNUM && r->type == EXPR_INTNUM)
    {
        ExprNode *res = calloc(1, sizeof(ExprNode));
        res->type = EXPR_INTNUM;
        if (type == EXPR_AND)
        {
            res->intval = get_logic(l) && get_logic(r);
        }
        else if (type == EXPR_OR)
        {
            res->intval = get_logic(l) || get_logic(r);
        }
        else if (type == EXPR_XOR)
        {
            res->intval = get_logic(l) ^ get_logic(r);
        }
        return res;
    }
    return &error_expr;
}

inline ExprNode *is_in_val_list(ExprNode *expr, ExprNode *list, Record *rec)
{
    ExprNode *res = calloc(1, sizeof(ExprNode)), tmp;
    res->type = EXPR_INTNUM;
    res->intval = 1;
    expr = eval_expr(expr, rec);
    if (expr->type == EXPR_ERROR)
    {
        return &error_expr;
    }
    while (list)
    {
        tmp = *eval_expr(list, rec);
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
                if (!stricmp(expr->strval, tmp.strval))
                {
                    return res;
                }
                break;
            case EXPR_DATETIME:
                if (make_datetime(expr->strval) == make_datetime(tmp.strval))
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

inline ExprNode *is_in_select(ExprNode *expr, SelectNode *select, Record *rec)
{
    Record *subrec;
    Records *subrecs;
    ExprNode *res = calloc(1, sizeof(ExprNode));
    res->type = EXPR_INTNUM;
    res->intval = 1;
    if (select->recs == NULL)
    {
        subrec = calloc(1, sizeof(Record));
        subrecs = calloc(1, sizeof(Records));
        clear_records(subrecs);
        do_select(select, subrec, subrecs, 1, 0, 0);
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
        plog("Operand should contain 1 column");
        return &error_expr;
    }
    else
    {
        for (uint i = 0; i < subrecs->cnt; ++i)
        {
            res = eval_eq_expr(&(subrecs->recs[i].item[0]), expr, rec);
            if (res && res->type == EXPR_INTNUM && res->intval == 1)
            {
                return res;
            }
        }
    }
    res->intval = 0;
    return res;
}

inline int is_error(ExprNode *expr)
{
    return !expr || expr->type == EXPR_ERROR;
}

inline int is_true(ExprNode *expr)
{
    return expr && expr->type == EXPR_INTNUM && expr->intval == 1;
}

inline int is_case_expr(u16 type)
{
    return type == EXPR_CASE_EXPR || type == EXPR_CASE_EXPR_ELSE;
}

inline int is_agr_func(ExprNode *expr)
{
    return expr && expr->type > EXPR_AGR_FUNC_BEGIN &&
           expr->type < EXPR_AGR_FUNC_END;
}

inline int is_lazy(ExprNode *expr)
{
    return expr && expr->type == EXPR_LAZY;
}

inline int is_datetime(ExprNode *expr)
{
    return expr && (expr->type == EXPR_STRING || expr->type == EXPR_DATETIME);
}

inline ExprNode *transform_op_expr(ExprNode *grp)
{
    if (grp == NULL)
    {
        return NULL;
    }
    ExprNode *expr = calloc(1, sizeof(ExprNode));
    memcpy(expr, grp, sizeof(ExprNode));
    expr->type = expr->op;
    return expr;
}

inline ExprNode *make_int_expr(int x)
{
    ExprNode *expr = calloc(1, sizeof(ExprNode));
    expr->type = EXPR_INTNUM;
    expr->intval = x;
    return expr;
}

inline ExprNode *make_float_expr(float f)
{
    ExprNode *expr = calloc(1, sizeof(ExprNode));
    expr->type = EXPR_APPROXNUM;
    expr->floatval = f;
    return expr;
}

inline ExprNode *eval_case(ExprNode *case_node, Record *rec)
{
    ExprNode *case_head = case_node->case_head,
             *expr = eval_expr(case_node->l, rec), *res;
    u16 type = case_node->type;
    if (is_case_expr(type) && is_error(expr))
    {
        return &error_expr;
    }
    while (case_head)
    {
        ExprNode *l = eval_expr(case_head->l, rec),
                 *r = eval_expr(case_head->r, rec);
        if (is_error(l) || is_error(r))
        {
            return &error_expr;
        }
        res = is_case_expr(type) ? eval_eq_expr(expr, l, rec) : l;
        if (is_error(res))
        {
            return &error_expr;
        }
        else if (is_true(res))
        {
            return r;
        }
        case_head = case_head->next;
    }
}

inline ExprNode *eval_func(ExprNode *func, Record *rec)
{
    ExprNode *a0, *a1, *a2, *t1, *t2;
    if (!stricmp(func->strval, "TIMESTAMPDIFF"))
    {
        a0 = func->r;
        if (a0 && a0->next && a0->next->next)
        {
            a1 = transform_op_expr(a0->next), a2 = transform_op_expr(a1->next);
            t1 = eval_expr(a1, rec), t2 = eval_expr(a2, rec);
            if (!is_datetime(t1) || !is_datetime(t2))
            {
                return &error_expr;
            }
            float diff = make_datetime(t2->strval) - make_datetime(t1->strval);
            if (!stricmp(a0->strval, "DAY"))
            {
                return make_float_expr(diff / 3600 / 24);
            }
            else if (!stricmp(a0->strval, "HOUR"))
            {
                return make_float_expr(diff / 3600);
            }
            else if (!stricmp(a0->strval, "MINUTE"))
            {
                return make_float_expr(diff / 60);
            }
            else if (!stricmp(a0->strval, "SECOND"))
            {
                return make_float_expr(diff);
            }
        }
    }
    plog("[ERROR](%d): Invalid function statement %s.\n",
         INVALID_FUNCTION_STATEMENT, func->text);
    return &error_expr;
}

inline ExprNode *eval_mod_expr(ExprNode *l, ExprNode *r, Record *rec)
{
    ExprNode *res = calloc(1, sizeof(ExprNode));
    l = eval_expr(l, rec), r = eval_expr(r, rec);
    if (l->type == EXPR_INTNUM && r->type == EXPR_INTNUM)
    {
        res->type = EXPR_INTNUM;
        res->intval = l->intval % r->intval;
        return res;
    }
    else
    {
        plog("[ERROR](%d): There is no matching operator \%.\n",
             -NO_MATCHING_OPERATOR);
        return &error_expr;
    }
}

inline ExprNode *eval_neg_expr(ExprNode *r, Record *rec)
{
    ExprNode *res = calloc(1, sizeof(ExprNode));
    r = eval_expr(r, rec);
    if (r->type == EXPR_INTNUM)
    {
        res->type = EXPR_INTNUM;
        res->intval = -(r->intval);
        return res;
    }
    else
    {
        plog("[ERROR](%d): There is no matching operator -.\n",
             -NO_MATCHING_OPERATOR);
        return &error_expr;
    }
}

inline ExprNode *eval_not_expr(ExprNode *r, Record *rec)
{
    ExprNode *res = calloc(1, sizeof(ExprNode));
    r = eval_expr(r, rec);
    if (r->type == EXPR_INTNUM)
    {
        res->type = EXPR_INTNUM;
        res->intval = !(r->intval);
        return res;
    }
    else
    {
        plog("[ERROR](%d): There is no matching operator ~.\n",
             -NO_MATCHING_OPERATOR);
        return &error_expr;
    }
}

inline ExprNode *eval_like_expr(ExprNode *l, ExprNode *r, Record *rec)
{
    ExprNode *res = calloc(sizeof(ExprNode), 1);
    l = eval_expr(l, rec), r = eval_expr(r, rec);
    if (l->type == EXPR_STRING && r->type == EXPR_STRING)
    {
        res->type = EXPR_INTNUM;
        res->intval = match(r->strval, l->strval);
        return res;
    }
    else
    {
        plog("[ERROR](%d): There is no matching operator `LIKE`.\n",
             -NO_MATCHING_OPERATOR);
        return &error_expr;
    }
}

/*
    核心函数, 根据record记录来求表达式值
*/
inline ExprNode *eval_expr(ExprNode *expr, Record *rec)
{
    if (expr == NULL)
    {
        return &error_expr;
    }
    ExprNode *l, *r, *res = calloc(1, sizeof(ExprNode));
    int flag = 0;
    if (is_lazy(expr->l) || is_lazy(expr->r))
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
        return make_expr_by_name(expr->strval, rec);
    case EXPR_TABLE_COLUMN:
        return make_expr_by_table_column(expr->table, expr->strval, rec);
    case EXPR_ADD:
        return eval_add_expr(expr->l, expr->r, rec);
    case EXPR_SUB:
        return eval_sub_expr(expr->l, expr->r, rec);
    case EXPR_MUL:
        return eval_mul_expr(expr->l, expr->r, rec);
    case EXPR_DIV:
        return eval_div_expr(expr->l, expr->r, rec);
    case EXPR_MOD:
        return eval_mod_expr(expr->l, expr->r, rec);
    case EXPR_AND:
    case EXPR_OR:
    case EXPR_XOR:
        return calc_logic(expr->type, eval_expr(expr->l, rec),
                          eval_expr(expr->r, rec));
    case EXPR_EQ:
        return eval_eq_expr(expr->l, expr->r, rec);
    case EXPR_NE:
        return eval_ne_expr(expr->l, expr->r, rec);
    case EXPR_LT:
        return eval_lt_expr(expr->l, expr->r, rec);
    case EXPR_GT:
        return eval_gt_expr(expr->l, expr->r, rec);
    case EXPR_LE:
        return eval_le_expr(expr->l, expr->r, rec);
    case EXPR_GE:
        return eval_ge_expr(expr->l, expr->r, rec);
    case EXPR_NEG:
        return eval_neg_expr(expr->r, rec);
    case EXPR_NOT:
        return eval_not_expr(expr->r, rec);
    case EXPR_NOT_IN_VAL_LIST:
        flag = 1;
    case EXPR_IN_VAL_LIST:
        res = is_in_val_list(expr->l, expr->r, rec);
        res->intval ^= flag;
        return res;
    case EXPR_NOT_LIKE:
        flag = 1;
    case EXPR_LIKE:
        res = eval_like_expr(expr->l, expr->r, rec);
        res->intval ^= flag;
        return res;
    case EXPR_NOT_IN_SELECT:
        flag = 1;
    case EXPR_IN_SELECT:
        res = is_in_select(expr->l, expr->select, rec);
        res->intval ^= flag;
        return res;
    case EXPR_FUNC:
        return eval_func(expr, rec);
    case EXPR_COUNT:
    case EXPR_COUNT_ALL:
    case EXPR_SUM:
        if (query_status == QUERY_BEGIN)
        {
            return &lazy_expr;
        }
        else if (query_status == QUERY_REEVAL)
        {
            return &(rec->item[col_cnt + expr->intval]);
        }
    case EXPR_CASE:
    case EXPR_CASE_ELSE:
    case EXPR_CASE_EXPR:
    case EXPR_CASE_EXPR_ELSE:
        return eval_case(expr, rec);
    default:
        plog("[ERROR](%d): Unknown expression.\n", -UNKNOWN_EXPRESSION);
        return &error_expr;
    }
}

inline int judge_cond(ExprNode *cond, Record *rec)
{
    if (cond == NULL)
    {
        return 1;
    }
    ExprNode *res = eval_expr(cond, rec);
    if (res->type == EXPR_ERROR)
    {
        return ERROR;
    }
    else
    {
        return res->intval;
    }
}

inline void append_record_column(ExprNode *val, Record *rec)
{
    memcpy(&rec->item[(rec->cnt)++], val, sizeof(ExprNode));
}

inline ExprNode *eval_odrby_expr(ExprNode *expr, Record *rec)
{
    if (expr == NULL)
    {
        return &error_expr;
    }
    else if (expr->type == EXPR_STRING || expr->type == EXPR_NAME)
    {
        return &(rec->item[get_column_index(expr->strval)]);
    }
    else
    {
        return eval_expr(expr, rec);
    }
}

/*
    将符合where条件的记录求列值, 统计进recs
*/
inline int extract_record(ExprNode *column_head, Record *rec, Records *recs)
{
    if (column_head == NULL)
    {
        return 0;
    }
    Record *target = calloc(1, sizeof(Record));
    if (column_head != NULL && column_head->type == SELECT_ALL)
    {
        for (uint i = 0; i < rec->beg[rec->cnt]; ++i)
        {
            append_record_column(&(rec->item[i]), target);
        }
    }
    else
    {
        while (column_head)
        {
            ExprNode *res = eval_expr(column_head, rec);
            if (is_error(res))
            {
                return ERROR;
            }
            else
            {
                append_record_column(res, target);
            }
            column_head = column_head->next;
        }
        if (is_grpby)
        {
            for (uint i = 0; i < vcol_cnt; ++i)
            {
                ExprNode *res = eval_expr(vcol[i], rec);
                if (is_error(res))
                {
                    return ERROR;
                }
                else
                {
                    append_record_column(res, target);
                }
            }
            for (uint i = 0; i < gcol_cnt; ++i)
            {
                ExprNode *res = eval_expr(gcol[i], rec);
                if (is_error(res))
                {
                    return ERROR;
                }
                else
                {
                    append_record_column(res, target);
                }
            }
        }
        if (is_odrby)
        {
            for (uint i = 0; i < ocol_cnt; ++i)
            {
                ExprNode *res =
                    eval_odrby_expr(transform_op_expr(ocol[i]), rec);
                if (is_error(res))
                {
                    return ERROR;
                }
                else
                {
                    append_record_column(res, target);
                }
            }
        }
    }
    add_record(target, recs);
    return 0;
}

inline int get_index_by_table_name(char *table)
{
    for (uint i = 0; i < catalog.tc; ++i)
    {
        if (!stricmp(table, catalog.tbls[i].name))
        {
            return i;
        }
    }
    return -1;
}

/*
    计算顶层select的列数量, 结果写入全局变量col_cnt和col_name[]
*/
inline int calc_col_cnt(TableNode *table_head)
{
    int res = 0;
    while (table_head)
    {
        int tmp = get_index_by_table_name(table_head->table);
        if (tmp == -1)
        {
            plog("[ERROR](%d): Unknown table name '%s'.\n", UNKNOWN_TABLE,
                 table_head->table);
            return 0;
        }
        else
        {
            for (uint i = 0; i < catalog.tbls[tmp].cc; ++i)
            {
                strcat(col_name[col_cnt++], catalog.tbls[tmp].cols[i].name);
            }
            // res += catalog.tbls[tmp].cc;
        }
        table_head = table_head->next;
    }
    return res;
}

/*
    加载列名, 用于最后的输出信息, 与求值无关
*/
inline void load_column_names(ExprNode *col, TableNode *tbl)
{
    while (col)
    {
        if (col->type == SELECT_ALL)
        {
            calc_col_cnt(tbl);
            // col_cnt = calc_col_cnt (tbl);
            return;
        }
        else
        {
            if (!strlen(col->text))
            {
                plog("No column name can be used.");
            }
            if (col->alias && strlen(col->alias))
            {
                strcat(col_name[col_cnt++], col->alias);
            }
            else
            {
                strcat(col_name[col_cnt++], col->text);
            }
        }
        col = col->next;
    }
}

inline int extract_agr(ExprNode *expr)
{
    if (expr == NULL)
    {
        return 0;
    }
    if (is_agr_func(expr))
    {
        vcol[vcol_cnt] = expr->r;
        vcol_prop[vcol_cnt] = expr->type;
        expr->intval = vcol_cnt++;
        return 1;
    }
    return extract_agr(expr->l) || extract_agr(expr->r);
}

inline void build_agr_col(ExprNode *col)
{
    for (uint i = 0; i < col_cnt; ++i)
    {
        col_prop[i] = extract_agr(col);
        col = col->next;
    }
}

inline void build_grp_col(ExprNode *grp)
{
    while (grp)
    {
        gcol[gcol_cnt] = grp;
        gsc[gcol_cnt] = grp->sc;
        grp = grp->next;
        ++gcol_cnt;
    }
}

inline int get_column_index(char *col)
{
    for (uint i = 0; i < col_cnt; ++i)
        if (!stricmp(col, col_name[i]))
        {
            return i;
        }
    return ERROR;
}

inline int build_odr_col(ExprNode *odr)
{
    while (odr)
    {
        ocol[ocol_cnt] = odr;
        osc[ocol_cnt] = odr->sc;
        if ((ocol_prop[ocol_cnt] = get_column_index(odr->strval)) == ERROR)
        {
            plog("[ERROR](%d): Unknown orderby expression %s.\n",
                 UNKNOWN_ORDERBY_EXPRESSION, odr->strval);
            return ERROR;
        }
        odr = odr->next;
        ++ocol_cnt;
    }
    return 0;
}

inline int cmp_g(Record *rec1, Record *rec2)
{
    for (uint i = 0; i < gcol_cnt; ++i)
    {
        if (is_true(eval_ne_expr(&(rec1->item[i + col_cnt + vcol_cnt]),
                                 &(rec2->item[i + col_cnt + vcol_cnt]), NULL)))
        {
            if (is_true(eval_lt_expr(&(rec1->item[i + col_cnt + vcol_cnt]),
                                     &(rec2->item[i + col_cnt + vcol_cnt]),
                                     NULL)) ^
                gsc[i])
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

inline int cmp_o(Record *rec1, Record *rec2)
{
    for (uint i = 0; i < ocol_cnt; ++i)
    {
        if (is_true(eval_ne_expr(&(rec1->item[ocol_prop[i]]),
                                 &(rec2->item[ocol_prop[i]]), NULL)))
        {
            if (is_true(eval_lt_expr(&(rec1->item[ocol_prop[i]]),
                                     &(rec2->item[ocol_prop[i]]), NULL)) ^
                osc[i])
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

inline void merge_item_left(ExprNode *lhs, ExprNode *rhs, u16 type, uint beg,
                            uint end)
{
    ExprNode *res = lhs;
    switch (type)
    {
    case EXPR_COUNT_ALL: //这个直接不管, 可以特殊处理
    case EXPR_COUNT:     //同上
        res = make_int_expr(end - beg + 1);
        break;
    case EXPR_SUM:
        res = eval_add_expr(lhs, rhs, NULL);
        break;
    }
    memcpy(lhs, res, sizeof(ExprNode));
}

inline ExprNode *get_col_by_index(ExprNode *col_head, uint id)
{
    for (uint i = 0; i < id; ++i)
    {
        col_head = col_head->next;
    }
    return col_head;
}

inline int merge_agr(Records *recs, ExprNode *col_head)
{
    uint beg = 0, end = 1;
    while (end < recs->cnt)
    {
        int res = cmp_g(&(recs->recs[beg]), &(recs->recs[end]));
        if (res != 0)
        {
            for (uint i = 0; i < vcol_cnt; ++i)
            {
                for (uint j = beg + 1; j < end; ++j)
                {
                    merge_item_left(&(recs->recs[beg].item[i + col_cnt]),
                                    &(recs->recs[j].item[i + col_cnt]),
                                    vcol_prop[i], beg, j);
                }
            }
            for (uint i = 0; i < col_cnt; ++i)
                if (col_prop[i])
                {
                    ExprNode *res = eval_expr(get_col_by_index(col_head, i),
                                              &(recs->recs[beg]));
                    memcpy(&(recs->recs[beg].item[i]), res, sizeof(ExprNode));
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
            merge_item_left(&(recs->recs[beg].item[i + col_cnt]),
                            &(recs->recs[j].item[i + col_cnt]), vcol_prop[i],
                            beg, j);
        }
    }
    for (uint i = 0; i < col_cnt; ++i)
        if (col_prop[i])
        {
            ExprNode *res =
                eval_expr(get_col_by_index(col_head, i), &(recs->recs[beg]));
            memcpy(&(recs->recs[beg].item[i]), res, sizeof(ExprNode));
        }
    recs->recs[beg].next = recs->cnt;
    return 0;
}

/*
    select为执行的语句AST节点, rec存放虚拟表的每条查找记录, recs存储查找结果,
   subq标记这是否是子查询 返回值表示过程是否出错
*/
inline int do_select(SelectNode *select, Record *rec, Records *recs, byte subq,
                     byte grpby, byte odrby)
{
    if (is_limit = (select->limit != NULL))
    {
        limit = *(select->limit);
    }
    if (load_tables(select->table_head, rec) == ERROR)
    {
        return ERROR;
    }
    if (!subq)
    {
        load_column_names(select->column_head, select->table_head);
    } //如果不是子查询就加载列名, 因为最终输出的列名作用于最顶层的select的
    if (grpby)
    {
        build_agr_col(select->column_head);
        build_grp_col(select->group);
    } //将聚合函数单独找出来
    if (odrby)
    {
        if (build_odr_col(select->order) == ERROR)
        {
            return ERROR;
        }
    }
    do
    {
        int res = judge_cond(select->where, rec);
        if (res == ERROR)
        {
            return res;
        }
        else if (res == 1)
        {
            extract_record(select->column_head, rec, recs);
        }
    } while (get_next_record(rec));
    if (grpby)
    {
        query_status = QUERY_REEVAL;
        qsort(recs->recs, recs->cnt, sizeof(Record), cmp_g);
        int res = merge_agr(recs, select->column_head);
        if (res == ERROR)
        {
            return res;
        }
    }
    select->recs = recs;
    return 0;
}

inline void clear_record(Record *rec)
{
    rec->cnt = 0;
    for (int i = 0; i < DATABASE_TABLE_COUNT; ++i)
    {
        free(rec->arr[i]);
        rec->arr[i] = NULL; //不清零会导致下次释放非法内存
        rec->siz[i] = 0;
    }
}

inline void clear_records(Records *recs)
{
    if (recs->size != 0)
    {
        for (int i = 0; i < recs->cnt; ++i)
        {
            clear_record(recs->recs + i);
        }
    }
    else
    {
        recs->size = RECS_INITIAL_LENGTH;
        recs->recs = malloc(RECS_INITIAL_LENGTH * sizeof(Record));
    }
    recs->cnt = 0;
}

inline void add_record(Record *rec, Records *recs)
{
    if (recs->cnt + 1 >= recs->size)
    {
        recs->size <<= 1;
        recs->recs = realloc(recs, recs->size * sizeof(Record));
    }
    memcpy(recs->recs + recs->cnt, rec, sizeof(Record));
    ++recs->cnt;
}

inline void query_initialize()
{
    clear_record(&rec);
    clear_records(&recs);
    for (uint i = 0; i < col_cnt; ++i)
    {
        col_name[i][0] = '\0';
        col_leng[i] = 0;
    }
    is_grpby = 0, is_odrby = 0, is_limit = 0;
    limit.start = 0, limit.count = INT_MAX;
    col_cnt = 0, vcol_cnt = 0, gcol_cnt = 0;
    memset(gsc, 0, sizeof(gsc));
    memset(osc, 0, sizeof(osc));
    memset(ocol_prop, 0, sizeof(ocol_prop));
    query_status = QUERY_BEGIN;
}

/*
    计算UTF-8字符串的CJK字符数量
*/
inline uint calc_cjk(char *ustr)
{
    uint res = 0;
    char *p = ustr;
    while (*p)
    {
        if (*p > 0)
        {
            ++p;
        }
        else if (*p < 0)
        {
            if (*(p + 1) < 0 && *(p + 2) < 0)
            {
                ++res;
            }
            p += 3;
        }
    }
    return res;
}

/*
    计算UTF-8字符串的占位宽度, 中文宽度为英文宽度的两倍
*/
inline uint ustrlen(const char *ustr)
{
    uint len = 0;
    for (char *p = ustr; *p; ++p)
    {
        if (*p > 0)
        {
            ++len;
        }
        else if (*p < 0)
        {
            if (*(p + 1) < 0 && *(p + 2) < 0)
            {
                len += 2;
            }
            p += 2;
        }
    }
    return len;
}

inline byte calc_length(RecordCell *rc)
{
    static char buf[BUFFER_LENGTH];
    if (rc == NULL)
    {
        return 0;
    }
    else
    {
        switch (rc->type)
        {
        case EXPR_INTNUM:
            sprintf_s(buf, BUFFER_LENGTH, "%d", rc->intval);
            break;
        case EXPR_APPROXNUM:
            sprintf_s(buf, BUFFER_LENGTH, "%.2f", rc->floatval);
            break;
        case EXPR_STRING:
        case EXPR_DATETIME:
            return ustrlen(rc->strval);
            break;
        }
        return ustrlen(buf);
    }
}

inline void print_value(ExprNode *val, byte len)
{
    if (val == NULL)
    {
        return;
    }
    else
    {
        switch (val->type)
        {
        case EXPR_INTNUM:
            printf("%*d", len, val->intval);
            break;
        case EXPR_APPROXNUM:
            printf("%*.2f", len, val->floatval);
            break;
        case EXPR_STRING:
        case EXPR_DATETIME:
            printf("%-*s", len + calc_cjk(val->strval), val->strval);
            break;
        }
    }
}

inline void print_interval_line()
{
    for (uint i = 0; i < col_cnt; ++i)
    {
        printf("+");
        repeat('-', col_leng[i] + 2);
    }
    puts("+");
}

#define next(i) (is_grpby ? recs->recs[i].next : ((i) + 1))
char format[BUFFER_LENGTH];
Records result;
inline void print_result(Records *recs)
{
    op_end = clock();
    clear_records(&result);
    uint row_cnt = 0;
    memset(format, 0, sizeof(format));
    if (recs == NULL || recs->cnt == 0)
    {
        printf("Empty set\n");
    }
    else
    {
        uint suml = 0;
        for (uint j = 0; j < col_cnt; ++j)
        {
            col_leng[j] = ustrlen(col_name[j]);
            for (uint i = 0; i < recs->cnt; ++i)
            {
                col_leng[j] =
                    max(col_leng[j], calc_length(&(recs->recs[i].item[j])));
            }
            suml += col_leng[j];
        }
        print_interval_line();
        for (uint j = 0; j < col_cnt; ++j)
        {
            printf("| %*s ", col_leng[j] + calc_cjk(col_name[j]), col_name[j]);
        }
        puts("|");
        print_interval_line();
        for (uint i = 0; i < recs->cnt; i = next(i))
        {
            add_record(&(recs->recs[i]), &result);
        }
        if (is_odrby)
        {
            qsort(result.recs, result.cnt, sizeof(Record), cmp_o);
        }
        for (uint i = limit.start, cnt = 0; i < result.cnt && cnt < limit.count;
             ++i, ++cnt)
        {
            for (uint j = 0; j < col_cnt; ++j)
            {
                printf("| ");
                print_value(&(result.recs[i].item[j]), col_leng[j]);
                printf(" ");
            }
            puts("|");
            ++row_cnt;
        }
        print_interval_line();
        printf("%d %s in set (%.2f sec)\n", row_cnt,
               row_cnt > 1 ? "rows" : "row", (op_end - op_start) / _SC_CLK_TCK);
    }
}
#undef next