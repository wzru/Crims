#include <stddef.h>
#include <malloc.h>
#include <string.h>

#include "database.h"
#include "define.h"
#include "server.h"
#include "json.h"
#include "ast.h"

char database_path[DATABASE_PATH_LENGTH] = DEFAULT_DATABASE_PATH;

CarTypeNode *head = NULL;
CarTypeNode *ct_ptr = NULL;
CarInfoNode *ci_ptr = NULL;
RentOrderNode *ro_ptr = NULL;

uint icnt[DATABASE_TABLE_COUNT], isiz[DATABASE_TABLE_COUNT] = {sizeof (CarType), sizeof (CarInfo), sizeof (RentOrder) };

int is_saved = 1;

byte crims_table_column_count[DATABASE_TABLE_COUNT] = {3, 7, 11};

DatabaseInfo catalog =
{
    "CRIMS", DATABASE_TABLE_COUNT,
    {
        {
            "CAR_TYPE", 3,
            {
                {"code",     EXPR_STRING, sizeof (char),         offsetof (CarType, code) },
                {"tname",    EXPR_STRING, CAR_TYPE_TNAME_LENGTH, offsetof (CarType, tname) },
                {"quantity", EXPR_INTNUM, sizeof (int),          offsetof (CarType, quantity) }
            }
        },
        {
            "CAR_INFO", 7,
            {
                {"cid",        EXPR_INTNUM,    sizeof (int),                 offsetof (CarInfo, cid) },
                {"plate",      EXPR_STRING,    CAR_INFORMATION_PLATE_LENGTH, offsetof (CarInfo, plate) },
                {"code",       EXPR_STRING,    sizeof (char),                offsetof (CarInfo, code) },
                {"cname",      EXPR_STRING,    CAR_INFORMATION_CNAME_LENGTH, offsetof (CarInfo, cname) },
                {"gear",       EXPR_STRING,    CAR_INFORMATION_GEAR_LENGTH,  offsetof (CarInfo, gear) },
                {"daily_rent", EXPR_APPROXNUM, sizeof (float),               offsetof (CarInfo, daily_rent) },
                {"rent",       EXPR_STRING,    sizeof (char),                offsetof (CarInfo, rent) }
            }
        },
        {
            "RENT_ORDER", 11,
            {
                {"oid",                    EXPR_STRING,    RENT_ORDER_OID_LENGTH,             offsetof (RentOrder, oid) },
                {"identity_number",        EXPR_STRING,    RENT_ORDER_IDENTITY_NUMBER_LENGTH, offsetof (RentOrder, identity_number) },
                {"pname",                  EXPR_STRING,    RENT_ORDER_PNAME_LENGTH,           offsetof (RentOrder, pname) },
                {"phone_number",           EXPR_STRING,    RENT_ORDER_PHONE_NUMBER_LENGTH,    offsetof (RentOrder, phone_number) },
                {"cid",                    EXPR_INTNUM,    sizeof (int),                      offsetof (RentOrder, cid) },
                {"pickup_time",            EXPR_DATETIME,  RENT_ORDER_TIME_LENGTH,            offsetof (RentOrder, pickup_time) },
                {"scheduled_dropoff_time", EXPR_DATETIME,  RENT_ORDER_TIME_LENGTH,            offsetof (RentOrder, scheduled_dropoff_time) },
                {"deposit",                EXPR_APPROXNUM, sizeof (float),                    offsetof (RentOrder, deposit) },
                {"actual_dropoff_time",    EXPR_DATETIME,  RENT_ORDER_TIME_LENGTH,            offsetof (RentOrder, actual_dropoff_time) },
                {"scheduled_fee",          EXPR_APPROXNUM, sizeof (float),                    offsetof (RentOrder, scheduled_fee) },
                {"actual_fee",             EXPR_APPROXNUM, sizeof (float),                    offsetof (RentOrder, actual_fee) }
            }
        }
    }
};

inline void read_initialize()
{
    head = calloc (1, sizeof (CarType));
    ct_ptr = head;
    ci_ptr = NULL;
    ro_ptr = NULL;
}

inline void read_car_type (FILE *stream)
{
    CarTypeNode *ct = calloc (1, sizeof (CarTypeNode));
    fread ( (void *) & (ct->ct), sizeof (CarType), 1, stream);
    ct_ptr->next = ct;
    ct_ptr = ct;
    ct_ptr->next = NULL;
    ct_ptr->head = ci_ptr = calloc (1, sizeof (CarInfoNode));
    ro_ptr = NULL;
}

inline void read_car_info (FILE *stream)
{
    CarInfoNode *ci = calloc (1, sizeof (CarInfoNode));
    fread ( (void *) & (ci->ci), sizeof (CarInfo), 1, stream);
    ci_ptr->next = ci;
    ci_ptr = ci;
    ci_ptr->next = NULL;
    ci_ptr->head = ro_ptr = calloc (1, sizeof (RentOrderNode));
    ro_ptr->next = NULL;
}

