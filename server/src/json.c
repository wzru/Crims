#include <stdio.h>

#include "json.h"

inline void jsonify_car_type (CarType *ct, char *json)
{
    sprintf (json, "{\"code\":\"%s\",\"tname\":\"%s\",\"quantity\":%d}",
             ct->code, ct->tname, ct->quantity);
}

inline void jsonify_car_info (CarInfo *ci, char *json)
{
    sprintf (json,
             "{\"cid\":%d,\"plate\":\"%s\",\"code\":\"%s\",\"cname\":\"%s\",\"gear\":\"%s\",\"daily_rent\":%f,\"rent\":\"%s\"}",
             ci->cid, ci->plate, ci->code, ci->cname, ci->gear, ci->daily_rent, ci->rent
            );
}

inline void jsonify_rent_order (RentOrder *ro, char *json)
{
    sprintf (json,
             "{\"oid\":\"%s\",\"identity_number\":\"%s\",\"pname\":\"%s\",\"phone_number\":\"%s\",\"cid\":%d,\"pickup_time\":\"%s\",\"scheduled_dropoff_time\":\"%s\",\"deposit\":%f,\"actual_dropoff_time\":\"%s\",\"scheduled_fee\":%f,\"actual_fee\":%f}",
             ro->oid, ro->identity_number, ro->pname, ro->phone_number, ro->cid,
             ro->pickup_time, ro->scheduled_dropoff_time, ro->deposit,
             ro->actual_dropoff_time, ro->scheduled_fee, ro->actual_fee
            );
}

inline int jsonify_value (char *json, ExprNode *val)
{
    if (val == NULL)
    {
        return 0;
    }
    else
    {
        switch (val->type)
        {
        case EXPR_INTNUM:
            return sprintf (json, "%d", val->intval);
        case EXPR_APPROXNUM:
            return sprintf (json, "%.2f", val->floatval);
        case EXPR_STRING:
        case EXPR_DATETIME:
            return sprintf (json, "'%s'", val->strval);
        }
    }
}

#define next(i) (is_grpby?recs->recs[i].next:((i)+1))

char json_buffer[JSON_BUFFER_LENGTH];
Records result;
inline void jsonify_result (Records *recs)
{
    char *p = json_buffer;
    clear_records (&result);
    uint row_cnt = 0;
    p += sprintf (p, "{\nsuccess:true,\ndata:[\n");
    if (recs == NULL || recs->cnt == 0);
    else
    {
        uint suml = 0;
        for (uint j = 0; j < col_cnt; ++j)
        {
            col_leng[j] = ustrlen (col_name[j]);
            for (uint i = 0; i < recs->cnt; ++i)
            {
                col_leng[j] = max (col_leng[j], calc_length (& (recs->recs[i].item[j])));
            }
            suml += col_leng[j];
        }
        for (uint i = 0; i < recs->cnt; i = next (i))
        {
            add_record (& (recs->recs[i]), &result);
        }
        if (is_odrby)
        {
            qsort (result.recs, result.cnt, sizeof (Record), cmp_o);
        }
        for (uint i = limit.start, cnt = 0; i < result.cnt
                && cnt < limit.count; ++i, ++cnt)
        {
            p += sprintf (p, "\t{");
            for (uint j = 0; j < col_cnt; ++j)
            {
                p += sprintf (p, "'%s':", col_name[j]);
                p += jsonify_value (p, & (result.recs[i].item[j]));
                if (j != col_cnt - 1)
                {
                    p += sprintf (p, ",");
                }
            }
            p += sprintf (p, "},\n");
            ++row_cnt;
        }
        p -= 2;
        sprintf (p, "\n\t]\n}");
    }
}
#undef next

inline void jsonify_error()
{
    sprintf (json_buffer, "{\nsuccess:false\n}");
}

inline void jsonify_success()
{
    sprintf (json_buffer, "{\nsuccess:true\n}");
}