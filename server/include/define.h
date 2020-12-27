#ifndef DEFINE_H
#define DEFINE_H

#define MAJOR_VERSION 1
#define MINOR_VERSION 0
#define PATCH_VERSION 0

//基本数据结构定义
#define CAR_TYPE_CODE_LENGTH 3
#define CAR_TYPE_TNAME_LENGTH 20
struct CarType //车辆分类信息结构体
{
    char code[CAR_TYPE_CODE_LENGTH]; //车辆类型编码, '1'-'5'
    char tname
    [CAR_TYPE_TNAME_LENGTH]; //车辆类型名称：经济型、商务型、豪华型、SUV、7座及以上车型
    int quantity;                //库存数量
};
typedef struct CarType CarType;

#define CAR_INFORMATION_PLATE_LENGTH 10
#define CAR_INFORMATION_CNAME_LENGTH 20
#define CAR_INFORMATION_GEAR_LENGTH 14
struct CarInfo //车辆基本信息结构体
{
    int cid;                                  //车辆编号, 顺序增加
    char plate[CAR_INFORMATION_PLATE_LENGTH]; //车牌号
    char code[CAR_TYPE_CODE_LENGTH];          //车辆类型编码, '1'-'5'
    char cname[CAR_INFORMATION_CNAME_LENGTH]; //车辆名称
    char gear[CAR_INFORMATION_GEAR_LENGTH];   //排挡方式
    float daily_rent;                         //每日租金
    char rent[2];                             //出租状态, 'y' | 'n'
};
typedef struct CarInfo CarInfo;

#define RENT_ORDER_OID_LENGTH 20
#define RENT_ORDER_IDENTITY_NUMBER_LENGTH 20
#define RENT_ORDER_PNAME_LENGTH 20
#define RENT_ORDER_PHONE_NUMBER_LENGTH 20
//#define RENT_ORDER_CAR_INDEX_LENGTH 4
#define RENT_ORDER_TIME_LENGTH 18
struct RentOrder
{
    char oid[RENT_ORDER_OID_LENGTH]; //订单编号,
    //由订单生成时间的年月日+当日订单顺序号组成
    char identity_number[RENT_ORDER_IDENTITY_NUMBER_LENGTH]; //身份证号
    char pname[RENT_ORDER_PNAME_LENGTH];                     //客人姓名
    char phone_number[RENT_ORDER_PHONE_NUMBER_LENGTH];       //手机号码
    int cid;                                             //租用车辆编号
    char pickup_time[RENT_ORDER_TIME_LENGTH];            //取车时间
    char scheduled_dropoff_time[RENT_ORDER_TIME_LENGTH]; //预约还车时间
    float deposit; //押金, 所租车辆应缴费用×5
    char actual_dropoff_time[RENT_ORDER_TIME_LENGTH]; //实际还车时间
    float scheduled_fee; //在预约还车时间前还车, 应缴费用=每日租金×预约租车天数
    float actual_fee; //在预约还车时间后还车, 实缴缴用=每日租金×实际租车天数
};
typedef struct RentOrder RentOrder;

//链表数据结构定义
struct RentOrderNode
{
    RentOrder ro;
    struct RentOrderNode *next;
};
typedef struct RentOrderNode RentOrderNode;

struct CarInfoNode
{
    CarInfo ci;
    RentOrderNode *head;
    struct CarInfoNode *next;
};
typedef struct CarInfoNode CarInfoNode;

struct CarTypeNode
{
    CarType ct;
    CarInfoNode *head;
    struct CarTypeNode *next;
};
typedef struct CarTypeNode CarTypeNode;

typedef unsigned char byte, u8;
typedef unsigned int uint, u32;
typedef unsigned short u16;
typedef unsigned long long ull, datetime, int64, i64;

enum //用来区分当前运行状态
{
    STATUS_SHELL,
    STATUS_SERVER,
    STATUS_EXEC,
    STATUS_EXIT,
    STATUS_ERROR,
    STATUS_UNKNOWN
};
extern byte crims_status;

#define BUFFER_LENGTH 65536
#define PATH_LENGTH 256

#define TABLE_COLUMN_COUNT 20
#define COLUMN_NAME_LENGTH 23
#define TABLE_NAME_LENGTH 11
#define DATABASE_NAME_LENGTH 10
#define DATABASE_TABLE_COUNT 3

#ifndef max
    #define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
    #define min(a, b) ((a) > (b) ? (b) : (a))
#endif

typedef void * (*pf) (void *);

#if defined(_WIN16) || defined(_WIN32) || defined(_WIN64)
#define OS_WIN
#else
#define OS_LINUX
    #ifndef stricmp
        #define stricmp strcasecmp
    #endif
    #ifndef sprintf_s
        #define sprintf_s snprintf
    #endif
    #ifndef Sleep
        #define Sleep sleep
    #endif
    #ifndef CLK_TCK
        #define CLK_TCK _SC_CLK_TCK
    #endif
#endif

extern char *help_info;

#endif