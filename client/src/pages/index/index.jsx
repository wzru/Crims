import React from "react";
import css from "./index.module.scss";
import { ADMIN, QUERY, STAT } from "../../constants/pages";

export default function Index(props) {
  return <div className={css['index']}>
    <div className={css['welcome']}>
      <div className={css['welcome-en']}>Welcome to Crims</div>
      <div className={css['welcome-zh']}>汽车租赁信息管理系统</div>
    </div>
    <div className={css['buttons']}>
      <div onClick={() => props.setPage(ADMIN)}>数据维护</div>
      <div onClick={() => props.setPage(QUERY)}>数据查询</div>
      <div onClick={() => props.setPage(STAT)}>数据统计</div>
    </div>
  </div>;
}
