#ifndef JSON_H
#define JSON_H

#include "define.h"

#define JSON_BUFFER_LENGTH 1024
extern char json_buffer[JSON_BUFFER_LENGTH];

inline void jsonify_car_type (CarType *ct, char *json);
inline void jsonify_car_info (CarInfo *ci, char *json);
inline void jsonify_rent_order (RentOrder *ro, char *json);

#endif