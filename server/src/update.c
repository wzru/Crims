#include "database.h"
#include "select.h"
#include "update.h"
#include "debug.h"
#include "exec.h"
#include "log.h"

inline int genericg_update (void *buf, int ti, SetNode *set)
{
    for (SetNode *p = set; p; p = p->next)
    {
        int vcnt = find_column_by_name (ti, p->column);
        if (vcnt == ERROR)
        {
            plog("[ERROR]: Unknown column name '%s'\n", p->column);
            return ERROR;
        }
        memcpy (buf + catalog.tbls[ti].cols[vcnt].offset,
                get_val_addr (catalog.tbls[ti].cols[vcnt].type, p->expr),
                catalog.tbls[ti].cols[vcnt].size);
    }
}

inline int traverse_update (int ti, int col, ExprNode *rhs, SetNode *set)
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
                    return genericg_update (& (ro->ro), ti, set);
                }
            }
            if (ti == TABLE_CAR_INFO && is_equal (&ci->ci, ti, col, rhs))
            {
                return genericg_update (& (ci->ci), ti, set);
            }
        }
        if (ti == TABLE_CAR_TYPE && is_equal (&ct->ct, ti, col, rhs))
        {
            return genericg_update (& (ct->ct), ti, set);
        }
    }
    return 0;
}

inline int do_update (UpdateNode *upd)
{
    if (upd == NULL || upd->table == NULL || upd->where == NULL
            || upd->set_head == NULL)
    {
        plog ("[ERROR]: Update parse failed\n");
        return ERROR;
    }
    if (upd->where->type != EXPR_EQ || upd->where->l->type != EXPR_NAME)
    {
        plog ("[ERROR]: Unknown update statement\n");
        return ERROR;
    }
    int ti = find_table_by_name (upd->table), c;
    switch (ti)
    {
    case ERROR:
        plog ("[ERROR]: Unknown table name\n");
        return ERROR;
    case TABLE_CAR_TYPE:
    case TABLE_CAR_INFO:
    case TABLE_RENT_ORDER:
        c = find_column_by_name (ti, upd->where->l->strval);
        if (!can_assign (catalog.tbls[ti].cols[c].type, upd->where->r->type))
        {
            plog ("[ERROR]: Types not match in update statement\n");
            return ERROR;
        }
        return traverse_update (ti, c, upd->where->r, upd->set_head);
    }
    return ERROR;
}