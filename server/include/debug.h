#ifndef DEBUG_H
#define DEBUG_H

#include "define.h"

enum ErrorCode
{
    AMBIGUOUS_COLUMN = -1010,//听说有4位的错误码看着会很厉害
    NO_MATCHING_OPERATOR,
    UNKNOWN_COLUMN,
    UNKNOWN_TABLE,
    UNKNOWN_EXPRESSION,
    TABLE_NOT_EXIST,
    INVALID_FUNCTION_STATEMENT,
    UNKNOWN_ORDERBY_EXPRESSION,
    ERROR = -1,
};

inline void input_car_type (CarType *ct);
inline void input_car_info (CarInfo *ci);
inline void input_rent_order (RentOrder *ro);

inline void output_car_type (CarType *ct);
inline void output_car_info (CarInfo *ci);
inline void output_rent_order (RentOrder *ro);

inline int test_read_write();

#endif