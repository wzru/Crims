#include <stdio.h>

#include "json.h"

inline void jsonify_car_type (CarType *ct, char *json)
{
    sprintf (json, "{\"code\":\"%c\",\"tname\":\"%s\",\"quantity\":%d}",
             ct->code, ct->tname, ct->quantity);
}

inline void jsonify_car_info (CarInfo *ci, char *json)
{
    sprintf (json,
             "{\"cid\":%d,\"plate\":\"%s\",\"code\":\"%c\",\"cname\":\"%s\",\"gear\":\"%s\",\"daily_rent\":%f,\"rent\":\"%c\"}",
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