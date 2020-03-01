#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug.h"
#include "json.h"
#include "database.h"

inline void output_car_type (CarType *ct)
{
    // printf ("code:'%c'\n", ct->code);
    // printf ("name:'%*s'\n", CAR_TYPE_NAME_LENGTH, ct->name);
    // printf ("quantity:%d\n", ct->quantity);
    memset (json_buffer, 0, JSON_BUFFER_LENGTH);
    jsonify_car_type (ct, json_buffer);
    printf ("%s\n", json_buffer);
}

inline void output_car_info (CarInfo *ci)
{
    // printf ("index:%d\n", ci->index);
    // printf ("plate:'%*s'\n", CAR_INFORMATION_PLATE_LENGTH, ci->plate);
    memset (json_buffer, 0, JSON_BUFFER_LENGTH);
    jsonify_car_info (ci, json_buffer);
    printf ("%s\n", json_buffer);
}

inline void output_rent_order (RentOrder *ro)
{
    memset (json_buffer, 0, JSON_BUFFER_LENGTH);
    jsonify_rent_order (ro, json_buffer);
    printf ("%s\n", json_buffer);
}

inline int test_read_write() //用来测试读写是否正常
{
    read (database_path);
    char s[20];
    int type;
    CarType ct;
    CarInfo ci;
    RentOrder ro;
    if (0)
    {
        freopen ("../data/test_input.txt", "r", stdin);
        while (scanf ("%s", s) != EOF)
        {
            puts ("Please input...");
            if (!strcmp (s, "input"))
            {
                scanf ("%d", &type);
                if (type == 1)
                {
                    input_car_type (&ct), insert_car_type (&ct);
                }
                else if (type == 2)
                {
                    input_car_info (&ci), insert_car_info (&ci);
                }
                else if (type == 3)
                {
                    input_rent_order (&ro), insert_rent_order (&ro);
                }
            }
            else
            {
                break;
            }
        }
    }
    system ("chcp 65001");
    recursive_print (head->next, NULL, NULL, TYPE_CAR, stdout);
    write (database_path);
}
