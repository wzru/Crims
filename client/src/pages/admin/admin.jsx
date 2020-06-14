import React, { useState, createContext, useEffect } from "react";
import css from "./admin.module.scss";
import addIcon from '../../assets/add.svg';
import addIconG from '../../assets/add-g.svg';
import delIcon from '../../assets/del.svg';
import delIconG from '../../assets/del-g.svg';
import editIcon from '../../assets/edit.svg';
import editIconG from '../../assets/edit-g.svg';
import downIcon from '../../assets/down.svg';
import downIconG from '../../assets/down-g.svg';
import Modal from "../../components/modal/modal";
import { CARTYPE, CARINFO, RENTORDER, DEFAULT } from '../../constants/sheet';
import { ADD, DEL, EDIT } from "../../constants/manipulation";
import { SELECTCARTYPE, SELECTCARINFO, SELECTRENTORDER } from '../../constants/sql';

export const AdminContext = createContext();

const ws = new WebSocket('ws://localhost:5000');
ws.onopen = function () {
  console.log('open')
}
const ws1 = new WebSocket('ws://localhost:5001');
ws1.onopen = function () {
  console.log('1open');
}

export default function Admin() {
  // 当前表格的分类
  const [type, setType] = useState(DEFAULT);
  const [active, setActive] = useState(false);
  /* ----车辆类型 cartype ---- */
  // 车辆类型数据
  const [carType, setCarType] = useState([]);
  // 选中的车辆类型数据
  const [curCarType, setCurCarType] = useState([]);
  // 新的车辆类型数据
  const [newCarType, setNewCarType] = useState({});
  // 编辑的车辆类型数据
  const [editingCarType, setEditingCarType] = useState();

  /* ----车辆信息 carinfo ---- */
  // 车辆信息数据
  const [carInfo, setCarInfo] = useState([]);
  // 选中的车辆信息数据
  const [curCarInfo, setCurCarInfo] = useState([]);
  const [newCarInfo, setNewCarInfo] = useState({ rent: 'y' });
  const [editingCarInfo, setEditingCarInfo] = useState({});

  /* ----租车订单 rentorder ---- */
  // 租车订单数据
  const [rentOrder, setRentOrder] = useState([]);
  // 选中的租车订单数据
  const [curRentOrder, setCurRentOrder] = useState([]);
  const [newRentOrder, setNewRentOrder] = useState({ scheduled_fee: 0 });
  const [editingRentOrder, setEditingRentOrder] = useState({});

  /* ---- 模态窗 ---- */
  // 是否显示模态窗
  const [showModal, setShowModal] = useState(false);
  const [modalTitle, setModalTitle] = useState('添加数据');
  const [modalButton, setModalButton] = useState('添加');

  /**
   * 点击选择表格
   * @param {string} sheetType
   */
  const selectSheet = (sheetType) => {
    setType(sheetType);
    setActive(false);
    let query, method;
    if (sheetType === CARTYPE) {
      query = SELECTCARTYPE;
      method = setCarType;
    }
    if (sheetType === CARINFO) {
      query = SELECTCARINFO;
      method = setCarInfo;
    }
    if (sheetType === RENTORDER) {
      query = SELECTRENTORDER;
      method = setRentOrder;
    }
    ws.send(query)
    ws.onmessage = function (msg) {
      const data = JSON.parse(msg.data);
      if (data.type === 'SELECT') {
        console.log(data);
        method(data.data);
      }
    }
  }
  /**
   * 点击选择表格中的任意一项
   * @param {string} id
   * @param {string} sheetType
   */
  const selectItem = (id, sheetType) => {
    console.log(id, sheetType);
    let curItems, curSheet, curMethod, editingMethod;
    // 如果是车辆分类表
    if (sheetType === CARTYPE) {
      curSheet = carType;
      curItems = curCarType.slice(0);
      curMethod = setCurCarType;
      editingMethod = setEditingCarType;
    }
    // 如果是车辆信息表
    if (sheetType === CARINFO) {
      curSheet = carInfo;
      curItems = curCarInfo.slice(0);
      curMethod = setCurCarInfo;
      editingMethod = setEditingCarInfo;
    }
    // 如果是租车订单表
    if (sheetType === RENTORDER) {
      curSheet = rentOrder;
      curItems = curRentOrder.slice(0);
      curMethod = setCurRentOrder;
      editingMethod = setEditingRentOrder;
    }
    const index = curItems.indexOf(id);
    if (index !== -1) {
      curItems = curItems.slice(0, index).concat(curItems.slice(index + 1));
    }
    else {
      curItems.push(id);
    }
    curMethod(curItems)
    if (curItems.length === 1) {
      console.log('cursheet', editingCarInfo)
      editingMethod(curSheet[id]);
      console.log(curItems, curSheet[id]);
    }
  }
  /**
   * 点击添加按钮
   */
  const onClickAdd = () => {
    const isCartype = (type === CARTYPE && !curCarType.length);
    const isCarInfo = (type === CARINFO && !curCarInfo.length);
    const isRentOrder = (type === RENTORDER && !curRentOrder.length);
    if (isCartype || isCarInfo || isRentOrder) {
      setShowModal(true);
      setModalTitle(ADD);
      setModalButton('添加')
    }
  }
  const onClickDel = () => {
    const isCartype = (type === CARTYPE && curCarType.length);
    const isCarInfo = (type === CARINFO && curCarInfo.length);
    const isRentOrder = (type === RENTORDER && curRentOrder.length);
    if (isCartype || isCarInfo || isRentOrder) {
      setShowModal(true);
      setModalTitle(DEL);
      setModalButton('删除')
    }
  }
  const onClickEdit = () => {
    const isCartype = (type === CARTYPE && curCarType.length === 1);
    const isCarInfo = (type === CARINFO && curCarInfo.length === 1);
    const isRentOrder = (type === RENTORDER && curRentOrder.length === 1);
    if (isCartype || isCarInfo || isRentOrder) {
      setShowModal(true);
      setModalTitle(EDIT);
      setModalButton('保存')
    }
  }
  /**
  * 点击确认添加按钮
  */
  const onClickConfirmAdd = () => {
    console.log('done');
    setShowModal(false);
    // 插入车辆类型表
    if (type === CARTYPE) {
      ws.send(`INSERT INTO CAR_TYPE VALUES ('${newCarType.code}', '${newCarType.tname}', ${Number(newCarType.quantity)});`);
      ws.send(SELECTCARTYPE);
    }
    // 插入车辆信息表
    if (type === CARINFO) {
      // 修改车辆信息表
      const { cid, plate, code, cname, gear, daily_rent, rent } = newCarInfo;
      ws.send(`INSERT INTO CAR_INFO VALUES (${Number(cid)}, '${plate}', '${code}', '${cname}', '${gear}', ${Number(daily_rent)}, '${rent}');`)
      ws.send(SELECTCARINFO);
      // 修改车辆类型表
      ws1.send(`SELECT quantity FROM CAR_TYPE WHERE code='${code}';`);
      ws1.onmessage = function (msg) {
        const new_quantity = JSON.parse(msg.data).data[0].quantity + 1;
        ws.send(`UPDATE CAR_TYPE SET quantity=${new_quantity} WHERE code='${code}';`);
      }
    }
    // 插入租车订单表
    if (type === RENTORDER) {
      const { identity_number, pname, phone_number, cid, pickup_time, scheduled_dropoff_time, actual_dropoff_time, scheduled_fee, actual_fee } = newRentOrder;
      const pickup_date = pickup_time.split('/')[0];
      // 查找当天的订单数 计算出租车订单的oid
      ws1.send(`SELECT * FROM RENT_ORDER WHERE pickup_time>='${pickup_date}/00:00' AND pickup_time<='${pickup_date}/23:59';`)
      ws1.onmessage = function (msg) {
        console.log('订单', msg.data);
        const cnt = JSON.parse(msg.data).data.length + 1;
        const oid = `${pickup_date.split('-')[0]}${pickup_date.split('-')[1]}${pickup_date.split('-')[2]}${cnt < 10 ? `0${cnt}` : cnt}`;
        ws.send(`INSERT INTO RENT_ORDER VALUES ('${oid}', '${identity_number}', '${pname}', '${phone_number}', ${Number(cid)}, '${pickup_time}', '${scheduled_dropoff_time}', ${Number(scheduled_fee) * 5}, '${actual_dropoff_time}', ${Number(scheduled_fee)}, ${Number(actual_fee)});`)
        ws.send(SELECTRENTORDER);
      }
    }
  }
  const onClickConfirmDel = () => {
    setShowModal(false);
    if (type === CARTYPE) {
      for (let index of curCarType) {
        ws.send(`DELETE FROM CAR_TYPE WHERE code='${carType[index].code}';`);
      }
      ws.send(SELECTCARTYPE);
      setCurCarType([]);
    }
    if (type === CARINFO) {
      for (let i = 0; i < curCarInfo.length; ++i) {
        const index = curCarInfo[i];
        // settimeout 不然收不到返回值
        setTimeout(() => {
          console.log('index', index)
          const { code, cid } = carInfo[index];
          ws.send(`DELETE FROM CAR_INFO WHERE cid=${Number(cid)};`);
          // 修改车辆类型表的quantity
          ws1.send(`SELECT quantity FROM CAR_TYPE WHERE code='${code}';`);
          ws1.onmessage = function (msg) {
            const new_quantity = JSON.parse(msg.data).data[0].quantity - 1;
            console.log('newquantity', new_quantity);
            ws.send(`UPDATE CAR_TYPE SET quantity=${new_quantity} WHERE code='${code}';`);
          }
        }, 200 * i);
      }
      ws.send(SELECTCARINFO);
      setCurCarInfo([])
    }
    if (type === RENTORDER) {
      for (let index of curRentOrder) {
        ws.send(`DELETE FROM RENT_ORDER WHERE oid='${rentOrder[index].oid}';`);
      }
      ws.send(SELECTRENTORDER);
      setCurRentOrder([]);
    }
  }
  const onClickConfirmEdit = () => {
    setShowModal(false);
    if (type === CARTYPE) {
      const { code, tname, quantity } = editingCarType;
      const { code: old_code } = carType[curCarType[0]];
      ws.send(`UPDATE CAR_TYPE SET code='${code}', tname='${tname}', quantity=${Number(quantity)} WHERE code='${old_code}';`)
      ws.send(SELECTCARTYPE);
      setCurCarType([]);
    }
    if (type === CARINFO) {
      const { cid, plate, code, cname, gear, daily_rent, rent } = editingCarInfo;
      const { cid: old_cid, code: old_code } = carInfo[curCarInfo[0]];
      // 更新车辆信息表
      ws.send(`UPDATE CAR_INFO SET cid=${Number(cid)}, plate='${plate}', code='${code}', cname='${cname}', gear='${gear}', daily_rent=${Number(daily_rent)}, rent='${rent}' WHERE cid=${old_cid};`)
      ws.send(SELECTCARINFO);
      // 更新车辆类型表的quantity
      setTimeout(() => {
        ws1.send(`SELECT quantity FROM CAR_TYPE WHERE code='${old_code}';`);
        ws1.onmessage = function (msg) {
          const new_quantity = JSON.parse(msg.data).data[0].quantity - 1;
          console.log('newquantity', new_quantity);
          ws.send(`UPDATE CAR_TYPE SET quantity=${new_quantity} WHERE code='${old_code}';`);
        }
      }, 0);
      setTimeout(() => {
        ws1.send(`SELECT quantity FROM CAR_TYPE WHERE code='${code}';`);
        ws1.onmessage = function (msg) {
          const new_quantity = JSON.parse(msg.data).data[0].quantity + 1;
          console.log('newquantity', new_quantity);
          ws.send(`UPDATE CAR_TYPE SET quantity=${new_quantity} WHERE code='${code}';`);
        }
      }, 200);
      setCurCarInfo([]);
    }
    if (type === RENTORDER) {
      const { oid, identity_number, pname, phone_number, cid, pickup_time, scheduled_dropoff_time, actual_dropoff_time, scheduled_fee, actual_fee } = editingRentOrder;
      const { oid: old_oid } = rentOrder[curRentOrder[0]];
      ws.send(`UPDATE RENT_ORDER SET oid='${oid}', identity_number='${identity_number}', pname='${pname}', phone_number='${phone_number}', cid=${Number(cid)}, pickup_time='${pickup_time}', scheduled_dropoff_time='${scheduled_dropoff_time}', deposit=${Number(scheduled_fee) * 5}, actual_dropoff_time='${actual_dropoff_time}', scheduled_fee=${Number(scheduled_fee)}, actual_fee=${Number(actual_fee)} WHERE oid='${old_oid}';`);
      ws.send(SELECTRENTORDER);
      setCurRentOrder([]);
    }
  }
  const onClickDone = () => {
    if (modalTitle === ADD) {
      onClickConfirmAdd();
    }
    if (modalTitle === DEL) {
      onClickConfirmDel();
    }
    if (modalTitle === EDIT) {
      onClickConfirmEdit();
    }
  }
  return <div className={css.index}>
    {/* 遮罩 */}
    < div hidden={!showModal} className={css['mask']} onClick={() => setShowModal(false)}></div>
    {/* 模态窗 */}
    <div hidden={!showModal} className={css['modal']}>
      <AdminContext.Provider
        value={{
          carType, newCarType, setNewCarType, curCarType, editingCarType, setEditingCarType,
          carInfo, newCarInfo, setNewCarInfo, curCarInfo, editingCarInfo, setEditingCarInfo,
          rentOrder, newRentOrder, setNewRentOrder, curRentOrder, editingRentOrder, setEditingRentOrder,
        }}>
        <Modal
          title={`${modalTitle}-${type}`}
          buttonText={modalButton}
          onClickCancel={() => setShowModal(false)}
          onClickDone={onClickDone}
        />
      </AdminContext.Provider>
    </div>
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
      <div className={css['options-all']} style={{ height: active ? '125px' : '' }}>
        <div className={css['options-all-item']} onClick={() => selectSheet(CARTYPE)}>车辆分类</div>
        <div className={css['options-all-item']} onClick={() => selectSheet(CARINFO)}>车辆信息</div>
        <div className={css['options-all-item']} onClick={() => selectSheet(RENTORDER)}>租车订单</div>
      </div>
    </div>
    {/* 按钮 */}
    <div className={css['buttons']}>
      <div className={css['buttons-left']}>
        <img
          alt='add'
          src={((type === CARTYPE && !curCarType.length) || (type === CARINFO && !curCarInfo.length) || (type === RENTORDER && !curRentOrder.length) ? addIcon : addIconG)}
          onClick={onClickAdd}
        />
        <img
          alt='del'
          src={((type === CARTYPE && curCarType.length) || (type === CARINFO && curCarInfo.length) || (type === RENTORDER && curRentOrder.length) ? delIcon : delIconG)}
          onClick={onClickDel}
        />
        <img
          alt=''
          src={((type === CARTYPE && curCarType.length === 1) || (type === CARINFO && curCarInfo.length === 1) || (type === RENTORDER && curRentOrder.length === 1) ? editIcon : editIconG)}
          onClick={onClickEdit}
        />
      </div>
      <div className={css['buttons-right']}>
        <div className={css['buttons-right-item']}><span>导入</span><img alt='' src={require('../../assets/upload.svg')} /></div>
        <div className={css['buttons-right-item']}><span>导出</span><img alt='' src={require('../../assets/download.svg')} /></div>
      </div>
    </div>
    {/* 车辆分类表格 */}
    {type === '车辆分类' && <div className={css['table']} hidden={type !== '车辆分类'}>
      <div className={`${css['table-item']} ${css['table-title']} `}>
        <div className={css['table-item-code']}>车辆类型编码</div>
        <div className={css['table-item-tname']}>车辆类型名称</div>
        <div className={css['table-item-quantity']}>库存数量</div>
      </div>
      {carType.map((item, index) => <div
        key={item.code}
        className={css['table-item']}
        onClick={() => selectItem(index, CARTYPE)}
        style={{ backgroundColor: curCarType.indexOf(index) !== -1 ? '#91d5ff' : '' }}
      >
        <div className={css['table-item-code']}>{item.code}</div>
        <div className={css['table-item-tname']}>{item.tname}</div>
        <div className={css['table-item-quantity']}>{item.quantity}</div>
      </div>)}
    </div>}
    {/* 车辆信息表格 */}
    {type === '车辆信息' && <div className={css['table']}>
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
          onClick={() => selectItem(index, CARINFO)}
          style={{ backgroundColor: curCarInfo.indexOf(index) !== -1 ? '#91d5ff' : '' }}
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
    {/* 租车订单 */}
    {
      type === RENTORDER && <div className={css['table']}>
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
              onClick={() => selectItem(index, RENTORDER)}
              style={{ backgroundColor: curRentOrder.indexOf(index) !== -1 ? '#91d5ff' : '' }}
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
      </div>
    }
  </div >
}