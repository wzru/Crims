#include <string.h>

#include "database.h"
#include "delete.h"
#include "debug.h"
#include "log.h"

inline int is_equal (void *p, int ti, int col, ExprNode *rhs)
{
    void *addr = p + catalog.tbls[ti].cols[col].offset;
    switch (rhs->type)
    {
    case EXPR_INTNUM:
        return * (int *) (addr) == rhs->intval;
    case EXPR_APPROXNUM:
        return * (float *) (addr) == rhs->intval;
    case EXPR_STRING:
    case EXPR_DATETIME:
        return !stricmp ( (char *) addr, rhs->strval);
    }
    plog ("[ERROR]: Unknown delete condition\n");
    return ERROR;
}

inline int traverse_delete (int ti, int col, ExprNode *rhs)
{
    int cnt = 0;
    for (CarTypeNode *ct = head->next, *pct = head; ct != NULL;
            pct = ct, ct = ct->next)
    {
        int ct_ci = 0, ct_ro = 0;
        for (CarInfoNode *ci = ct->head->next, *pci = ct->head; ci;
                pci = ci, ci = ci->next, ++ct_ci)
        {
            int ci_ro = 0;
            for (RentOrderNode *ro = ci->head->next, *pro = ci->head; ro;
                    pro = ro, ro = ro->next, ++ct_ro, ++ci_ro)
            {
                if (ti == TABLE_RENT_ORDER && is_equal (&ro->ro, ti, col, rhs))
                {
                    pro->next = ro->next;
                    //ro = ro->next;
                    --icnt[ti];
                    ++cnt;
                }
            }
            if (ti == TABLE_CAR_INFO && is_equal (&ci->ci, ti, col, rhs))
            {
                pci->next = ci->next;
                //ci = ci->next;
                --icnt[ti];
                ++cnt;
                icnt[TABLE_RENT_ORDER] -= ci_ro;
            }
        }
        if (ti == TABLE_CAR_TYPE && is_equal (&ct->ct, ti, col, rhs))
        {
            pct->next = ct->next;
            //ct = ct->next;//memory leak
            --icnt[ti];
            ++cnt;
            icnt[TABLE_CAR_INFO] -= ct_ci;
            icnt[TABLE_RENT_ORDER] -= ct_ro;
            update_ct_ptr();
        }
    }
    return cnt;
}

inline int do_delete (DeleteNode *del)
{
    if (del == NULL || del->table == NULL || del->where == NULL)
    {
        return ERROR;
    }
    if (del->where->type != EXPR_EQ || del->where->l->type != EXPR_NAME)
    {
        plog ("[ERROR]: Unknown delete statement\n");
        return ERROR;
    }
    int ti = find_table_by_name (del->table), c;
    switch (ti)
    {
    case ERROR:
        plog ("[ERROR]: Unknown table name\n");
        return ERROR;
    case TABLE_CAR_TYPE:
    case TABLE_CAR_INFO:
    case TABLE_RENT_ORDER:
        c = find_column_by_name (ti, del->where->l->strval);
        if (!can_assign (catalog.tbls[ti].cols[c].type, del->where->r->type))
        {
            plog ("[ERROR]: Types not match in delete statement\n");
            return ERROR;
        }
        return traverse_delete (ti, c, del->where->r);
    }
    return ERROR;
}