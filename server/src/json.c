#include <stdio.h>

#include "json.h"

inline void jsonify_car_type (CarType *ct, char *json)
{
    sprintf (json, "{\"code\":\"%c\",\"name\":\"%s\",\"quantity\":%d}",
             ct->code, ct->name, ct->quantity);
}

inline void jsonify_car_info (CarInfo *ci, char *json)
{
    sprintf (json,
             "{\"index\":%d,\"plate\":\"%s\",\"code\":\"%c\",\"name\":\"%s\",\"gear\":\"%s\",\"daily_rent\":%f,\"rent\":\"%c\"}",
             ci->index, ci->plate, ci->code, ci->name, ci->gear, ci->daily_rent, ci->rent
            );
}

inline void jsonify_rent_order (RentOrder *ro, char *json)
{
    sprintf (json,
             "{\"index\":\"%s\",\"identity_number\":\"%s\",\"name\":\"%s\",\"phone_number\":\"%s\",\"car_index\":\"%s\",\"pickup_time\":\"%s\",\"scheduled_dropoff_time\":\"%s\",\"deposit\":%f,\"actual_dropoff_time\":\"%s\",\"scheduled_fee\":%f,\"actual_fee\":%f}",
             ro->index, ro->identity_number, ro->name, ro->phone_number, ro->car_index,
             ro->pickup_time, ro->scheduled_dropoff_time, ro->deposit,
             ro->actual_dropoff_time, ro->scheduled_fee, ro->actual_fee
            );
}