inline void read_rent_order (FILE *stream)
{
    RentOrderNode *ro = calloc (1, sizeof (RentOrderNode));
    fread ( (void *) & (ro->ro), sizeof (RentOrder), 1, stream);
    ro_ptr->next = ro;
    ro_ptr = ro;
    ro_ptr->next = NULL;
}

inline int read (char *db)
{
    read_initialize();
    FILE *fp = fopen (db, "rb");
    if (fp == NULL)
    {
        printf ("File open ERROR!\n");
        return -1;
    }
    while (!feof (fp))
    {
        int c = fgetc (fp);
        switch (c)
        {
        case TYPE_CAR:
            ++icnt[TYPE_CAR];
            read_car_type (fp);
            break;
        case TYPE_INFO:
            ++icnt[TYPE_INFO];
            read_car_info (fp);
            break;
        case TYPE_ORDER:
            ++icnt[TYPE_ORDER];
            read_rent_order (fp);
            break;
        default:
            //printf("WTF?");
            break;
        }
    }
    fclose (fp);
    return 0;
}

inline void write_car_type (CarType *ct, FILE *stream)
{
    fputc (TYPE_CAR, stream);
    fwrite (ct, sizeof (CarType), 1, stream);
}

inline void write_car_info (CarInfo *ci, FILE *stream)
{
    fputc (TYPE_INFO, stream);
    fwrite (ci, sizeof (CarInfo), 1, stream);
}

inline void write_rent_order (RentOrder *ro, FILE *stream)
{
    fputc (TYPE_ORDER, stream);
    fwrite (ro, sizeof (RentOrder), 1, stream);
}

inline void write_recursively (CarTypeNode *ct, CarInfoNode *ci,
                               RentOrderNode *ro, int type, FILE *stream)
{
    switch (type)
    {
    case TYPE_CAR:
        if (ct == NULL)
        {
            return;
        }
        write_car_type (& (ct->ct), stream);
        write_recursively (ct, ct->head->next, ro, TYPE_INFO, stream);
        break;
    case TYPE_INFO:
        if (ci == NULL)
        {
            return write_recursively (ct->next, NULL, NULL, TYPE_CAR, stream);
        }
        write_car_info (& (ci->ci), stream);
        write_recursively (ct, ci, ci->head->next, TYPE_ORDER, stream);
        break;
    case TYPE_ORDER:
        if (ro == NULL)
        {
            return write_recursively (ct, ci->next, NULL, TYPE_INFO, stream);
        }
        write_rent_order (& (ro->ro), stream);
        write_recursively (ct, ci, ro->next, TYPE_ORDER, stream);
        break;
    }
}

inline int write (char *db)
{
    FILE *fp = fopen (db, "wb");
    if (fp == NULL)
    {
        printf ("File open ERROR!\n");
        return -1;
    }
    write_recursively (head->next, NULL, NULL, TYPE_CAR, fp);
    fclose (fp);
    is_saved = 1;
    return 0;
}

inline void input_car_type (CarType *ct)
{
    memset (ct, 0, sizeof (CarType));
    scanf (" %c%s%d", & (ct->code), ct->tname, & (ct->quantity));
}
inline void input_car_info (CarInfo *ci)
{
    memset (ci, 0, sizeof (CarInfo));
    scanf ("%d%s %c%s%s%f %c", & (ci->cid), (ci->plate), & (ci->code), (ci->cname),
           (ci->gear), & (ci->daily_rent), & (ci->rent));
}
inline void input_rent_order (RentOrder *ro)
{
    memset (ro, 0, sizeof (RentOrder));
    scanf ("%s%s%s%s%d%s%s%f%s%f%f", (ro->oid), (ro->identity_number), (ro->pname),
           (ro->phone_number), & (ro->cid), (ro->pickup_time),
           (ro->scheduled_dropoff_time), & (ro->deposit), (ro->actual_dropoff_time),
           & (ro->scheduled_fee), & (ro->actual_fee));
}

inline void insert_car_type (CarType *ct)
{
    ct_ptr->next = malloc (sizeof (CarTypeNode));
    ct_ptr = ct_ptr->next;
    //ct_ptr->ct = *ct;
    memcpy (& (ct_ptr->ct), ct, sizeof (CarType));
    ci_ptr = ct_ptr->head = malloc (sizeof (CarInfoNode));
    ci_ptr->next = NULL;
    ct_ptr->next = NULL;
    ro_ptr = ci_ptr->head = malloc (sizeof (RentOrderNode));
    ro_ptr->next = NULL;
}
inline void insert_car_info (CarInfo *ci)
{
    for (CarTypeNode *p = head->next; p; p = p->next)
        if (p->ct.code == ci->code)
        {
            CarInfoNode *q;
            for (q = p->head; q->next; q = q->next);
            q->next = malloc (sizeof (CarInfoNode));
            memcpy (& (q->next->ci), ci, sizeof (CarInfo));
            //q->next->ci = *ci;
            q->next->next = NULL;
            q->next->head = malloc (sizeof (RentOrderNode));
            q->next->head->next = NULL;
            return;
        }
}

