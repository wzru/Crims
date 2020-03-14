# Application Programming Interface                                                                                                                                                                                                                                                                                        

## Connection

前后端通过socket通信，前端可与后端进行socket连接（默认端口：8000）

之后可进行命令交互，交互语言为CSQL，具体定义见下

## CSQL 

**Crims Structured Query Language(CSQL)**是SQL的一门方言，是专为Crims设计的一门DSL。CSQL大小写不敏感。  

### TYPE

| 类型名         | 描述                                                         |
| -------------- | ------------------------------------------------------------ |
| int            | -2147483648 到 2147483647                                    |
| char(size)     | 保存固定长度的字符串。在括号中指定字符串的长度。最多 255 个字符。 |
| float(size, d) | 带有浮动小数点的小数字。在括号中规定最大位数。在 d 参数中规定小数点右侧的最大位数。 |
| datetime       | 日期和时间的组合。格式：YYYY/MM/DD-HH:MM                     |

### TABLES & COLOUMS Definition

CSQL有且仅有如下TABLES和COLOUMS定义：

#### ①CAR_TYPE

|     code     | tname        | quantity |
| :----------: | ------------ | -------- |
| 车辆类型编码 | 车辆类型名称 | 库存数量 |
|    string    | string       | number   |
| PRIMARY KEY  |              |          |

```sql
CREATE TABLE CAR_TYPE
(
    code char(1) NOT NULL PRIMARY KEY,
    tname char(20),
    quantity int
);
```

#### ②CAR_INFO

|            cid             | plate  | code         | cname    | gear     | daily_rent | rent     |
| :------------------------: | ------ | ------------ | -------- | -------- | ---------- | -------- |
|          车辆编号          | 车牌号 | 车辆类型编码 | 车辆名称 | 排挡方式 | 每日租金   | 出租状态 |
|           number           | string | string       | string   | string   | number     | string   |
| PRIMARY KEY, IDENTITY(1,1) |        |              |          |          |            |          |

```sql
CREATE TABLE CAR_INFO
(
	cid int NOT NULL PRIMARY KEY AUTO_INCREMENT,
    plate char(10),
    code char(1),
    cname char(20),
    gear char(10),
    daily_rent float(10,2),
    rent char(1)
); 
```

#### ③RENT_ORDER

| oid         | identity_number | pname    | phone_number | cid          | pickup_time | scheduled_dropoff_time | deposit | actual_dropoff_time | scheduled_fee | actual_fee |
| ----------- | --------------- | -------- | ------------ | ------------ | :---------: | ---------------------- | ------- | ------------------- | ------------- | ---------- |
| 订单编号    | 身份证号        | 客人姓名 | 手机号码     | 租用车辆编号 |  取车时间   | 预约还车时间           | 押金    | 实际还车时间        | 应缴费用      | 实缴缴用   |
| string      | string          | string   | string       | int          |   string    | string                 | number  | string              | number        | number     |
| PRIMARY KEY |                 |          |              |              |             |                        |         |                     |               |            |

```sql
CREATE TABLE RENT_ORDER
(
    oid char(20) NOT NULL PRIMARY KEY,
    identity_number char(20),
    pname char(20),
    phone_number char(20),
    cid int,
    pickup_time datetime,
    scheduled_dropoff_time datetime,
    deposit float(10,2),
    actual_dropoff_time datetime DEFAULT '0',
    scheduled_fee float(10,2),
    actual_fee float(10,2)
);
```

### Data Manipulation Language,DML

CSQL支持且仅支持如下语法:

#### INSERT

```sql
INSERT INTO <table_name> 
	VALUES (value1, value2, value3, ...);
```

②③均不支持赋值主键列

#### DELETE

```sql
DELETE FROM <table_name>
	WHERE some_column=some_value;
```

#### UPDATE

```sql
UPDATE table_name
	SET column1=value1,column2=value2, ...
	WHERE some_column=some_value;
```

#### SELECT

```sql
SELECT column_name,column_name
	FROM table_name;
SELECT * FROM table_name;
```

#### WHERE

```sql
SELECT column_name,column_name
	FROM table_name
	WHERE column_name operator value;
```

下面的运算符可以在 WHERE 子句中使用：

| 运算符  | 描述                                                   |
| :------ | :----------------------------------------------------- |
| =       | 等于                                                   |
| <>      | 不等于。注释：在 SQL 的一些版本中，该操作符可被写成 != |
| >       | 大于                                                   |
| <       | 小于                                                   |
| >=      | 大于等于                                               |
| <=      | 小于等于                                               |
| BETWEEN | 在某个范围内                                           |
| LIKE    | 搜索某种模式                                           |
| IN      | 指定针对某个列的多个可能值                             |

#### LIKE

在 CSQL 中，可使用以下通配符：

