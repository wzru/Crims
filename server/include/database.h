#ifndef DATABASE_H
#define DATABASE_H

#include <stdio.h>

#include "define.h"
#include "exec.h"

#define DATABASE_PATH_LENGTH 256
#define DEFAULT_DATABASE_PATH "../data/.db"
extern char database_path[DATABASE_PATH_LENGTH];

extern CarTypeNode *head;
extern CarTypeNode *ct_ptr;
extern CarInfoNode *ci_ptr;
extern RentOrderNode *ro_ptr;

enum
{
    TYPE_CAR = 0,
    TYPE_INFO,
    TYPE_ORDER
};

typedef struct ColumnInfo
{
    char name[COLUMN_NAME_LENGTH];
    u16 type;
    byte size;
    byte offset;
} ColumnInfo;
typedef struct TableInfo
{
    char name[TABLE_NAME_LENGTH];
    byte cc;
    ColumnInfo cols[TABLE_COLUMN_COUNT];
} TableInfo;
typedef struct DatabaseInfo
{
    char name[DATABASE_NAME_LENGTH];
    int tc;
    TableInfo tbls[DATABASE_TABLE_COUNT];
} DatabaseInfo;
extern DatabaseInfo catalog;

extern int is_saved;

extern uint icnt[DATABASE_TABLE_COUNT], isiz[DATABASE_TABLE_COUNT];

inline void database_initialize();

inline void read_initialize();//读时初始化数据库

inline void read_car_type (FILE *stream);//从stream读入一个CarType
inline void read_car_info (FILE *stream);//从stream读入一个CarInfo
inline void read_rent_order (FILE *stream);//从stream读入一个RentOrder

inline int read (char *db); //从db路径读入数据
inline int write (char *db); //向db路径写入数据
inline void recursive_print (CarTypeNode *ct, CarInfoNode *ci,//调试输出
                             RentOrderNode *ro, int type, FILE *stream);

inline void input_car_type (CarType *ct);
inline void input_car_info (CarInfo *ci);
inline void input_rent_order (RentOrder *ro);

inline void print_car_type (CarType *ct, FILE *stream);
inline void print_car_info (CarInfo *ci, FILE *stream);
inline void print_rent_order (RentOrder *ro, FILE *stream);

inline void insert_car_type (CarType *ct);
inline void insert_car_info (CarInfo *ci);
inline void insert_rent_order (RentOrder *ro);

#endif