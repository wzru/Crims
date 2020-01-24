# Application Programming Interface                                                                                                                                                                                                                                                                                        

## Connection

前后端通过socket通信，前端启动后可与后端进行socket连接（默认地址：127.0.0.1:8000）

之后可进行命令交互，交互语言为CSQL，具体定义见下

## CSQL

**Crims Structured Query Language(CSQL)**是SQL的一门方言，是专为Crims设计的一门DSL。

### TABLES & COLOUMS Definition

CSQL**大小写敏感**，有且仅有如下TABLES和COLOUMS定义：

#### ①CAR_TYPE

|     code     | tname        | quantity |
| :----------: | ------------ | -------- |
| 车辆类型编码 | 车辆类型名称 | 库存数量 |
|    string    | string       | number   |
| PRIMARY KEY  |              |          |

#### ②CAR_INFO

|            cid             | plate  | code         | cname    | gear     | daily_rent | rent     |
| :------------------------: | ------ | ------------ | -------- | -------- | ---------- | -------- |
|          车辆编号          | 车牌号 | 车辆类型编码 | 车辆名称 | 排挡方式 | 每日租金   | 出租状态 |
|           number           | string | string       | string   | string   | number     | string   |
| PRIMARY KEY, IDENTITY(1,1) |        |              |          |          |            |          |

#### ③RENT_ORDER

| oid         | identity_number | pname    | phone_number | cid          | pickup_time | scheduled_dropoff_time | deposit | actual_dropoff_time | scheduled_fee | actual_fee |
| ----------- | --------------- | -------- | ------------ | ------------ | :---------: | ---------------------- | ------- | ------------------- | ------------- | ---------- |
| 订单编号    | 身份证号        | 客人姓名 | 手机号码     | 租用车辆编号 |  取车时间   | 预约还车时间           | 押金    | 实际还车时间        | 应缴费用      | 实缴缴用   |
| string      | string          | string   | string       | int          |   string    | string                 | number  | string              | number        | number     |
| PRIMARY KEY |                 |          |              |              |             |                        |         |                     |               |            |

### Data Manipulation Language,DML

语句返回值类型均为json

CSQL支持且仅支持如下语法:

#### INSERT INTO

```sql
INSERT INTO <table_name> 
	VALUES (value1, value2, value3, ...);
```

②③均不支持赋值主键列

#### DELETE FROM

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

| 运算符  | 描述                                                       |
| :------ | :--------------------------------------------------------- |
| =       | 等于                                                       |
| <>      | 不等于。**注释：**在 SQL 的一些版本中，该操作符可被写成 != |
| >       | 大于                                                       |
| <       | 小于                                                       |
| >=      | 大于等于                                                   |
| <=      | 小于等于                                                   |
| BETWEEN | 在某个范围内                                               |
| LIKE    | 搜索某种模式                                               |
| IN      | 指定针对某个列的多个可能值                                 |

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

ORDER BY 多列的时候，eg:

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
IMPORT FROM file_path TO table_name;
```

将指定路径的xls文件加入到指定的表中。

#### EXPORT

```sql
EXPORT FROM json TO file_path;
```

将给定的json转成xls存入指定路径

