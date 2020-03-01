#include <malloc.h>
#include <string.h>

#include "database.h"
#include "define.h"
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
                {"code",     EXPR_STRING, sizeof (char),         0 },
                {"tname",    EXPR_STRING, CAR_TYPE_TNAME_LENGTH, sizeof (char) },
                {"quantity", EXPR_INTNUM, sizeof (int),          sizeof (char) + CAR_TYPE_TNAME_LENGTH }
            }
        },
        {
            "CAR_INFO", 7,
            {
                {"cid",        EXPR_INTNUM,    sizeof (int),                 0 },
                {"plate",      EXPR_STRING,    CAR_INFORMATION_PLATE_LENGTH, sizeof (int) },
                {"code",       EXPR_STRING,    sizeof (char),                sizeof (int) + CAR_INFORMATION_PLATE_LENGTH },
                {"cname",      EXPR_STRING,    CAR_INFORMATION_CNAME_LENGTH, sizeof (int) + CAR_INFORMATION_PLATE_LENGTH + sizeof (char) },
                {"gear",       EXPR_STRING,    CAR_INFORMATION_GEAR_LENGTH,  sizeof (int) + CAR_INFORMATION_PLATE_LENGTH + sizeof (char) + CAR_INFORMATION_CNAME_LENGTH },
                {"daily_rent", EXPR_APPROXNUM, sizeof (float),               sizeof (int) + CAR_INFORMATION_PLATE_LENGTH + sizeof (char) + CAR_INFORMATION_CNAME_LENGTH + CAR_INFORMATION_GEAR_LENGTH },
                {"rent",       EXPR_STRING,    sizeof (char),                sizeof (int) + CAR_INFORMATION_PLATE_LENGTH + sizeof (char) + CAR_INFORMATION_CNAME_LENGTH + CAR_INFORMATION_GEAR_LENGTH + sizeof (float) }
            }
        },
        {
            "RENT_ORDER", 11,
            {
                {"oid",                    EXPR_INTNUM,    RENT_ORDER_OID_LENGTH,             0 },
                {"identity_number",        EXPR_STRING,    RENT_ORDER_IDENTITY_NUMBER_LENGTH, RENT_ORDER_OID_LENGTH },
                {"pname",                  EXPR_STRING,    RENT_ORDER_PNAME_LENGTH,           RENT_ORDER_OID_LENGTH + RENT_ORDER_IDENTITY_NUMBER_LENGTH },
                {"phone_number",           EXPR_STRING,    RENT_ORDER_PHONE_NUMBER_LENGTH,    RENT_ORDER_OID_LENGTH + RENT_ORDER_IDENTITY_NUMBER_LENGTH + RENT_ORDER_PNAME_LENGTH },
                {"cid",                    EXPR_INTNUM,    sizeof (int),                      RENT_ORDER_OID_LENGTH + RENT_ORDER_IDENTITY_NUMBER_LENGTH + RENT_ORDER_PNAME_LENGTH + RENT_ORDER_PHONE_NUMBER_LENGTH },
                {"pickup_time",            EXPR_DATETIME,  RENT_ORDER_TIME_LENGTH,            RENT_ORDER_OID_LENGTH + RENT_ORDER_IDENTITY_NUMBER_LENGTH + RENT_ORDER_PNAME_LENGTH + RENT_ORDER_PHONE_NUMBER_LENGTH + sizeof (int) },
                {"scheduled_dropoff_time", EXPR_DATETIME,  RENT_ORDER_TIME_LENGTH,            RENT_ORDER_OID_LENGTH + RENT_ORDER_IDENTITY_NUMBER_LENGTH + RENT_ORDER_PNAME_LENGTH + RENT_ORDER_PHONE_NUMBER_LENGTH + sizeof (int) + RENT_ORDER_TIME_LENGTH },
                {"deposit",                EXPR_APPROXNUM, sizeof (float),                    RENT_ORDER_OID_LENGTH + RENT_ORDER_IDENTITY_NUMBER_LENGTH + RENT_ORDER_PNAME_LENGTH + RENT_ORDER_PHONE_NUMBER_LENGTH + sizeof (int) + RENT_ORDER_TIME_LENGTH * 2 },
                {"actual_dropoff_time",    EXPR_DATETIME,  RENT_ORDER_TIME_LENGTH,            RENT_ORDER_OID_LENGTH + RENT_ORDER_IDENTITY_NUMBER_LENGTH + RENT_ORDER_PNAME_LENGTH + RENT_ORDER_PHONE_NUMBER_LENGTH + sizeof (int) + RENT_ORDER_TIME_LENGTH * 2 + sizeof (float) },
                {"scheduled_fee",          EXPR_APPROXNUM, sizeof (float),                    RENT_ORDER_OID_LENGTH + RENT_ORDER_IDENTITY_NUMBER_LENGTH + RENT_ORDER_PNAME_LENGTH + RENT_ORDER_PHONE_NUMBER_LENGTH + sizeof (int) + RENT_ORDER_TIME_LENGTH * 3 + sizeof (float) },
                {"scheduled_fee",          EXPR_APPROXNUM, sizeof (float),                    RENT_ORDER_OID_LENGTH + RENT_ORDER_IDENTITY_NUMBER_LENGTH + RENT_ORDER_PNAME_LENGTH + RENT_ORDER_PHONE_NUMBER_LENGTH + sizeof (int) + RENT_ORDER_TIME_LENGTH * 3 + sizeof (float) * 2}
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

inline void print_value (ExprNode *val)
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
            printf ("%d ", val->intval);
            break;
        case EXPR_APPROXNUM:
            printf ("%f ", val->floatval);
            break;
        case EXPR_STRING:
        case EXPR_DATETIME:
            printf ("%s ", val->strval);
            break;
        }
    }
}

char format[JSON_BUFFER_LENGTH];
inline void print_result (Records *recs)
{
    //system("@echo off");
    system ("chcp 65001");
    memset (format, 0, sizeof (format));
    if (recs == NULL || recs->cnt == 0)
    {
        printf ("Empty set\n");
    }
    else
    {
        for (uint i = 0; i < recs->cnt; ++i)
        {
            for (uint j = 0; j < col_cnt; ++j)
            {
                print_value (& (recs->recs[i].item[j]));
            }
            printf("\n");
        }
    }
}