| 通配符                         | 描述                       |
| :----------------------------- | :------------------------- |
| %                              | 替代 0 个或多个字符        |
| _                              | 替代一个字符               |
| [*charlist*]                   | 字符列中的任何单一字符     |
| [^*charlist*] 或 [!*charlist*] | 不在字符列中的任何单一字符 |

#### ORDER BY

```sql
SELECT column_name,column_name
	FROM table_name
	ORDER BY column_name,column_name ASC|DESC;
```

ORDER BY 多列的时候，例如:

```sql
ORDER BY A,B        --这个时候都是默认按升序排列
ORDER BY A DESC, B   --这个时候 A 降序，B 升序排列
ORDER BY A, B DESC  --这个时候 A 升序，B 降序排列
```

即 **desc** 或者 **asc** 只对它紧跟着的第一个列名有效，其他不受影响，仍然是默认的升序。

###  Data Control Language,DCL

#### SAVE

```sql
SAVE;
```

保存更改到ROM文件

#### EXIT

```sql
EXIT;
```

退出程序，如果有未保存更改则会提示，否则后台程序退出。

#### IMPORT

```sql
IMPORT FROM file_path INTO table_name;
```

将指定路径的xls文件加入到指定的表中。

#### EXPORT

```sql
EXPORT FROM json INTO file_path;
```

将给定的json转成xls存入指定路径

## Reference

### 车辆分类信息查询功能 

> 每次退出系统前，实现数据的保存功能

```sql
SAVE;
EXIT;
```

> 也可以手工选择菜单进行加载

```sql
IMPORT FROM ./1.xls INTO CAR_TYPE;
```

> 车辆分类信息维护：包括对车辆分类信息的录入、修改和删除等功能。 
>
> 车辆基本信息维护：包括对车辆基本信息的录入、修改和删除等功能。 
>
> 租车订单信息维护：包括对车辆订单信息的录入、修改和删除等功能。

```sql
INSERT INTO CAR_INFO VALUES("鄂A3S880", "1", "大众朗逸", "1.6自动", "135", "y");

INSERT INTO RENT_ORDER VAULES
	(2019021509,
     230101198505050021,
     '王二麻子',
     13412345678,
     16,
     '2019/02/17-13:00',
     '2019/02/16-13:00',
     1105.0,
     '2019/02/17-13:00',
     221.0,
     442.0
    );

UPDATE RENT_ORDER
	SET name="李四", phone_number="13388888888"
	WHERE oid="2019021505";

DELETE FROM CAR_TYPE
	WHERE code='5';
```

> 以车辆类别为条件来查找并显示满足条件的车辆分类信息。例如，查找并显示当前可租赁车辆类型名称为经济型的车辆信息（车辆类型、车辆名称、排挡方式、每日租金）。

```sql
SELECT code, cname, gear, daily_rent 
	FROM CAR_INFO
	WHERE rent='n' 
	AND code IN (SELECT code 
                 FROM CAR_TYPE 
        	     WHERE tname="经济型");
```

> 需支持模糊查询，如未指定车辆类型名称进行查询时，应显示所有当前可租赁车辆信息。

```sql
SELECT code, cname, gear, daily_rent  
	FROM CAR_INFO
	WHERE rent='n';
```

> 需支持综合查询，如同时选定商务型和经济型进行查询时，应显示这两类车型的当前可租赁车辆信息。 

```sql
SELECT code, cname, gear, daily_rent 
	FROM CAR_INFO
	WHERE rent='n'
	AND code IN (SELECT code 
                 FROM CAR_TYPE
                 WHERE tname="经济型" 
                 OR tname="商务型");
```

### 车辆基本信息查询功能

> 以车牌号码为条件，查找并显示满足条件的车辆基本信息。例如，查找并显示车牌号码为“鄂AW123Q”的车辆基本信息。需支持模糊查询，如查询条件为“鄂 A”，则应显示所有车牌号码中含有 “鄂 A”的车辆基本信息。 

```sql
SELECT *
	FROM CAR_INFO	
	WHERE plate LIKE "%鄂A%";
```

> 以车辆名称为条件，查找并显示满足条件的车辆基本信息。例如，查找并显示车辆名称为“别克英朗”的车辆基本信息。需支持模糊查询，如查询条件为“别克”，则应显示车辆名称中含有“别克”的车辆基本信息。 

```sql
SELECT *
	FROM CAR_INFO
	WHERE cname LIKE "%别克%";
```

> 以出租状态为条件，查找并显示满足条件的车辆基本信息。例如，查找并显示当前未出租车辆基本信息。 

```sql
SELECT *
	FROM CAR_INFO
	WHERE rent="n";
```

> 以车牌号码、车辆名称、出租状态多条件组合，查找并显示满足条件的车辆基本信息。例如，查找并显示车辆名称为“别克英朗”且当前未出租的车辆基本信息。 

