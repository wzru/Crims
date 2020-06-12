# SQL Executor Feature List

## SELECT

- [x]  ```sql
  SELECT code FROM CAR_TYPE;
  ```

- [x] ```sql
  SELECT code, cname, gear, daily_rent 
    FROM CAR_INFO
     	WHERE code IN (SELECT code 
                    FROM CAR_TYPE 
           	     WHERE tname="经济型");
  ```

- [x] ```sql
  SELECT code, cname, gear, daily_rent 
    	FROM CAR_INFO
      	WHERE rent='n' 
      	AND code IN (SELECT code 
                     FROM CAR_TYPE 
            	     WHERE tname="商务型");
  ```

- [x] ```sql
  SELECT code, cname, gear, daily_rent  
    	FROM CAR_INFO
      	WHERE rent='n';
  ```

- [x] ```sql
  SELECT * FROM CAR_TYPE;
  ```

- [x] ```sql
  SELECT * FROM CAR_TYPE, CAR_INFO;
  ```

- [x] ```sql
  SELECT code, cname, gear, daily_rent 
    	FROM CAR_INFO
      	WHERE rent='n'
      	AND code IN (SELECT code 
                     FROM CAR_TYPE
                     WHERE tname="经济型" 
                     OR tname="商务型");
  ```

- [x] ```sql
  SELECT *
    	FROM CAR_INFO	
      	WHERE plate LIKE "%鄂A%";
  ```

- [x] ```sql
  SELECT *
    	FROM CAR_INFO
      	WHERE cname LIKE "%别克%";
  ```

- [x] ```sql
  SELECT *
    	FROM CAR_INFO
      	WHERE rent="n";
  ```

- [x] ```sql
  SELECT cid, cname
    	FROM CAR_INFO
      	WHERE cname = "别克英朗" 
      	AND rent="y";
  ```
- [x] ```sql
  SELECT cid, cname AS "名字", daily_rent*30 AS "包月价格"
    	FROM CAR_INFO
      	WHERE cname = "别克英朗" 
      	AND rent="y";
  ```

- [x] ```sql
  SELECT *
    	FROM CAR_INFO
      	WHERE cname = "别克英朗" 
      	AND rent="n";
  ```

- [x] ```sql
  SELECT *
    	FROM RENT_ORDER
      	WHERE identity_number="470291339330808704";
  ```

- [x] ```sql
  SELECT *
    	FROM RENT_ORDER
      	WHERE cid IN (SELECT cid
                      FROM CAR_INFO
                      WHERE plate="鄂AW123Q");
  ```

- [x] ```sql
  SELECT *
    	FROM RENT_ORDER
      	WHERE pickup_time <= '2019-02-15/13:05';
  ```

- [x] ```sql
  SELECT * FROM RENT_ORDER
      	WHERE pickup_time >= '2019-02-11'
      	AND pickup_time <= '2019-05-12';
  ```

- [x] ```sql
  SELECT CAR_TYPE.code FROM CAR_TYPE, CAR_INFO WHERE CAR_TYPE.code=CAR_INFO.code;
  ```

- [x] ```sql
  SELECT SUM(daily_rent) AS "类型总日租" FROM CAR_INFO GROUP BY code;
  ```

- [x] ```sql
  SELECT code, SUM(daily_rent) AS "类型总日租" FROM CAR_INFO GROUP BY code;
  ```

- [x] ```sql
  SELECT SUM(CAR_INFO.rent='y') AS "已出租数"
         FROM CAR_TYPE, CAR_INFO
         WHERE CAR_TYPE.code=CAR_INFO.code
         GROUP BY CAR_TYPE.tname;
  ```

- [x] ```sql
  SELECT CAR_TYPE.tname, 
    	   CAR_TYPE.quantity, 
    	   SUM(CAR_INFO.rent='y') AS "已出租数", 
    	   SUM(CAR_INFO.rent='n') AS "未出租数"
    	  FROM CAR_TYPE, CAR_INFO
        WHERE CAR_TYPE.code=CAR_INFO.code
        GROUP BY CAR_TYPE.tname;
  ```

