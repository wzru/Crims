#include <string.h>
#include <stdio.h>

#include "debug.h"
#include "json.h"

inline void input_car_type (CarType *ct)
{

}

inline void input_car_info (CarInfo *ci)
{

}

inline void input_rent_order (RentOrder *ro)
{

}

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