inline void insert_rent_order (RentOrder *ro)
{
    for (CarTypeNode *p = head->next; p; p = p->next)
        for (CarInfoNode *q = p->head->next; q; q = q->next)
            if (q->ci.cid == ro->cid)
            {
                RentOrderNode *r;
                for (r = q->head; r->next; r = r->next);
                r->next = malloc (sizeof (RentOrderNode));
                memcpy (& (r->next->ro), ro, sizeof (RentOrder));
                //r->next->ro = *ro;
                r->next->next = NULL;
                return;
            }
}

char print_json_buffer[JSON_BUFFER_LENGTH];

inline void print_car_type (CarType *ct, FILE *stream)
{
    jsonify_car_type (ct, print_json_buffer);
    fprintf (stream, "%s\n", print_json_buffer);
}

inline void print_car_info (CarInfo *ci, FILE *stream)
{
    jsonify_car_info (ci, print_json_buffer);
    fprintf (stream, "%s\n", print_json_buffer);
}

inline void print_rent_order (RentOrder *ro, FILE *stream)
{
    jsonify_rent_order (ro, print_json_buffer);
    fprintf (stream, "%s\n", print_json_buffer);
}

inline void recursive_print (CarTypeNode *ct, CarInfoNode *ci,
                             RentOrderNode *ro, int type, FILE *stream)
{
    switch (type)
    {
    case TYPE_CAR:
        if (ct == NULL)
        {
            return;
        }
        print_car_type (& (ct->ct), stream);
        recursive_print (ct, ct->head->next, ro, TYPE_INFO, stream);
        break;
    case TYPE_INFO:
        if (ci == NULL)
        {
            return recursive_print (ct->next, NULL, NULL, TYPE_CAR, stream);
        }
        print_car_info (& (ci->ci), stream);
        recursive_print (ct, ci, ci->head->next, TYPE_ORDER, stream);
        break;
    case TYPE_ORDER:
        if (ro == NULL)
        {
            return recursive_print (ct, ci->next, NULL, TYPE_INFO, stream);
        }
        print_rent_order (& (ro->ro), stream);
        recursive_print (ct, ci, ro->next, TYPE_ORDER, stream);
        break;
    }
}

inline void database_initialize()
{
    read (database_path);
    query_initialize();
}

/*
    计算UTF-8字符串的CJK字符数量
*/
inline uint calc_cjk (char *ustr)
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
            if (* (p + 1) < 0 && * (p + 2) < 0)
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
inline uint ustrlen (const char *ustr)
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
            if (* (p + 1) < 0 && * (p + 2) < 0)
            {
                len += 2;
            }
            p += 2;
        }
    }
    return len;
}

inline byte calc_length (RecordCell *rc)
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
            sprintf_s (buf, BUFFER_LENGTH, "%d", rc->intval);
            break;
        case EXPR_APPROXNUM:
            sprintf_s (buf, BUFFER_LENGTH, "%.2f", rc->floatval);
            break;
        case EXPR_STRING:
        case EXPR_DATETIME:
            return ustrlen (rc->strval);
            break;
        }
        return ustrlen (buf);
    }
}


inline void print_value (ExprNode *val, byte len)
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
            printf ("%*d", len, val->intval);
            break;
        case EXPR_APPROXNUM:
            printf ("%*.2f", len, val->floatval);
            break;
        case EXPR_STRING:
        case EXPR_DATETIME:
            printf ("%-*s", len + calc_cjk (val->strval), val->strval);
            break;
        }
    }
}

inline void print_interval_line()
{
    for (uint i = 0; i < col_cnt; ++i)
    {
        printf ("+");
        repeat ('-', col_leng[i] + 2);
    }
    puts ("+");
}

#define next(i) (is_grpby?recs->recs[i].next:((i)+1))

char format[JSON_BUFFER_LENGTH];
inline void print_result (Records *recs)
{
    op_end = clock();
    uint row_cnt = 0;
    memset (format, 0, sizeof (format));
    if (recs == NULL || recs->cnt == 0)
    {
        printf ("Empty set\n");
    }
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
        // #ifdef DEBUG
        // for (uint j = 0; j < col_cnt; ++j)
        // {
        //     printf ("%d ", col_leng[j]);
        // }
        // putchar ('\n');
        // #endif
        print_interval_line();
        for (uint j = 0; j < col_cnt; ++j)
        {
            printf ("| %*s ", col_leng[j] + calc_cjk (col_name[j]), col_name[j]);
        }
        puts ("|");
        print_interval_line();
        for (uint i = 0; i < recs->cnt; i = next (i))
        {
            for (uint j = 0; j < col_cnt; ++j)
            {
                printf ("| ");
                print_value (& (recs->recs[i].item[j]), col_leng[j]);
                printf (" ");
            }
            puts ("|");
            ++row_cnt;
        }
        print_interval_line();
        printf ("%d %s in set (%.2f sec)\n", row_cnt, row_cnt > 1 ? "rows" : "row",
                (op_end - op_start) / CLK_TCK);
    }
}