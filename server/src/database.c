#include <stddef.h>
#include <malloc.h>
#include <string.h>

#include "select.h"
#include "database.h"
#include "define.h"
#include "debug.h"
#include "json.h"
#include "ast.h"

char database_path[PATH_LENGTH] = DEFAULT_DATABASE_PATH;

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
                {"code",     EXPR_STRING, CAR_TYPE_CODE_LENGTH,  offsetof (CarType, code) },
                {"tname",    EXPR_STRING, CAR_TYPE_TNAME_LENGTH, offsetof (CarType, tname) },
                {"quantity", EXPR_INTNUM, sizeof (int),          offsetof (CarType, quantity) }
            }
        },
        {
            "CAR_INFO", 7,
            {
                {"cid",        EXPR_INTNUM,    sizeof (int),                 offsetof (CarInfo, cid) },
                {"plate",      EXPR_STRING,    CAR_INFORMATION_PLATE_LENGTH, offsetof (CarInfo, plate) },
                {"code",       EXPR_STRING,    CAR_TYPE_CODE_LENGTH,         offsetof (CarInfo, code) },
                {"cname",      EXPR_STRING,    CAR_INFORMATION_CNAME_LENGTH, offsetof (CarInfo, cname) },
                {"gear",       EXPR_STRING,    CAR_INFORMATION_GEAR_LENGTH,  offsetof (CarInfo, gear) },
                {"daily_rent", EXPR_APPROXNUM, sizeof (float),               offsetof (CarInfo, daily_rent) },
                {"rent",       EXPR_STRING,    sizeof (char) * 2,            offsetof (CarInfo, rent) }
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

/*
    读取数据之前初始化一些全局变量
*/
inline void read_initialize()
{
    head = calloc (1, sizeof (CarType));
    ct_ptr = head;
    ci_ptr = NULL;
    ro_ptr = NULL;
}

/*
    从stream中读取车辆类别
*/
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

/*
    从stream中读取车辆信息
*/
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

/*
    从stream中读取租车订单
*/
inline void read_rent_order (FILE *stream)
{
    RentOrderNode *ro = calloc (1, sizeof (RentOrderNode));
    fread ( (void *) & (ro->ro), sizeof (RentOrder), 1, stream);
    ro_ptr->next = ro;
    ro_ptr = ro;
    ro_ptr->next = NULL;
}


/*
    递归读取数据文件
*/
inline int read_db (char *db)
{
    read_initialize();
    FILE *fp = fopen (db, "rb");
    if (fp == NULL)
    {
        printf ("[ERROR]: Database file open failed!\n");
        return ERROR;
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
    printf ("[DATA]: Database file read successfully!\n");
    return 0;
}

/*
    向stream中写入车辆类别
*/
inline void write_car_type (CarType *ct, FILE *stream)
{
    fputc (TYPE_CAR, stream);
    fwrite (ct, sizeof (CarType), 1, stream);
}

/*
    向stream中写入车辆信息
*/
inline void write_car_info (CarInfo *ci, FILE *stream)
{
    fputc (TYPE_INFO, stream);
    fwrite (ci, sizeof (CarInfo), 1, stream);
}

/*
    向stream中写入租车订单
*/
inline void write_rent_order (RentOrder *ro, FILE *stream)
{
    fputc (TYPE_ORDER, stream);
    fwrite (ro, sizeof (RentOrder), 1, stream);
}

/*
    递归写入数据文件
*/
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

/*
    写数据文件
*/
inline int write_db (char *db)
{
    FILE *fp = fopen (db, "wb");
    if (fp == NULL)
    {
        printf ("[ERROR]: Database file open failed!\n");
        return ERROR;
    }
    write_recursively (head->next, NULL, NULL, TYPE_CAR, fp);
    fclose (fp);
    is_saved = 1;
    printf ("[DATA]: Database file written successfully!\n");
    return 0;
}

inline void input_car_type (CarType *ct)
{
    memset (ct, 0, sizeof (CarType));
    scanf ("%s%s%d", ct->code, ct->tname, & (ct->quantity));
}
inline void input_car_info (CarInfo *ci)
{
    memset (ci, 0, sizeof (CarInfo));
    scanf ("%d%s%s%s%s%f%s", & (ci->cid), (ci->plate), (ci->code), (ci->cname),
           (ci->gear), & (ci->daily_rent), (ci->rent));
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
    ct_ptr->next = calloc (1, sizeof (CarTypeNode));
    ct_ptr = ct_ptr->next;
    //ct_ptr->ct = *ct;
    memcpy (& (ct_ptr->ct), ct, sizeof (CarType));
    ci_ptr = ct_ptr->head = calloc (1, sizeof (CarInfoNode));
    ci_ptr->next = NULL;
    ct_ptr->next = NULL;
    ro_ptr = ci_ptr->head = calloc (1, sizeof (RentOrderNode));
    ro_ptr->next = NULL;
}

inline void insert_car_info (CarInfo *ci)
{
    for (CarTypeNode *p = head->next; p; p = p->next)
        if (!stricmp (p->ct.code, ci->code))
        {
            CarInfoNode *q;
            for (q = p->head; q->next; q = q->next);
            q->next = calloc (1, sizeof (CarInfoNode));
            memcpy (& (q->next->ci), ci, sizeof (CarInfo));
            //q->next->ci = *ci;
            q->next->next = NULL;
            q->next->head = calloc (1, sizeof (RentOrderNode));
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
                r->next = calloc (1, sizeof (RentOrderNode));
                memcpy (& (r->next->ro), ro, sizeof (RentOrder));
                //r->next->ro = *ro;
                r->next->next = NULL;
                return;
            }
}

char print_json_buffer[BUFFER_LENGTH];

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
    read_db (database_path);
    query_initialize();
}

inline int find_table_by_name (char *table)
{
    for (int i = 0; i < catalog.tc; ++i)
        if (!stricmp (table, catalog.tbls[i].name))
        {
            return i;
        }
    return ERROR;
}

inline int find_column_by_name (int ti, char *col)
{
    for (int i = 0; i < catalog.tbls[ti].cc; ++i)
        if (!stricmp (col, catalog.tbls[ti].cols[i].name))
        {
            return i;
        }
    return ERROR;
}

inline int can_assign (u16 type1, u16 type2)
{
    return (max (type1, type2) <= EXPR_APPROXNUM
            || (min (type1, type2) >= EXPR_STRING
                && max (type1, type2) <= EXPR_DATETIME));
}

inline void update_ct_ptr()
{
    for (CarTypeNode *p = head->next; p; p = p->next)
    {
        if (p->next == NULL)
        {
            ct_ptr = p;
        }
    }
}

inline void *get_val_addr (u16 dest, ExprNode *expr)
{
    if (expr == NULL)
    {
        return NULL;
    }
    switch (expr->type)
    {
    case EXPR_INTNUM:
        if (dest == EXPR_APPROXNUM)
        {
            expr->floatval = expr->intval;
            return & (expr->floatval);
        }
        else
        {
            return & (expr->intval);
        }
    case EXPR_APPROXNUM:
        if (dest == EXPR_INTNUM)
        {
            expr->intval = expr->floatval;
            return & (expr->intval);
        }
        else
        {
            return & (expr->floatval);
        }
    case EXPR_STRING:
    case EXPR_DATETIME:
        return expr->strval;
    }
}