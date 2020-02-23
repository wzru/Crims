#ifndef DEBUG_H
#define DEBUG_H

#include "define.h"

inline void input_car_type (CarType *ct);
inline void input_car_info (CarInfo *ci);
inline void input_rent_order (RentOrder *ro);

inline void output_car_type (CarType *ct);
inline void output_car_info (CarInfo *ci);
inline void output_rent_order (RentOrder *ro);

inline int debug();

#endif