- [x] ```sql
  SELECT CAR_TYPE.tname, SUM(RENT_ORDER.actual_fee)
        FROM CAR_TYPE, RENT_ORDER
        WHERE CAR_TYPE.code IN (SELECT CAR_INFO.code 
                               FROM CAR_INFO 
                               WHERE cid=RENT_ORDER.cid) 
        AND RENT_ORDER.pickup_time > '2019-2-1' 
        AND RENT_ORDER.pickup_time < '2019-2-28'
        GROUP BY CAR_TYPE.tname;
  ```

- [x] ```sql
  SELECT CAR_INFO.plate, 
    	   CAR_INFO.cname, 
    	   SUM(RENT_ORDER.actual_fee) AS "营业额", 
           SUM(TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time)) AS "租用天数"
           FROM CAR_INFO, RENT_ORDER
           WHERE CAR_INFO.cid=RENT_ORDER.cid
           GROUP BY CAR_INFO.plate;
  ```

- [x] ```sql
  SELECT CAR_INFO.plate, 
    	   CAR_INFO.cname, 
    	   SUM(RENT_ORDER.actual_fee) AS "营业额", 
         SUM(TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time))/365.0 AS "租用率"
           FROM CAR_INFO, RENT_ORDER
           WHERE CAR_INFO.cid=RENT_ORDER.cid
           GROUP BY CAR_INFO.plate;
  ```

- [x] ```sql
  SELECT CAR_INFO.plate,
    	   CAR_INFO.cname,
    	   SUM(TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time)) AS "rdays",
           SUM(RENT_ORDER.actual_fee) AS "营业额", 
           SUM(TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time))/TIMESTAMPDIFF(DAY, '2019-1-1', '2019-5-20') AS "租用率"
           FROM CAR_INFO, RENT_ORDER
           WHERE CAR_INFO.cid=RENT_ORDER.cid
           AND RENT_ORDER.pickup_time > '2019-1-1' 
           GROUP BY CAR_INFO.plate
           ORDER BY "rdays" desc
           LIMIT 10;
  ```

- [x] ```sql
  SELECT RENT_ORDER.pname,
    	   RENT_ORDER.identity_number,
         RENT_ORDER.phone_number,
         SUM(RENT_ORDER.actual_fee) AS "money"
           FROM RENT_ORDER
           WHERE RENT_ORDER.pickup_time > '2019-1-1'
           GROUP BY RENT_ORDER.identity_number
           ORDER BY money desc 
           LIMIT 10;
  ```

## INSERT

- [x] ```sql
  INSERT INTO CAR_TYPE VALUES ('6', '跑车', 100);
  ```

- [x] ```sql
  INSERT INTO CAR_TYPE VALUES ('6', '跑车', 100), ('7', '卡丁车', 300);
  ```

- [x] ```sql
  SELECT * FROM CAR_TYPE;
  INSERT INTO CAR_TYPE VALUES ('6', '跑车', 100);
  SELECT * FROM CAR_TYPE;
  ```

## DELETE

- [x] ```sql
  INSERT INTO CAR_TYPE VALUES ('6', '跑车', 100), ('7', '卡丁车', 300); 
  SELECT * FROM CAR_TYPE;
  DELETE FROM CAR_TYPE WHERE code='6';
  SELECT * FROM CAR_TYPE;
  ```

- [x] ```sql
  INSERT INTO CAR_TYPE VALUES ('6', '跑车', 100), ('7', '卡丁车', 300); 
  SELECT * FROM CAR_TYPE;
  DELETE FROM CAR_TYPE WHERE code='6';
  SELECT * FROM CAR_TYPE;
  INSERT INTO CAR_TYPE VALUES ('6', '摩托车', 400);
  SELECT * FROM CAR_TYPE;
  ```

- [x] ```sql
  DELETE FROM CAR_TYPE WHERE code='5';
  DELETE FROM CAR_TYPE WHERE code='4';
  DELETE FROM CAR_TYPE WHERE code='3';
  DELETE FROM CAR_TYPE WHERE code='2';
  DELETE FROM CAR_TYPE WHERE code='1';
  SELECT * FROM CAR_TYPE;
  ```

- [x] ```sql
  DELETE FROM CAR_TYPE WHERE code='5';
  INSERT INTO CAR_TYPE VALUES ('6', '跑车', 100);
  SELECT * FROM CAR_TYPE;
  ```

