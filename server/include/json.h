#ifndef JSON_H
#define JSON_H

#include "define.h"
#include "select.h"

extern char json_buffer[BUFFER_LENGTH];

inline void jsonify_car_type (CarType *ct, char *json);
inline void jsonify_car_info (CarInfo *ci, char *json);
inline void jsonify_rent_order (RentOrder *ro, char *json);

inline int jsonify_value (char *json, ExprNode *val);
inline void jsonify_result (Records *recs);
inline void jsonify_error (char *type);
inline void jsonify_success (char *type);

#endif