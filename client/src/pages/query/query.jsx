import React, { useState } from "react";
import css from "./query.module.scss";
import { DEFAULT, CTQUERY, CIQUERY, ROQUERY } from "../../constants/sheet";
import downIcon from '../../assets/down.svg';
import downIconG from '../../assets/down-g.svg';
import writeWorkbookToLocalFile from "../../utils/writeFile";

const ws2 = new WebSocket('ws://localhost:5002');
ws2.onopen = function () {
  console.log('2open')
}
const ws3 = new WebSocket('ws://localhost:5003');
ws3.onopen = function () {
  console.log('3open');
}

export default function Query() {
  const [advanced, setAdvanced] = useState(false);
  const [type, setType] = useState(DEFAULT);
  const [active, setActive] = useState(false);
  // 数据表格
  const [carType, setCarType] = useState([]);
  const [carInfo, setCarInfo] = useState([]);
  const [rentOrder, setRentOrder] = useState([]);
  // 子选项
  const [tnameOption, setTnameOption] = useState([]);
  const rentOption = [{ name: '已出租', value: 'y' }, { name: '未出租', value: 'n' }];
  // 用户输入的值
  // 车辆类型表
  const [ctTname, setCtTname] = useState([]);
  const [ctRent, setCtRent] = useState([]);
  // 车辆信息表
  const [ciPlate, setCiPlate] = useState('');
  const [ciCname, setCiCname] = useState('');
  const [ciRent, setCiRent] = useState([]);
  // 租车订单表
  const [roId, setRoId] = useState('');
  const [roPhone, setRoPhone] = useState('');
  const [roPlate, setRoPlate] = useState('');
  const [roCname, setRoCname] = useState('');
  const [roTime, setRoTime] = useState({ begin: '', end: '' });
  // 高级查询
  const [sql, setSql] = useState('');
  const [sheetHeader, setSheetHeader] = useState([]);
  const [sheetData, setSheetData] = useState([]);

  const selectSheet = (sheetType) => {
    setType(sheetType);
    setActive(false);
    if (sheetType === CTQUERY) {
      // 请求到所有的车辆类型
      ws3.send(`SELECT tname FROM CAR_TYPE;`);
      ws3.onmessage = function (msg) {
        const { data } = JSON.parse(msg.data);
        console.log(data);
        const newTnameOption = [];
        for (let item of data) {
          newTnameOption.push(item['tname']);
        }
        setTnameOption(newTnameOption);
      }
    }
    if (sheetType === CIQUERY) {

    }
    if (sheetType === ROQUERY) {

    }
  }
  const clickSubOption = (suboption, id) => {
    if (suboption === 'ctTname') {
      const newCtTname = ctTname.slice(0);
      const index = newCtTname.indexOf(id);
      if (index === -1) {
        newCtTname.push(id);
      }
      else {
        newCtTname.splice(index, 1);
      }
      setCtTname(newCtTname);
    }
    if (suboption === 'ctRent') {
      const newCtRent = ctRent.slice(0);
      const index = newCtRent.indexOf(id);
      if (index === -1) {
        newCtRent.push(id);
      }
      else {
        newCtRent.splice(index, 1);
      }
      setCtRent(newCtRent);
    }
    if (suboption === 'ciRent') {
      const newCiRent = ciRent.slice(0);
      const index = newCiRent.indexOf(id);
      if (index === -1) {
        newCiRent.push(id);
      }
      else {
        newCiRent.splice(index, 1);
      }
      setCiRent(newCiRent);
    }
  }
  const onClickQuery = () => {
    // 非高级查询
    if (!advanced) {
      if (type === CTQUERY) {
        // tname
        let tnameSql = '';
        let tname = ctTname.length ? ctTname.slice(0) : Array.from(new Array(tnameOption.length).keys());
        tname.forEach(item => tnameSql += ` tname='${tnameOption[item]}' OR`)
        tnameSql = tnameSql.slice(0, -2);
        // sql
        const sql = `
        SELECT code, cname, gear, daily_rent
          FROM CAR_INFO
          WHERE ${ctRent.length === 1 ? `rent='${rentOption[ctRent[0]].value}'` : ''}
          ${ctRent.length === 1 ? 'AND' : ''} code IN (SELECT code FROM CAR_TYPE WHERE${tnameSql});`
        ws2.send(sql);
        console.log(sql);
        ws2.onmessage = function (msg) {
          console.log(msg.data);
          setCarType(JSON.parse(msg.data).data);
        }
      }
      if (type === CIQUERY) {
        // rent
        let rentSql = '';
        let rent = ciRent.length ? ciRent.slice(0) : Array.from(new Array(rentOption.length).keys());
        rent.forEach(item => rentSql += ` rent='${rentOption[item].value}' OR`);
        rentSql = rentSql.slice(0, -2);
        const sql = `
        SELECT *
          FROM CAR_INFO
          WHERE plate LIKE '.*${ciPlate}.*'
          AND cname LIKE '.*${ciCname}.*'
          ${ciRent.length === 1 ? `AND rent='${rentOption[ciRent[0]].value}'` : ''} ;`

        console.log(sql);
        ws2.send(sql);
        ws2.onmessage = function (msg) {
          console.log(JSON.parse(msg.data).data)
          setCarInfo(JSON.parse(msg.data).data);
        }
      }
      if (type === ROQUERY) {
        console.log(roTime);
        const sql = `
        SELECT *
          FROM RENT_ORDER
          WHERE identity_number LIKE '.*${roId}.*'
          AND phone_number LIKE '.*${roPhone}.*'
          AND pickup_time >= '${roTime.begin}' AND pickup_time <= '${roTime.end}'
          AND cid in (SELECT cid FROM CAR_INFO WHERE plate LIKE '.*${roPlate}.*' AND cname LIKE '.*${roCname}.*');`
        ws2.send(sql);
        ws2.onmessage = function (msg) {
          console.log(JSON.parse(msg.data).data)
          setRentOrder(JSON.parse(msg.data).data);
        }
      }
    }
    // 高级查询
    else {
      console.log(sql);
      ws2.send(sql);
      ws2.onmessage = function (msg) {
        console.log(JSON.parse(msg.data).data)
        const data = JSON.parse(msg.data).data;
        let newHeader = [];
        if (data.length) {
          for (let key in data[0]) {
            newHeader.push(key);
          }
        }
        setSheetHeader(newHeader);
        setSheetData(data);
      }
    }
  }
  const exportFile = () => {
    if (!advanced) {
      if (type === CTQUERY) {
        writeWorkbookToLocalFile(carType, type);
      }
      if (type === CIQUERY) {
        writeWorkbookToLocalFile(carInfo, type);
      }
      if (type === ROQUERY) {
        writeWorkbookToLocalFile(rentOrder, type);
      }
    }
    else {
      writeWorkbookToLocalFile(sheetData, '高级查询');
    }
  }
  return <div className={css['index']}>
    {/* 下拉菜单 */}
    <div
      className={css['options']}
      onMouseLeave={() => setActive(false)}
      style={{ visibility: advanced ? 'hidden' : 'visible' }}
    >
      <div
        className={css['options-cur']}
        onMouseOver={() => setActive(true)}
      >
        {type}
        <img alt='' className={css['options-cur-icon']} src={active ? downIcon : downIconG} />
      </div>
      <div className={css['options-all']} style={{ height: active ? '125px' : '' }}>
        <div className={css['options-all-item']} onClick={() => selectSheet(CTQUERY)}>{CTQUERY}</div>
        <div className={css['options-all-item']} onClick={() => selectSheet(CIQUERY)}>{CIQUERY}</div>
        <div className={css['options-all-item']} onClick={() => selectSheet(ROQUERY)}>{ROQUERY}</div>
      </div>
    </div>
    {/* 导出按钮 */}
    <div className={css['buttons-right']}>
      <div className={css['buttons-right-item']} onClick={exportFile}><span>导出</span><img alt='' src={require('../../assets/download.svg')} /></div>
      <div
        className={css['buttons-right-item']}
        style={{ backgroundColor: advanced ? "#fa8c16" : '#2295ff' }}
        onClick={() => { setAdvanced(!advanced) }}
      >
        <span>高级查询</span>
      </div>
    </div>
    {/* 按钮 */}
    <div className={css['query-component']}>
      {type === CTQUERY && !advanced &&
        <div className={css['suboptions']}>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>车辆类别</div>
            <div className={css['suboptions-item-content']}>
              {tnameOption.map((item, index) => <div
                key={item}
                className={`${css['button']} ${ctTname.indexOf(index) === -1 ? css['unselected'] : css['selected']}`}
                onClick={() => clickSubOption('ctTname', index)}
              >
                {item}
              </div>)}
            </div>
          </div>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>出租状态</div>
            <div className={css['suboptions-item-content']}>
              {rentOption.map((item, index) => <div
                key={item.name}
                className={`${css['button']} ${ctRent.indexOf(index) === -1 ? css['unselected'] : css['selected']}`}
                data-value={item.value}
                onClick={() => clickSubOption('ctRent', index)}
              >
                {item.name}
              </div>)}
            </div>
          </div>
        </div>
      }
      {
        type === CIQUERY && !advanced &&
        <div className={css['suboptions']}>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>车牌号</div>
            <div className={css['suboptions-item-content']}><input onInput={(e) => setCiPlate(e.target.value)} /></div>
          </div>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>车辆名称</div>
            <div className={css['suboptions-item-content']}>
              <input onInput={(e) => setCiCname(e.target.value)} />
            </div>
          </div>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>出租状态</div>
            <div className={css['suboptions-item-content']}>
              {rentOption.map((item, index) => <div
                key={item.name}
                className={`${css['button']} ${ciRent.indexOf(index) === -1 ? css['unselected'] : css['selected']}`}
                data-value={item.value}
                onClick={() => clickSubOption('ciRent', index)}
              >
                {item.name}
              </div>)}
            </div>
          </div>
        </div>
      }
      {
        type === ROQUERY && !advanced &&
        <div className={css['suboptions']} hidden={advanced}>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>身份证号</div>
            <div className={css['suboptions-item-content']}>
              <input onInput={(e) => setRoId(e.target.value)} />
            </div>
          </div>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>手机号码</div>
            <div className={css['suboptions-item-content']}>
              <input onInput={(e) => setRoPhone(e.target.value)} />
            </div>
          </div>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>车牌号码</div>
            <div className={css['suboptions-item-content']}>
              <input onInput={(e) => setRoPlate(e.target.value)} />
            </div>
          </div>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>车辆名称</div>
            <div className={css['suboptions-item-content']}>
              <input onInput={(e) => setRoCname(e.target.value)} />
            </div>
          </div>
          <div className={css['suboptions-item']}>
            <div className={css['suboptions-item-title']}>租车时间范围</div>
            <div className={css['suboptions-item-content']}>
              <input type='date' onInput={(e) => setRoTime({ ...roTime, begin: e.target.value })} />
              <input type='date' onInput={(e) => setRoTime({ ...roTime, end: e.target.value })} />
            </div>
          </div>
        </div>

      }
      {/* 高级查询 */}
      {
        advanced &&
        <div className={css['suboptions']}>
          <textarea
            className={css['suboptions-textarea']}
            placeholder='请输入SQL语句'
            onInput={(e) => setSql(e.target.value)}
          />
        </div>
      }
      <div className={css['query-button']} onClick={onClickQuery}>查询</div>
    </div>
    {/* 表格 */}
    {type === CTQUERY && !advanced && <div className={css['table']}>
      <div className={`${css['table-item']} ${css['table-title']} `}>
        <div className={css['table-item-code']}>车辆类型编码</div>
        <div className={css['table-item-cname']}>车辆名称</div>
        <div className={css['table-item-gear']}>排挡方式</div>
        <div className={css['table-item-daily']}>每日租金</div>
      </div>
      {carType.map((item, index) => <div
        key={index}
        className={css['table-item']}
      >
        <div className={css['table-item-code']}>{item.code}</div>
        <div className={css['table-item-cname']}>{item.cname}</div>
        <div className={css['table-item-gear']}>{item.gear}</div>
        <div className={css['table-item-daily']}>{item.daily_rent}</div>
      </div>)}
    </div>}
    {type === CIQUERY && !advanced && <div className={css['table']}>
      <div className={`${css['table-item']} ${css['table-title']} `}>
        <div className={css['table-item-cid']}>车辆编号</div>
        <div className={css['table-item-plate']}>车牌号</div>
        <div className={css['table-item-code']}>车辆类型编码</div>
        <div className={css['table-item-cname']}>车辆名称</div>
        <div className={css['table-item-gear']}>排挡方式</div>
        <div className={css['table-item-daily']}>每日租金</div>
        <div className={css['table-item-rent']}>出租状态</div>
      </div>
      {carInfo.map((item, index) => {
        return <div
          key={item.cid}
          className={css['table-item']}
        >
          <div className={css['table-item-cid']}>{item.cid}</div>
          <div className={css['table-item-plate']}>{item.plate}</div>
          <div className={css['table-item-code']}>{item.code}</div>
          <div className={css['table-item-cname']}>{item.cname}</div>
          <div className={css['table-item-gear']}>{item.gear}</div>
          <div className={css['table-item-daily']}>{item.daily_rent}</div>
          <div className={`${css['table-item-rent']} ${item.rent === 'y' ? css['table-item-rented'] : css['table-item-unrented']} `}>
            {item.rent === 'y' ? '已出租' : '未出租'}
          </div>
        </div>
      })}
    </div>}
    {type === ROQUERY && !advanced && <div className={css['table']}>
      <div className={`${css['table-item']} ${css['table-title']} `}>
        <div className={css['table-item-oid']}>订单编号</div>
        <div className={css['table-item-identity']}>身份证号</div>
        <div className={css['table-item-pname']}>客人姓名</div>
        <div className={css['table-item-phone']}>手机号码</div>
        <div className={css['table-item-cid']}>租用车辆编号</div>
        <div className={css['table-item-pickup']}>取车时间</div>
        <div className={css['table-item-scheduledtime']}>预约还车时间</div>
        <div className={css['table-item-deposit']}>押金</div>
        <div className={css['table-item-actualtime']}>实际还车时间</div>
        <div className={css['table-item-scheduledfee']}>应缴费用</div>
        <div className={css['table-item-actualfee']}>实缴费用</div>
      </div>
      {
        rentOrder.map((item, index) => {
          return <div
            key={item.oid}
            className={css['table-item']}
          >
            <div className={css['table-item-oid']}>{item.oid}</div>
            <div className={css['table-item-identity']}>{item.identity_number}</div>
            <div className={css['table-item-pname']}>{item.pname}</div>
            <div className={css['table-item-phone']}>{item.phone_number}</div>
            <div className={css['table-item-cid']}>{item.cid}</div>
            <div className={css['table-item-pickup']}>{item.pickup_time}</div>
            <div className={css['table-item-scheduledtime']}>{item.scheduled_dropoff_time}</div>
            <div className={css['table-item-deposit']}>{item.deposit}</div>
            <div className={css['table-item-actualtime']}>{item.actual_dropoff_time}</div>
            <div className={css['table-item-scheduledfee']}>{item.scheduled_fee}</div>
            <div className={css['table-item-actualfee']}>{item.actual_fee}</div>
          </div>
        })
      }
    </div>}
    {(sheetHeader.length !== 0 && advanced) &&
      <div className={css['table']}>
        <div className={`${css['table-item']} ${css['table-title']} `}>
          {sheetHeader.map((item, index) => (
            <div key={index} className={css['table-item-items']}>{item}</div>
          ))}
        </div>
        {sheetData.map((item, index) => {
          return (
            <div key={index} className={`${css['table-item']}`}>
              {sheetHeader.map(key => {
                return (
                  <div key={key} className={css['table-item-items']}>{item[key]}</div>
                )
              })}
            </div>
          )
        })}
      </div>
    }
    {sheetHeader.length === 0 && advanced &&
      < div style={{ color: '#2295ff' }}>没有数据喔</div>
    }
  </div >
}