```sql
SELECT *
	FROM CAR_INFO
	WHERE cname="别克英朗" 
	AND rent="n";
```

### 租车订单信息查询功能

> 按客人信息（身份证号或手机号码）为条件，查找并显示满足条件的客人租车订单信息信息。例如，查找并显示身份证为“23010119920010024”的客人租车订单信息信息。查找并显示手机号码为“13412341234”的客人租车订单信息信息。 

```sql
SELECT *
	FROM RENT_ORDER
	WHERE identity_number="23010119920010024";
	
SELECT *
	FROM RENT_ORDER
	WHERE phone_number="13412341234";
```

> 按车辆信息（车牌号码或车辆名称）为条件，查找并显示满足条件的租车订单信息。例如，查找并显示车牌号码为“鄂AW123Q”的车辆的所有租车订单信息信息。查找并显示车辆名称为“别克英朗” 的车辆的所有租车订单信息信息。 

```sql
SELECT *
	FROM RENT_ORDER
	WHERE cid IN (SELECT cid
                  FROM CAR_INFO
                  WHERE plate="鄂AW123Q");
              
SELECT *
	FROM RENT_ORDER
	WHERE cid IN (SELECT cid
                  FROM CAR_INFO
                  WHERE cname="别克英朗");
```

>以租车时间范围为条件，查找并显示满足条件的所有租车订单信息。例如，查找并显示在 2019年 2 月 11 日至 2019 年 5 月 12 日之间发生的所有租车订单信息

```sql
SELECT *
	FROM RENT_ORDER
	WHERE pickup_time >= '2019/02/11'
	AND pickup_time <= '2019/05/12';
```

### 数据统计

>  统计当前每种车辆类型的车辆总数、已出租数、未出租数。

```sql
SELECT CAR_TYPE.tname, 
	   CAR_TYPE.quantity, 
	   SUM(CAR_INFO.rent='y') AS "已出租数", 
	   SUM(CAR_INFO.rent='n') AS "未出租数"
	FROM CAR_TYPE, CAR_INFO
    WHERE CAR_TYPE.code=CAR_INFO.code
    GROUP BY CAR_TYPE.tname;
```

> 统计每月每种车辆类型的营业额（产生的实缴费用）

```sql
SELECT CAR_TYPE.tname, SUM(RENT_ORDER.actual_fee)
    FROM CAR_TYPE, RENT_ORDER
    WHERE CAR_TYPE.code = (SELECT CAR_INFO.code 
                           FROM CAR_INFO 
                           WHERE cid=RENT_ORDER.cid) 
    AND RENT_ORDER.pickup_time > '2019/2/1' 
    AND RENT_ORDER.pickup_time < '2019/2/28'
    GROUP BY CAR_TYPE.tname;
```

>  输入年份，统计该年每辆车的营业额（产生的实缴费用）、租用率。 

```sql
SELECT CAR_INFO.plate, 
	   CAR_INFO.cname, 
	   SUM(RENT_ORDER.actual_fee) AS "营业额", 
       SUM(TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time))/365 AS "租用率"
       FROM CAR_INFO, RENT_ORDER
       WHERE CAR_INFO.cid=RENT_ORDER.cid
       GROUP BY CAR_INFO.plate;
```

> 列出当年以来累计出租天数最多的 10 辆车的出租信息，按累计出租天数降序排序后输出。

```sql
SELECT CAR_INFO.plate,
	   CAR_INFO.cname,
	   SUM(CASE WHEN RENT_ORDER.pickup_time > '2019/1/1' 
           THEN TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time) END) AS "累计出租天数",
       SUM(CASE WHEN RENT_ORDER.pickup_time > '2019/1/1' 
           THEN RENT_ORDER.actual_fee END) AS "营业额", 
       SUM(CASE WHEN RENT_ORDER.pickup_time > '2019/1/1' 
           THEN TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time) END)/TIMESTAMPDIFF(DAY, '2019/1/1', '2019/5/20') AS "租用率"
       FROM CAR_INFO, RENT_ORDER
       WHERE CAR_INFO.cid=RENT_ORDER.cid
       GROUP BY CAR_INFO.plate
       ORDER BY "累计出租天数" desc, "营业额" desc 
       LIMIT 10;
```

> 列出当年以来累计消费最多的顾客信息，按消费金额降序排序后输出。

```sql
SELECT RENT_ORDER.pname,
	   RENT_ORDER.identity_number,
       RENT_ORDER.phone_number,
       SUM(CASE WHEN RENT_ORDER.pickup_time > '2019/1/1' 
           THEN RENT_ORDER.actual_fee END) AS "消费金额"
       FROM RENT_ORDER
       GROUP BY RENT_ORDER.identity_number
       ORDER BY "消费金额" desc 
       LIMIT 10;
```

