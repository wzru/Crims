#include "database.h"
#include "analyze.h"
#include "ast.h"

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