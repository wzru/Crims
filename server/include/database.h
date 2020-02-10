#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>
#include "define.h"

#define DATABASE_PATH_LENGTH 256
#define DEFAULT_DATABASE_PATH "./.db"
extern char database_path[DATABASE_PATH_LENGTH];

extern CarTypeNode *head;
extern CarTypeNode *ct_ptr;
extern CarInfoNode *ci_ptr;
extern RentOrderNode *ro_ptr;

#define TYPE_CAR 'A'
#define TYPE_INFO 'B'
#define TYPE_ORDER 'C'

extern int is_saved;

inline void read_initialize();//读时初始化数据库

inline void read_car_type (FILE *stream);//从stream读入一个CarType
inline void read_car_info (FILE *stream);//从stream读入一个CarInfo
inline void read_rent_order (FILE *stream);//从stream读入一个RentOrder

inline int read (char *db); //从db路径读入数据
inline int write (char *db); //向db路径写入数据

#endif