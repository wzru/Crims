#include <string.h>
#include <malloc.h>

#include "insert.h"
#include "database.h"
#include "debug.h"
#include "log.h"

inline void *get_val_addr (u16 dest, ExprNode *expr)
{
    if (expr == NULL)
    {
        return NULL;
    }
    switch (expr->type)
    {
    case EXPR_INTNUM:
        if(dest==EXPR_APPROXNUM)
        {
            expr->floatval = expr->intval;
            return & (expr->floatval);
        } else return & (expr->intval);
    case EXPR_APPROXNUM:
        if(dest==EXPR_INTNUM)
        {
            expr->intval = expr->floatval;
            return & (expr->intval);
        } else return & (expr->floatval);
    case EXPR_STRING:
    case EXPR_DATETIME:
        return expr->strval;
    }
}

inline int do_insert (InsertNode *insert)
{
    if (insert == NULL || insert->table == NULL || insert->value_list_head == NULL)
    {
        return ERROR;
    }
    int ti = find_table_by_name (insert->table);
    // CarType *ct = NULL;
    // CarInfo *ci = NULL;
    // RentOrder *ro = NULL;
    void *buf = NULL;
    if (ti == ERROR)
    {
        plog ("[ERROR]: Unknown table name\n");
        return ERROR;
    }
    int ccnt = 0;
    for (ValueListNode *vl = insert->value_list_head; vl; vl = vl->next)
    {
        int vcnt = 0;
        buf = calloc (1, isiz[ti]);
        for (ExprNode *v = vl->head; v; v = v->next, ++vcnt)
        {
            if (can_assign (catalog.tbls[ti].cols[vcnt].type, v->type))
            {
                memcpy (buf + catalog.tbls[ti].cols[vcnt].offset, get_val_addr (catalog.tbls[ti].cols[vcnt].type, v),
                        catalog.tbls[ti].cols[vcnt].size);
            }
            else
            {
                plog ("[ERROR]: Insert type NOT match\n");
                return ERROR;
            }
        }
        ++icnt[ti];
        ++ccnt;
        switch (ti)
        {
        case TABLE_CAR_TYPE:
            insert_car_type (buf);
            break;
        case TABLE_CAR_INFO:
            insert_car_info (buf);
            break;
        case TABLE_RENT_ORDER:
            insert_rent_order (buf);
            break;
        }
        free (buf);
    }
    plog ("[DATA]: Successfully insert %d %s\n", ccnt, ccnt > 1 ? "rows" : "row");
    return ccnt;
}