#include <malloc.h>

#include "database.h"
#include "define.h"

inline void read_initialize()
{
    head = malloc (sizeof (CarType));
    ct_ptr = head;
    ci_ptr = NULL;
    ro_ptr = NULL;
}

inline void read_car_type (FILE *stream)
{
    CarTypeNode *ct = malloc (sizeof (CarTypeNode));
    fread ( (void *) & (ct->ct), sizeof (CarType), 1, stream);
    ct_ptr->next = ct;
    ct_ptr = ct;
    ct_ptr->head = ci_ptr = malloc (sizeof (CarInfoNode));
    ro_ptr = NULL;
}

inline void read_car_info (FILE *stream)
{
    CarInfoNode *ci = malloc (sizeof (CarInfoNode));
    fread ( (void *) & (ci->ci), sizeof (CarInfo), 1, stream);
    ci_ptr->next = ci;
    ci_ptr = ci;
    ci_ptr->head = ro_ptr = malloc (sizeof (RentOrderNode));
}

inline void read_rent_order (FILE *stream)
{
    RentOrderNode *ro = malloc (sizeof (RentOrderNode));
    fread ( (void *) & (ro->ro), sizeof (RentOrder), 1, stream);
    ro_ptr->next = ro;
    ro_ptr = ro;
}

inline int read (char *db)
{
    read_initialize();
    FILE *fp = fopen (db, "r");
    if (fp == NULL)
    {
        printf ("File open ERROR!\n");
        return -1;
    }
    char c;
    while ( (c = fgetc (fp)) != feof)
    {
        switch (c)
        {
        case TYPE_CAR:
            read_car_type (fp);
            break;
        case TYPE_INFO:
            read_car_info (fp);
            break;
        case TYPE_ORDER:
            read_rent_order (fp);
            break;
        }
    }
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

inline void recursive_write (CarTypeNode *ct, CarInfoNode *ci,
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
        recursive_write (ct, ct->head->next, ro, TYPE_INFO, stream);
        break;
    case TYPE_INFO:
        if (ci == NULL)
        {
            return recursive_write (ct->next, NULL, NULL, TYPE_CAR, stream);
        }
        write_car_info (& (ci->ci), stream);
        recursive_write (ct, ci, ci->head->next, TYPE_ORDER, stream);
        break;
    case TYPE_ORDER:
        if (ro == NULL)
        {
            return recursive_write (ct, ci->next, NULL, TYPE_INFO, stream);
        }
        write_rent_order (& (ro->ro), stream);
        recursive_write (ct, ci, ro->next, TYPE_ORDER, stream);
        break;
    }
}

inline int write (char *db)
{
    FILE *fp = fopen (db, "w");
    if (fp == NULL)
    {
        printf ("File open ERROR!\n");
        return -1;
    }
    recursive_write (head->next, NULL, NULL, TYPE_CAR, fp);
    is_saved = 1;
    return 0;
}