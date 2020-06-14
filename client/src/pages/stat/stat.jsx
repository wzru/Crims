import React, { useState, useEffect } from "react";
import css from "./stat.module.scss";
import { RENT, MTO, OCP, RANK } from "../../constants/stat";
import COLOR from '../../constants/color';
import downIcon from '../../assets/down.svg';
import downIconG from '../../assets/down-g.svg';
import { Bar } from "react-chartjs-2";

const ws4 = new WebSocket('ws://localhost:5004');
ws4.onopen = function () {
  console.log('4open')
}

export default function STAT() {
  const [type, setType] = useState(MTO);
  // 数据
  const [rentStat, setRentStat] = useState([]);
  const [mtoStat, setMtoStat] = useState([]);
  const [mtoChart, setMtoChart] = useState({});
  const [ocpStat, setOcpStat] = useState([]);
  const [rankStat, setRankStat] = useState([]);
  const [rankChart, setRankChart] = useState([]);
  const [active, setActive] = useState(false);

  const [mtoTime, setMtoTime] = useState('');
  const [ocpTime, setOcpTime] = useState('');

  const [curTime, setCurTime] = useState({ hour: '', minute: '' });

  useEffect(() => {
    updateTime();
  }, [])

  const updateTime = () => {
    const hour = new Date().getHours();
    const minute = new Date().getMinutes();
    setCurTime({ hour: hour < 10 ? `0${hour}` : hour, minute: minute < 10 ? `0${minute}` : minute })
  }

  const selectSheet = (sheetType) => {
    setType(sheetType);
    setActive(false);
    updateTime();
    if (sheetType === RENT) {
      ws4.send(`
      SELECT CAR_TYPE.tname AS "车辆类型名称",
        CAR_TYPE.quantity AS "车辆总数", 
        SUM(CAR_INFO.rent='y') AS "已出租数",
        SUM(CAR_INFO.rent='n') AS "未出租数"
        FROM CAR_TYPE, CAR_INFO
        WHERE CAR_TYPE.code=CAR_INFO.code
        GROUP BY CAR_TYPE.tname;`
      );
      ws4.onmessage = function (msg) {
        const { data } = JSON.parse(msg.data);
        console.log(data);
        setRentStat(data);
      }
    }
    if (sheetType === RANK) {
      // const year = new Date().getFullYear();
      const year = '2019';
      ws4.send(`
      SELECT CAR_INFO.plate AS "车牌号",
        CAR_INFO.cname AS "车辆名称",
        SUM(TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time)) AS "sum",
        SUM(TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time))/365*100 AS "租用率",
        SUM(RENT_ORDER.actual_fee) AS "营业额"
        FROM CAR_INFO, RENT_ORDER
        WHERE CAR_INFO.cid=RENT_ORDER.cid
        AND RENT_ORDER.pickup_time > '${year}-1-1' 
        GROUP BY CAR_INFO.plate
        ORDER BY "sum" DESC
        LIMIT 10;
      `)
      ws4.onmessage = function (msg) {
        const { data } = JSON.parse(msg.data);
        console.log(data);
        setRankStat(data.sort((a, b) => b['sum'] - a['sum']));
        // 图标数据
        const chartLables = [];
        const chartData = [];
        for (let item of data) {
          chartLables.push(item['车牌号']);
          chartData.push(item['营业额']);
        }
        setRankChart({
          labels: chartLables,
          datasets: [{
            label: `${year}年月营业额（元人民币）`,
            backgroundColor: COLOR[Math.floor(Math.random() * COLOR.length)],
            data: chartData
          }]
        });
      }
    }
  }
  const onClickQuery = () => {
    updateTime();
    if (type === MTO) {
      const [year, month] = mtoTime.split('-');
      ws4.send(`
      SELECT CAR_TYPE.tname AS "车辆类型", 
        SUM(RENT_ORDER.actual_fee) AS "营业额"
        FROM CAR_TYPE, CAR_INFO, RENT_ORDER
        WHERE CAR_TYPE.code = CAR_INFO.code
        AND CAR_INFO.cid = RENT_ORDER.cid
        AND RENT_ORDER.pickup_time > '${mtoTime}-1'
        AND RENT_ORDER.pickup_time < '${mtoTime}-${new Date(year, month, 0).getDate()}'
        GROUP BY CAR_TYPE.tname;
    `);
      ws4.onmessage = function (msg) {
        const { data } = JSON.parse(msg.data);
        console.log(data);
        setMtoStat(data);
        // 图标数据
        const chartLables = [];
        const chartData = [];
        for (let item of data) {
          chartLables.push(item['车辆类型']);
          chartData.push(item['营业额']);
        }
        setMtoChart({
          labels: chartLables,
          datasets: [{
            label: `${year}年${month}月营业额（元人民币）`,
            backgroundColor: COLOR[Math.floor(Math.random() * COLOR.length)],
            data: chartData
          }]
        });
      }
    }
    if (type === OCP) {
      ws4.send(`
      SELECT CAR_INFO.plate AS "车牌号", 
        CAR_INFO.cname AS "车辆名称", 
        SUM(RENT_ORDER.actual_fee) AS "营业额", 
        SUM(TIMESTAMPDIFF(DAY, RENT_ORDER.pickup_time, RENT_ORDER.actual_dropoff_time))/365*100 AS "租用率"
        FROM CAR_INFO, RENT_ORDER
        WHERE CAR_INFO.cid=RENT_ORDER.cid
        AND RENT_ORDER.pickup_time >= '${ocpTime}-01-01' AND RENT_ORDER.pickup_time <= '${ocpTime}-12-31'
        GROUP BY CAR_INFO.plate;
      `);
      ws4.onmessage = function (msg) {
        const { data } = JSON.parse(msg.data);
        console.log(data);
        setOcpStat(data);
      }
    }
  }

  return <div className={css['index']}>
    {/* 下拉菜单 */}
    <div
      className={css['options']}
      onMouseLeave={() => setActive(false)}
    >
      <div
        className={css['options-cur']}
        onMouseOver={() => setActive(true)}
      >
        {type}
        <img alt='' className={css['options-cur-icon']} src={active ? downIcon : downIconG} />
      </div>
      <div className={css['options-all']} style={{ height: active ? '165px' : '' }}>
        <div className={css['options-all-item']} onClick={() => selectSheet(RENT)}>{RENT}</div>
        <div className={css['options-all-item']} onClick={() => selectSheet(MTO)}>{MTO}</div>
        <div className={css['options-all-item']} onClick={() => selectSheet(OCP)}>{OCP}</div>
        <div className={css['options-all-item']} onClick={() => selectSheet(RANK)}>{RANK}</div>
      </div>
    </div>
    {type === MTO &&
      <div className={css['suboptions']}>
        <div className={css['suboptions-item']}>
          <div className={css['suboptions-item-title']}>请选择年月</div>
          <input type='month' onInput={(e) => setMtoTime(e.target.value)} />
        </div>
        <div className={css['query-button']} onClick={onClickQuery}>统计</div>
      </div>
    }
    {type === OCP &&
      <div className={css['suboptions']}>
        <div className={css['suboptions-item']}>
          <div className={css['suboptions-item-title']}>请输入年份</div>
          <input onInput={(e) => setOcpTime(e.target.value)} />
        </div>
        <div className={css['query-button']} onClick={onClickQuery}>统计</div>
      </div>
    }
    <div
      style={{ color: '#2295ff', marginBottom: '20px' }}
    >
      统计时间：{`${new Date().getFullYear()}年${new Date().getMonth() + 1}月${new Date().getDate()}日${curTime.hour}时${curTime.minute}分`}
    </div>
    {/* 统计当前每种车辆类型的车辆总数、已出租数、未出租数 */}
    {type === RENT && rentStat.length !== 0 &&
      <div className={css['table']}>
        <div className={`${css['table-item']} ${css['table-title']} `}>
          <div className={css['table-item-items']}>车辆类型</div>
          <div className={css['table-item-items']}>车辆总数</div>
          <div className={css['table-item-items']}>已出租数</div>
          <div className={css['table-item-items']}>未出租数</div>
        </div>
        {rentStat.map((item, index) => {
          return (
            <div key={index} className={`${css['table-item']}`}>
              <div className={css['table-item-items']}>{item['车辆类型名称']}</div>
              <div className={css['table-item-items']}>{item['车辆总数']}</div>
              <div className={css['table-item-items']}>{item['已出租数']}</div>
              <div className={css['table-item-items']}>{item['未出租数']}</div>
            </div>
          )
        })}
        <div className={css['table-item']} style={{ fontWeight: "bold" }}>
          <div className={css['table-item-items']}>合计</div>
          <div className={css['table-item-items']}>{rentStat.map(item => item['车辆总数']).reduce((pre, cur) => pre + cur)}</div>
          <div className={css['table-item-items']}>{rentStat.map(item => item['已出租数']).reduce((pre, cur) => pre + cur)}</div>
          <div className={css['table-item-items']}>{rentStat.map(item => item['未出租数']).reduce((pre, cur) => pre + cur)}</div>
        </div>
      </div>
    }
    {/* 统计当前每种车辆类型的车辆总数、已出租数、未出租数 */}
    {type === MTO && mtoStat.length !== 0 &&
      <div className={css['table']}>
        <div className={`${css['table-item']} ${css['table-title']} `}>
          <div className={css['table-item-items']}>车辆类型</div>
          <div className={css['table-item-items']}>营业额</div>
        </div>
        {mtoStat.map((item, index) => {
          return (
            <div key={index} className={`${css['table-item']}`}>
              <div className={css['table-item-items']}>{item['车辆类型']}</div>
              <div className={css['table-item-items']}>{item['营业额']}</div>
            </div>
          )
        })}
        <div className={css['table-item']} style={{ fontWeight: "bold" }}>
          <div className={css['table-item-items']}>合计</div>
          <div className={css['table-item-items']}>{mtoStat.map(item => item['营业额']).reduce((pre, cur) => pre + cur)}</div>
        </div>
      </div>
    }
    {type === MTO && mtoStat.length !== 0 &&
      <div>
        <Bar data={mtoChart} width={500} height={300} />
      </div>
    }
    {type === OCP && ocpStat.length !== 0 &&
      <div className={css['table']}>
        <div className={`${css['table-item']} ${css['table-title']} `}>
          <div className={css['table-item-items']}>车牌号</div>
          <div className={css['table-item-items']}>车辆名称</div>
          <div className={css['table-item-items']}>营业额</div>
          <div className={css['table-item-items']}>租用率</div>
        </div>
        {ocpStat.map((item, index) => {
          return (
            <div key={index} className={`${css['table-item']}`}>
              <div className={css['table-item-items']}>{item['车牌号']}</div>
              <div className={css['table-item-items']}>{item['车辆名称']}</div>
              <div className={css['table-item-items']}>{item['营业额']}</div>
              <div className={css['table-item-items']}>{item['租用率']}%</div>
            </div>
          )
        })}
        <div className={css['table-item']} style={{ fontWeight: "bold" }}>
          <div className={css['table-item-items']}>合计</div>
          <div className={css['table-item-items']}><span role='img' aria-label='小汽车'>🚗</span></div>
          <div className={css['table-item-items']}>{ocpStat.map(item => item['营业额']).reduce((pre, cur) => pre + cur)}</div>
          <div className={css['table-item-items']}>{ocpStat.map(item => item['租用率']).reduce((pre, cur) => pre + cur)}%</div>
        </div>
      </div>
    }
    {type === RANK && rankStat.length !== 0 &&
      <div className={css['table']}>
        <div className={`${css['table-item']} ${css['table-title']} `}>
          <div className={css['table-item-items']}>车牌号</div>
          <div className={css['table-item-items']}>车辆名称</div>
          <div className={css['table-item-items']}>累计出租天数</div>
          <div className={css['table-item-items']}>营业额</div>
          <div className={css['table-item-items']}>租用率</div>
        </div>
        {rankStat.map((item, index) => {
          return (
            <div key={index} className={`${css['table-item']}`}>
              <div className={css['table-item-items']}>{item['车牌号']}</div>
              <div className={css['table-item-items']}>{item['车辆名称']}</div>
              <div className={css['table-item-items']}>{item['sum']}</div>
              <div className={css['table-item-items']}>{item['营业额']}</div>
              <div className={css['table-item-items']}>{item['租用率']}%</div>
            </div>
          )
        })}
      </div>
    }
    {type === RANK && rankStat.length !== 0 &&
      <div>
        <Bar data={rankChart} width={800} height={300} />
      </div>
    }
  </div>
}