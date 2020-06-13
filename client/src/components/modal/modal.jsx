import React, { useContext } from "react";
import css from "./modal.module.scss";
import { AdminContext } from "../../pages/admin/admin";
import { ADD, DEL, EDIT } from "../../constants/manipulation";
import { CARTYPE, CARINFO, RENTORDER } from "../../constants/sheet";

export default function Modal(props) {
  let {
    carType, newCarType, setNewCarType, curCarType, editingCarType, setEditingCarType,
    carInfo, newCarInfo, setNewCarInfo, curCarInfo, editingCarInfo, setEditingCarInfo,
    rentOrder, newRentOrder, setNewRentOrder, curRentOrder, editingRentOrder, setEditingRentOrder,
  } = useContext(AdminContext);
  return <div className={css['index']}>
    <div className={css['title']}>{props.title}</div>
    <div className={css['content']}>
      {/* 添加车辆类型 */}
      {
        props.title.split('-')[0] === ADD && props.title.split('-')[1] === CARTYPE &&
        <div className={css['modal-content']}>
          <div className={css['modal-content-item']}>
            <span>车辆类型编码</span>
            <input
              type='number'
              onInput={(e) => { setNewCarType({ ...newCarType, code: e.target.value }) }}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>车辆类型名称</span>
            <input
              onInput={(e) => { setNewCarType({ ...newCarType, tname: e.target.value }); }}
              placeholder='如：经济型'
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>库存数量</span>
            <input
              type='number'
              onInput={(e) => setNewCarType({ ...newCarType, quantity: e.target.value })}
            />
          </div>
        </div>
      }
      {/* 添加车辆信息 */}
      {
        props.title.split('-')[0] === ADD && props.title.split('-')[1] === CARINFO &&
        <div className={css['modal-content']}>
          <div className={css['modal-content-item']}>
            <span>车辆编号</span>
            <input
              onInput={(e) => { setNewCarInfo({ ...newCarInfo, cid: e.target.value }) }}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>车牌号</span>
            <input
              onInput={(e) => { setNewCarInfo({ ...newCarInfo, plate: e.target.value }); }}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>车辆类型编码</span>
            <input
              type='number'
              onInput={(e) => setNewCarInfo({ ...newCarInfo, code: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>车辆名称</span>
            <input
              onInput={(e) => setNewCarInfo({ ...newCarInfo, cname: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>排挡方式</span>
            <input
              onInput={(e) => setNewCarInfo({ ...newCarInfo, gear: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>每日租金</span>
            <input
              type='number'
              onInput={(e) => setNewCarInfo({ ...newCarInfo, daily_rent: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>出租状态</span>
            <select onChange={(e) => setNewCarInfo({ ...newCarInfo, rent: e.target.value })}>
              <option
                value='y'
              >
                已出租
                </option>
              <option
                value='n'
              >
                未出租
               </option>
            </select>
          </div>
        </div>
      }
      {/* 添加租车订单 */}
      {
        props.title.split('-')[0] === ADD && props.title.split('-')[1] === RENTORDER &&
        <div className={css['modal-content']}>
          <div className={css['modal-content-item']}>
            <span>身份证号</span>
            <input
              type='number'
              onInput={(e) => setNewRentOrder({ ...newRentOrder, identity_number: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>客人姓名</span>
            <input
              onInput={(e) => setNewRentOrder({ ...newRentOrder, pname: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>手机号码</span>
            <input
              onInput={(e) => setNewRentOrder({ ...newRentOrder, phone_number: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>租用车辆编号</span>
            <input
              type='number'
              onInput={(e) => setNewRentOrder({ ...newRentOrder, cid: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>取车时间</span>
            <input
              onChange={(e) => setNewRentOrder({ ...newRentOrder, pickup_time: e.target.value })}
              placeholder='如：2000-05-12/10:27'
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>预约还车时间</span>
            <input
              onChange={(e) => setNewRentOrder({ ...newRentOrder, scheduled_dropoff_time: e.target.value })}
              placeholder='如：2000-05-12/10:27'
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>押金</span>
            <input
              disabled
              value={newRentOrder.scheduled_fee * 5}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>实际还车时间</span>
            <input
              onChange={(e) => setNewRentOrder({ ...newRentOrder, actual_dropoff_time: e.target.value })}
              placeholder='如：2000-05-12/10:27'
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>应缴费用</span>
            <input
              type='number'
              onInput={(e) => setNewRentOrder({ ...newRentOrder, scheduled_fee: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>实缴费用</span>
            <input
              type='number'
              onInput={(e) => setNewRentOrder({ ...newRentOrder, actual_fee: e.target.value })}
            />
          </div>
        </div>
      }
      {/* 删除车辆类型 */}
      {
        props.title.split('-')[0] === DEL && props.title.split('-')[1] === CARTYPE &&
        <div>
          是否确认删除这{curCarType.length}条数据？
        </div>
      }
      {/* 删除车辆信息 */}
      {
        props.title.split('-')[0] === DEL && props.title.split('-')[1] === CARINFO &&
        <div>
          是否确认删除这{curCarInfo.length}条数据？
        </div>
      }
      {/* 删除租车订单 */}
      {
        props.title.split('-')[0] === DEL && props.title.split('-')[1] === RENTORDER &&
        <div>
          是否确认删除这{curRentOrder.length}条数据？
        </div>
      }
      {/* 编辑车辆类型 */}
      {
        props.title.split('-')[0] === EDIT && props.title.split('-')[1] === CARTYPE &&
        <div className={css['modal-content']} key={curCarType.length}>
          <div className={css['modal-content-item']}>
            <span>车辆类型编码</span>
            <input
              type='number'
              defaultValue={curCarType.length ? carType[curCarType[0]].code : ''}
              onInput={(e) => { setEditingCarType({ ...editingCarType, code: e.target.value }) }}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>车辆类型名称</span>
            <input
              placeholder='如：经济型'
              onInput={(e) => { setEditingCarType({ ...editingCarType, tname: e.target.value }); }}
              defaultValue={curCarType.length ? carType[curCarType[0]].tname : ''}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>库存数量</span>
            <input
              type='number'
              onInput={(e) => setEditingCarType({ ...editingCarType, quantity: e.target.value })}
              defaultValue={curCarType.length ? carType[curCarType[0]].quantity : ''}
            />
          </div>
        </div>
      }
      {/* 编辑车辆信息 */}
      {
        props.title.split('-')[0] === EDIT && props.title.split('-')[1] === CARINFO &&
        <div className={css['modal-content']} key={curCarInfo.length}>
          <div className={css['modal-content-item']}>
            <span>车辆编号</span>
            <input
              onInput={(e) => { setEditingCarInfo({ ...editingCarInfo, cid: e.target.value }) }}
              defaultValue={curCarInfo.length ? carInfo[curCarInfo[0]].cid : ''}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>车牌号</span>
            <input
              onInput={(e) => { setEditingCarInfo({ ...editingCarInfo, plate: e.target.value }); }}
              defaultValue={curCarInfo.length ? carInfo[curCarInfo[0]].plate : ''}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>车辆类型编码</span>
            <input
              type='number'
              onInput={(e) => setEditingCarInfo({ ...editingCarInfo, code: e.target.value })}
              defaultValue={curCarInfo.length ? carInfo[curCarInfo[0]].code : ''}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>车辆名称</span>
            <input
              onInput={(e) => setEditingCarInfo({ ...editingCarInfo, cname: e.target.value })}
              defaultValue={curCarInfo.length ? carInfo[curCarInfo[0]].cname : ''}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>排挡方式</span>
            <input
              onInput={(e) => setEditingCarInfo({ ...editingCarInfo, gear: e.target.value })}
              defaultValue={curCarInfo.length ? carInfo[curCarInfo[0]].gear : ''}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>每日租金</span>
            <input
              type='number'
              onInput={(e) => setEditingCarInfo({ ...editingCarInfo, daily_rent: e.target.value })}
              defaultValue={curCarInfo.length ? carInfo[curCarInfo[0]].daily_rent : ''}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>出租状态</span>
            <select
              defaultValue={(curCarInfo.length && carInfo[curCarInfo[0]].rent === 'y') ? 'y' : 'n'}
              onChange={(e) => setEditingCarInfo({ ...editingCarInfo, rent: e.target.value })}
            >
              <option
                value='y'
              >
                已出租
                </option>
              <option
                value='n'
              >
                未出租
               </option>
            </select>
          </div>
        </div>
      }
      {/* 更新租车订单 */}
      {
        props.title.split('-')[0] === EDIT && props.title.split('-')[1] === RENTORDER &&
        <div className={css['modal-content']} key={curRentOrder.length}>
          <div className={css['modal-content-item']}>
            <span>身份证号</span>
            <input
              defaultValue={curRentOrder.length ? rentOrder[curRentOrder[0]].identity_number : ''}
              type='number'
              onInput={(e) => setEditingRentOrder({ ...editingRentOrder, identity_number: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>客人姓名</span>
            <input
              defaultValue={curRentOrder.length ? rentOrder[curRentOrder[0]].pname : ''}
              onInput={(e) => setEditingRentOrder({ ...editingRentOrder, pname: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>手机号码</span>
            <input
              defaultValue={curRentOrder.length ? rentOrder[curRentOrder[0]].phone_number : ''}
              onInput={(e) => setEditingRentOrder({ ...editingRentOrder, phone_number: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>租用车辆编号</span>
            <input
              type='number'
              defaultValue={curRentOrder.length ? rentOrder[curRentOrder[0]].cid : ''}
              onInput={(e) => setEditingRentOrder({ ...editingRentOrder, cid: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>取车时间</span>
            <input
              defaultValue={curRentOrder.length ? rentOrder[curRentOrder[0]].pickup_time : ''}
              onChange={(e) => setEditingRentOrder({ ...editingRentOrder, pickup_time: e.target.value })}
              placeholder='如：2000-05-12/10:27'
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>预约还车时间</span>
            <input
              defaultValue={curRentOrder.length ? rentOrder[curRentOrder[0]].scheduled_dropoff_time : ''}
              onChange={(e) => setEditingRentOrder({ ...editingRentOrder, scheduled_dropoff_time: e.target.value })}
              placeholder='如：2000-05-12/10:27'
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>押金</span>
            <input
              disabled
              defaultValue={editingRentOrder.scheduled_fee * 5}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>实际还车时间</span>
            <input
              defaultValue={curRentOrder.length ? rentOrder[curRentOrder[0]].actual_dropoff_time : ''}
              onChange={(e) => setEditingRentOrder({ ...editingRentOrder, actual_dropoff_time: e.target.value })}
              placeholder='如：2000-05-12/10:27'
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>应缴费用</span>
            <input
              type='number'
              defaultValue={curRentOrder.length ? rentOrder[curRentOrder[0]].scheduled_fee : ''}
              onInput={(e) => setEditingRentOrder({ ...editingRentOrder, scheduled_fee: e.target.value })}
            />
          </div>
          <div className={css['modal-content-item']}>
            <span>实缴费用</span>
            <input
              type='number'
              defaultValue={curRentOrder.length ? rentOrder[curRentOrder[0]].actual_fee : ''}
              onInput={(e) => setEditingRentOrder({ ...editingRentOrder, actual_fee: e.target.value })}
            />
          </div>
        </div>
      }
    </div>
    <div className={css['foot']}>
      <button onClick={props.onClickCancel} className={`${css['cancel']} ${css['button']}`}>取消</button>
      <button
        onClick={props.onClickDone}
        className={`${css['done']} ${css['button']}`}
        style={{ backgroundColor: props.buttonText === '删除' ? '#fb3f51' : '#1890ff' }}
      >
        {props.buttonText}
      </button>
    </div>
  </div >
}